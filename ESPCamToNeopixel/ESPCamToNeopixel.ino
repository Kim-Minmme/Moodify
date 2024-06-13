#include "Arduino.h"
#include "esp_camera.h"
#include "base64.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#define CAMERA_RESOLUTION FRAMESIZE_SVGA

const char* ssid = "jwjung";
const char* password = "01027075860";
const char* serverUrl = "http://125.180.95.161:20090/create/color";

#define PIN 12
#define NUMPIXELS 12

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // WiFi 연결 시도
  Serial.print("Connecting to WiFi...");
  for (int retries = 0; WiFi.status() != WL_CONNECTED && retries < 20; retries++) {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi");
    return;
  }

  Serial.println("\nConnected to WiFi");

  // 카메라 초기화
  if (!initializeCamera()) {
    Serial.println("Camera initialization failed");
    return;
  }

  // 네오픽셀 초기화
  pixels.begin();
}

void loop() {
  // WiFi 상태 확인 및 재연결 시도
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected, attempting to reconnect...");
    WiFi.reconnect();
    delay(3000);
    return;
  }

  // 카메라 캡처
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(5000);
    return;
  }

  // 이미지 base64 인코딩 및 전송
  String base64Image = base64::encode((uint8_t*)fb->buf, fb->len);
  String jsonData = "{\"image\":\"" + base64Image + "\"}";
  Serial.println(base64Image);

  String hexColor = httpPost(serverUrl, jsonData);
  Serial.println(hexColor);

  // 네오픽셀 색상 설정
  setNeoPixelColor(hexColor);
  esp_camera_fb_return(fb);
  delay(1000);
}

String httpPost(const char* serverUrl, const String& jsonData) {
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

void setNeoPixelColor(const String& hexString) {
  uint8_t r = strtol(hexString.substring(2, 4).c_str(), NULL, 16);
  uint8_t g = strtol(hexString.substring(4, 6).c_str(), NULL, 16);
  uint8_t b = strtol(hexString.substring(6, 8).c_str(), NULL, 16);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
