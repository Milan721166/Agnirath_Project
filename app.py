from flask import Flask, render_template, Response, request, send_file, redirect, url_for
import requests
import time
import os
import threading
from datetime import datetime
import cv2
import numpy as np

app = Flask(__name__)

# Configuration - UPDATE THESE!
ESP32_CAM_IP = "192.168.93.77"  # Must match ESP32-CAM's IP
ESP32_CAM_PORT = 80
CAPTURE_INTERVAL = 5  # seconds
IMAGE_FOLDER = 'static/images'
VIDEO_FOLDER = 'static/videos'
TIMEOUT = 5  # seconds

# Create folders if they don't exist
os.makedirs(IMAGE_FOLDER, exist_ok=True)
os.makedirs(VIDEO_FOLDER, exist_ok=True)

# Global variables
is_recording = False
video_writer = None
last_capture_time = 0
esp32_available = False

def check_esp32_connection():
    """Check if ESP32-CAM is reachable"""
    global esp32_available
    try:
        response = requests.get(
            f"http://{ESP32_CAM_IP}:{ESP32_CAM_PORT}/status",
            timeout=TIMEOUT
        )
        esp32_available = response.status_code == 200
    except Exception as e:
        print(f"Connection check failed: {str(e)}")
        esp32_available = False
    return esp32_available

def get_esp32_cam_image():
    """Fetch image from ESP32-CAM with better error handling"""
    if not check_esp32_connection():
        return None
        
    try:
        response = requests.get(
            f"http://{ESP32_CAM_IP}:{ESP32_CAM_PORT}/capture",
            timeout=TIMEOUT
        )
        if response.status_code == 200:
            img_array = np.frombuffer(response.content, dtype=np.uint8)
            return cv2.imdecode(img_array, cv2.IMREAD_COLOR)
    except Exception as e:
        print(f"Error getting image: {str(e)}")
    return None

def generate_frames():
    """Video streaming generator function"""
    while True:
        img = get_esp32_cam_image()
        if img is not None:
            ret, buffer = cv2.imencode('.jpg', img)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        time.sleep(0.1)

def auto_capture_images():
    """Thread function to automatically capture images"""
    global last_capture_time
    
    while True:
        current_time = time.time()
        if current_time - last_capture_time >= CAPTURE_INTERVAL:
            if check_esp32_connection():
                try:
                    img = get_esp32_cam_image()
                    if img is not None:
                        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                        filename = f"{IMAGE_FOLDER}/capture_{timestamp}.jpg"
                        cv2.imwrite(filename, img)
                        print(f"Auto-captured image: {filename}")
                        last_capture_time = current_time
                except Exception as e:
                    print(f"Auto-capture error: {str(e)}")
            else:
                print("ESP32-CAM not available for auto-capture")
        time.sleep(1)

@app.route('/')
def index():
    """Video streaming and control page"""
    cam_status = "Online" if check_esp32_connection() else "Offline"
    
    images = [f for f in os.listdir(IMAGE_FOLDER) if f.endswith('.jpg')]
    images.sort(reverse=True)
    
    videos = [f for f in os.listdir(VIDEO_FOLDER) if f.endswith('.avi')]
    videos.sort(reverse=True)
    
    return render_template('index.html', 
                         images=images, 
                         videos=videos, 
                         is_recording=is_recording, 
                         interval=CAPTURE_INTERVAL,
                         cam_status=cam_status,
                         cam_ip=ESP32_CAM_IP)

@app.route('/video_feed')
def video_feed():
    """Video streaming route"""
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/capture')
def capture():
    """Manual image capture"""
    if check_esp32_connection():
        img = get_esp32_cam_image()
        if img is not None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"{IMAGE_FOLDER}/manual_{timestamp}.jpg"
            cv2.imwrite(filename, img)
            return redirect(url_for('index'))
    return "Failed to capture image", 500

@app.route('/download_image/<filename>')
def download_image(filename):
    """Download captured image"""
    return send_file(f"{IMAGE_FOLDER}/{filename}", as_attachment=True)

@app.route('/download_video/<filename>')
def download_video(filename):
    """Download recorded video"""
    return send_file(f"{VIDEO_FOLDER}/{filename}", as_attachment=True)

@app.route('/delete_image/<filename>')
def delete_image(filename):
    """Delete captured image"""
    try:
        os.remove(f"{IMAGE_FOLDER}/{filename}")
    except Exception as e:
        print(f"Error deleting image: {str(e)}")
    return redirect(url_for('index'))

@app.route('/delete_video/<filename>')
def delete_video(filename):
    """Delete recorded video"""
    try:
        os.remove(f"{VIDEO_FOLDER}/{filename}")
    except Exception as e:
        print(f"Error deleting video: {str(e)}")
    return redirect(url_for('index'))

@app.route('/start_recording')
def start_recording():
    """Start video recording"""
    global is_recording, video_writer
    
    if not is_recording and check_esp32_connection():
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{VIDEO_FOLDER}/recording_{timestamp}.avi"
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        video_writer = cv2.VideoWriter(filename, fourcc, 10.0, (320, 240))
        is_recording = True
        
        def recording_thread():
            global is_recording, video_writer
            while is_recording:
                img = get_esp32_cam_image()
                if img is not None:
                    video_writer.write(cv2.resize(img, (320, 240)))
                time.sleep(0.1)
            video_writer.release()
        
        threading.Thread(target=recording_thread, daemon=True).start()
    
    return redirect(url_for('index'))

@app.route('/stop_recording')
def stop_recording():
    """Stop video recording"""
    global is_recording
    is_recording = False
    return redirect(url_for('index'))

@app.route('/update_interval', methods=['POST'])
def update_interval():
    """Update auto-capture interval"""
    global CAPTURE_INTERVAL
    try:
        new_interval = int(request.form['interval'])
        if 1 <= new_interval <= 60:
            CAPTURE_INTERVAL = new_interval
    except ValueError:
        pass
    return redirect(url_for('index'))

if __name__ == '__main__':
    # Start auto-capture thread
    threading.Thread(target=auto_capture_images, daemon=True).start()
    
    # Run Flask app
    app.run(host='0.0.0.0', port=5000, threaded=True)