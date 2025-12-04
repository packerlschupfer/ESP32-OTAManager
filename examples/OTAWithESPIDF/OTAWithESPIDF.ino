// OTAWithESPIDF.ino
// Example of using OTAManager with default ESP-IDF logging

#include <Arduino.h>
#include <WiFi.h>
#include <OTAManager.h>  // Will use ESP-IDF logging by default

// Enable debug logging for OTAManager
#define OTAMANAGER_DEBUG

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// Custom network check function
bool checkNetworkConnection() {
  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("OTAManager Example - With ESP-IDF Logging");
  
  // Set ESP-IDF log level (optional)
  esp_log_level_set("*", ESP_LOG_INFO);        // Default level for all tags
  esp_log_level_set("OTAMgr", ESP_LOG_DEBUG);  // Debug level for OTAManager
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print WiFi connection details
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  
  // Initialize OTA with custom settings
  // All OTA logs will go through ESP-IDF logging (visible in Serial)
  OTAManager::initialize(
    "esp32-espidf",     // Custom hostname
    "ota-password",     // Custom password
    3232,               // Default port
    checkNetworkConnection  // Custom network check function
  );
  
  // Set custom callbacks
  OTAManager::setStartCallback([]() {
    // ESP-IDF logging from callbacks
    ESP_LOGI("MyApp", "OTA Update starting!");
  });
  
  OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
    static unsigned int lastProgress = 0;
    unsigned int currentProgress = (progress * 100) / total;
    
    // Log every 10% to avoid flooding
    if (currentProgress >= lastProgress + 10) {
      ESP_LOGI("MyApp", "OTA Progress: %u%%", currentProgress);
      lastProgress = currentProgress - (currentProgress % 10);
    }
  });
  
  OTAManager::setEndCallback([]() {
    ESP_LOGI("MyApp", "OTA Update complete! Rebooting...");
  });
  
  OTAManager::setErrorCallback([](ota_error_t error) {
    ESP_LOGE("MyApp", "OTA Error occurred: %d", error);
  });
  
  Serial.println("Setup complete. OTA Manager initialized with ESP-IDF logging.");
  Serial.printf("Flash this device using IP: %s or hostname: esp32-espidf.local\n", 
                WiFi.localIP().toString().c_str());
  
  // You'll see ESP-IDF formatted logs like:
  // D (1234) OTAMgr: Initializing OTA Manager
  // I (1235) OTAMgr: OTA password protection enabled
  // I (1236) OTAMgr: OTA Manager initialized successfully
  // D (2000) OTAMgr: Waiting for OTA updates on 192.168.1.100:3232...
}

void loop() {
  // Handle OTA updates
  OTAManager::handleUpdates();
  
  // Your application code here
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 30000) {
    ESP_LOGI("MyApp", "Application running normally...");
    lastLog = millis();
  }
  
  delay(10);
}