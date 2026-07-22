// C11CompatibilityTest.ino
// Demonstrates that OTAManager works with C++11 and doesn't require C++17 features

#include <Arduino.h>
#include <WiFi.h>

// Test 1: Default ESP-IDF logging (no USE_CUSTOM_LOGGER defined)
#include <OTAManager.h>

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("OTAManager C++11 Compatibility Test");
  Serial.println("This example shows the library works without C++17 features");
  
  // Set ESP-IDF log level
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("OTAMgr", ESP_LOG_DEBUG);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected!");
  
  // Initialize OTA - logs will go to ESP-IDF
  OTAManager::initialize("esp32-c11test", "password");
  
  Serial.println("OTA Manager initialized with ESP-IDF logging");
  Serial.println("No LogInterface.h included, no Logger dependency");
  
  // You'll see ESP-IDF logs like:
  // I (1234) OTAMgr: OTA Manager initialized successfully
  // D (2000) OTAMgr: Waiting for OTA updates...
}

void loop() {
  OTAManager::handleUpdates();
  delay(10);
}

// To test with custom Logger:
// 1. Add to platformio.ini:
//    build_flags = -DUSE_CUSTOM_LOGGER
//    lib_deps = Logger
// 2. Modify this file to include Logger setup:
//    #define USE_CUSTOM_LOGGER
//    #include "Logger.h"
//    #include "LogInterfaceImpl.h"
//    // ... rest of code ...