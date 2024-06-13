#include "Arduino.h"
#include "esp_camera.h"
#include "base64.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#define CAMERA_RESOLUTION FRAMESIZE_SVGA

#define NEO_PIN 6
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "Minme-Kim";
const char* password = "d}<4&WSG9c-Zz?k1z='V?o04F";
const char* serverUrl = "http://125.180.95.161:20090/create/color";

String httpPost(String serverUrl, String jsonData);
void modifyColor(const char* hexColor);
void hexToRGB(const char* hex, uint8_t &r, uint8_t &g, uint8_t &b);
bool initializeCamera();

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(1000);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi");
    return;
  }

  Serial.println("\nConnected to WiFi");

  if (!initializeCamera()) {
    Serial.println("Camera initialization failed");
    return;
  }

  pixels.begin();
  pixels.show();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected, attempting to reconnect...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(5000);
    return;
  }

  String base64Image = base64::encode((uint8_t*)fb->buf, fb->len);
  String jsonData = "{\"image\":\"" + base64Image + "\"}";
  Serial.println(base64Image);

  String hexColor = httpPost(serverUrl, jsonData);
  if (hexColor != NULL && hexColor.length() == 7) {
    modifyColor(hexColor.c_str());
  } else {
    Serial.println("Invalid HEX color received");
  }

  esp_camera_fb_return(fb);
  delay(5000);
}

String httpPost(String serverUrl, String jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      http.end();
      return response;
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      http.end();
      return "";
    }
  } else {
    Serial.println("WiFi Disconnected");
    return "";
  }
}

void modifyColor(const char* hexColor) {
  uint8_t r, g, b;
  hexToRGB(hexColor, r, g, b);
  
  for(int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void hexToRGB(const char* hex, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (hex[0] == '#') {
    uint32_t color = strtoul(hex + 1, NULL, 16);
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
  } else {
    r = g = b = 0;
  }
}

bool initializeCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = CAMERA_RESOLUTION;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }

  sensor_t* s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  return true;
}