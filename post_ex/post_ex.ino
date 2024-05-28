#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "jwjung";
const char* password = "01027075860";

// Server URL
const char* serverUrl = "http://192.168.137.1:8000/test"; // replace with your server URL

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // JSON data to send
    String jsonData = "{\"test\":\"Hello world!\", \"아무개\":181818}";
    
    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
  
  delay(10000); // send a request every 10 seconds
}

