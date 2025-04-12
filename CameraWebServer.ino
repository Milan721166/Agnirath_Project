#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "FS.h"
#include "SD_MMC.h"

// Replace with your network credentials
const char* ssid = "Redmi Note 9 Pro";
const char* password = "00000000";

WebServer server(80);

// CAMERA_MODEL_AI_THINKER pin configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

unsigned long lastCaptureTime = 0;
const long captureInterval = 5000; // 5 seconds in milliseconds

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize SD card
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  
  // Use RGB565 format instead of JPEG
  config.pixel_format = PIXFORMAT_RGB565;
  
  // Reduced frame size
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.jpeg_quality = 12;
  config.fb_count = 2;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  // Start HTTP server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/capture", HTTP_GET, handleCapture);
  server.on("/status", HTTP_GET, [](){
    server.send(200, "text/plain", "OK");
  });
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  server.handleClient();
  
  // Auto-capture every 5 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - lastCaptureTime >= captureInterval) {
    captureToSD();
    lastCaptureTime = currentMillis;
  }
}

void captureToSD() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Convert to JPEG
  size_t jpg_buf_len = 0;
  uint8_t *jpg_buf = NULL;
  bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
  
  if (!jpeg_converted) {
    Serial.println("JPEG conversion failed");
    esp_camera_fb_return(fb);
    return;
  }

  // Save to SD card
  String path = "/image_" + String(millis()) + ".jpg";
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(jpg_buf, jpg_buf_len);
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();

  free(jpg_buf);
  esp_camera_fb_return(fb);
}

void handleRoot() {
  String html = "<html><head><title>ESP32-CAM</title>";
  html += "<style>body { font-family: Arial; text-align: center; margin-top: 20px; }";
  html += "button { background-color: #4CAF50; border: none; color: white; padding: 10px 20px;";
  html += "text-align: center; text-decoration: none; display: inline-block; font-size: 16px;";
  html += "margin: 4px 2px; cursor: pointer; border-radius: 4px; }</style></head>";
  html += "<body><h1>ESP32-CAM Controller</h1>";
  html += "<img id='stream' src='/stream' style='width: 320px; height: 240px;'><br>";
  html += "<button onclick='capture()'>Capture Image</button>";
  html += "<p>Auto-capturing to SD card every 5 seconds</p>";
  html += "<script>function capture() {";
  html += "var img = document.getElementById('stream');";
  html += "img.src = '/capture?' + new Date().getTime();";
  html += "setTimeout(function(){ img.src = '/stream?' + new Date().getTime(); }, 1000);";
  html += "}</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleStream() {
  WiFiClient client = server.client();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }

    // Convert RGB565 to JPEG
    size_t jpg_buf_len = 0;
    uint8_t *jpg_buf = NULL;
    bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
    
    if (!jpeg_converted) {
      Serial.println("JPEG conversion failed");
      esp_camera_fb_return(fb);
      continue;
    }

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    client.write((char *)jpg_buf, jpg_buf_len);
    server.sendContent("\r\n");

    free(jpg_buf);
    esp_camera_fb_return(fb);
    delay(16);
  }
}

void handleCapture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  // Convert to JPEG
  size_t jpg_buf_len = 0;
  uint8_t *jpg_buf = NULL;
  bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
  
  if (!jpeg_converted) {
    server.send(500, "text/plain", "JPEG conversion failed");
    esp_camera_fb_return(fb);
    return;
  }

  // Create a String object from the buffer
  String imageData;
  imageData.reserve(jpg_buf_len);
  for (size_t i = 0; i < jpg_buf_len; i++) {
    imageData += (char)jpg_buf[i];
  }

  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(jpg_buf_len));
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "image/jpeg", imageData);
  
  free(jpg_buf);
  esp_camera_fb_return(fb);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}