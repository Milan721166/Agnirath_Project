README for ESP32-CAM Web Server Project

Overview
This project creates a web server on an ESP32-CAM (AI Thinker) board that streams video and allows camera control through a web interface. The setup involves both Arduino IDE for the ESP32 firmware and a Python web application for enhanced functionality.

Project Structure
esp32-cam-webserver/
├── arduino/                  # ESP32-CAM firmware files
│   ├── CameraWebServer.ino   # Main Arduino sketch
│   ├── app_httpd.cpp         # HTTP server implementation
│   ├── camera_index.h        # HTML/CSS/JS for web interface
│   ├── camera_pins.h         # Pin configuration for ESP32-CAM
│   └── ci.json              # Configuration file
├── app.py                    # Python web application
└── README.md                 # This file

Hardware Requirements
- ESP32-CAM (AI Thinker) board with ESP32-CAM MB
- USB Type B(for programming)
- 5V power supply

Setup Instructions

1. Arduino IDE Setup (for ESP32-CAM)

1. Install Arduino IDE (1.8.x or newer)
2. Add ESP32 board support:
   - Go to File > Preferences
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to Additional Boards Manager URLs
   - Go to Tools > Board > Boards Manager, search for "esp32" and install
3. Select the correct board:
   - Board: "AI Thinker ESP32-CAM"
   - Flash Mode: "QIO"
   - Flash Frequency: "80MHz"
   - Upload Speed: "115200"
   - Port: Select your COM port
4. Install required libraries:
   - ESP32 Camera (through Library Manager)
   - WiFi (included with ESP32 board package)

5. Upload the firmware:
   - Open `CameraWebServer.ino` in Arduino IDE
   - Update WiFi credentials in the sketch
   - Upload to the ESP32-CAM

 2. Python Web Application Setup

1. Install Python 3.8 or newer
2. Create a virtual environment (recommended):
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```
3. Install required Python packages:
   ```bash
   pip install flask flask-socketio eventlet opencv-python
   ```
4. Run the application:
   ```bash
   python app.py
   ```
5. Access the web interface at `http://localhost:5000`

File Descriptions

 Arduino Files
- CameraWebServer.ino: Main sketch that initializes camera and WiFi
- app_httpd.cpp: Implements the HTTP server and streaming functionality
- camera_index.h: Contains the HTML/CSS/JS for the web interface
- camera_pins.h: Defines pin mappings for the ESP32-CAM board
- ci.json: Configuration file for camera settings

Python Files
- app.py: Flask application that provides enhanced web interface and controls

Usage

1. After uploading the Arduino sketch, note the IP address printed in the Serial Monitor
2. Run `app.py` to start the local web server
3. Open a browser and navigate to `http://localhost:5000`
4. The interface will:
   - Show live video stream from ESP32-CAM
   - Allow camera configuration (resolution, quality, etc.)
   - Provide motion detection controls (if implemented)

Troubleshooting

1. Upload issues with ESP32-CAM:
   - Ensure proper connections between ESP32 and programmer
   - Hold "BOOT" button while uploading
   - Check baud rate and COM port settings

2. No video stream:
   - Verify ESP32 is connected to WiFi
   - Check power supply (5V with sufficient current)
   - Verify camera module is properly seated

3. Python app connection issues:
   - Ensure ESP32 and computer are on the same network
   - Update IP address in `app.py` if needed

Customization

- Modify `camera_index.h` to change the web interface
- Adjust camera settings in `ci.json`
- Extend functionality in `app.py` with additional features

License
This project is open-source. Modify and distribute as needed. Devlop By Agnirath 
