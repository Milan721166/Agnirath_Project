![image](https://github.com/user-attachments/assets/4473214c-05ca-451a-8d55-ea70ffb700cd)
Arduino IDE Setup for ESP32-CAM
1. Install Arduino IDE
Download and install the latest Arduino IDE from arduino.cc.

2. Add ESP32 Board Support
Open Arduino IDE.

Go to File → Preferences.

In Additional Boards Manager URLs, paste:

https://dl.espressif.com/dl/package_esp32_index.json
Click OK.

3. Install ESP32 Board Package
Go to Tools → Board → Boards Manager.

Search for "esp32".

Install "ESP32 by Espressif Systems".

Wait for installation to complete.

Select Correct Board & Settings
Go to Tools → Board → ESP32 Arduino.

Select "AI Thinker ESP32-CAM".

Set the following:

Flash Mode: QIO

Flash Frequency: 80MHz

Upload Speed: 115200

Port: Select the COM port your ESP32-CAM is connected to.

Partition Scheme: Default (or Minimal SPIFFS if needed)

Install Required Libraries
Go to Sketch → Include Library → Manage Libraries.

Search and install:

ESP32 Camera (by Espressif Systems)

WiFi (should be included with ESP32 board package)

Upload the Code to ESP32-CAM
1. Open CameraWebServer.ino
Open the main sketch file in Arduino IDE.

2. Configure WiFi Credentials
Modify these lines in the code:

cpp
Copy
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
3. Connect ESP32-CAM to USB-to-Serial Adapter
Ensure proper wiring (check pin connections below).

4. Enter Bootloader Mode
Hold the "BOOT" button on the ESP32-CAM.

Press the "RESET" button once while still holding "BOOT".

Release "BOOT" after reset.

5. Upload the Sketch
Click the Upload (→) button in Arduino IDE.

Wait for compilation and upload to complete.

6. Check Serial Monitor (Optional)
Open Tools → Serial Monitor (Baud rate: 115200).

You should see:

Copy
Connecting to WiFi...
WiFi connected
Camera Ready! Use 'http://<ESP_IP>' to connect
Note the IP address for accessing the web server.


Troubleshooting Upload Issues
"Failed to connect to ESP32"

Check wiring (TX/RX should be crossed).

Hold "BOOT" button during upload.

Try lowering upload speed (921600 → 115200).

No COM Port Detected

Install correct CP2102/CH340 drivers for your USB adapter.

Random Crashes After Upload

Ensure stable 5V power supply (USB may not be enough; use external power).

Next Steps
After successful upload:

Open a web browser and enter the ESP32-CAM's IP.
You should see the live camera stream.
Run app.py (Python server) for enhanced features.

