// AdvancedLogging.ino
// Demonstrates the advanced logging features of OTAManager

#include <Arduino.h>
#include <WiFi.h>
#include <OTAManager.h>

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("OTAManager Advanced Logging Example");
  Serial.println("=====================================");
  Serial.println();
  
  // Demonstrate different logging configurations:
  // 1. Basic: Only Error, Warning, Info logs
  // 2. Debug: Add -DOTAMANAGER_DEBUG to platformio.ini
  // 3. Network Debug: Add -DOTAMANAGER_DEBUG_NETWORK
  // 4. Progress Debug: Add -DOTAMANAGER_DEBUG_PROGRESS
  
  Serial.println("Current build configuration:");
  #ifdef USE_CUSTOM_LOGGER
    Serial.println("- Using custom Logger");
  #else
    Serial.println("- Using ESP-IDF logging");
  #endif
  
  #ifdef OTAMANAGER_DEBUG
    Serial.println("- Debug logging ENABLED");
  #else
    Serial.println("- Debug logging DISABLED");
  #endif
  
  #ifdef OTAMANAGER_DEBUG_NETWORK
    Serial.println("- Network debug ENABLED");
  #endif
  
  #ifdef OTAMANAGER_DEBUG_PROGRESS
    Serial.println("- Progress debug ENABLED");
  #endif
  
  Serial.println();
  
  // Set ESP-IDF log levels for demonstration
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("OTAMgr", ESP_LOG_VERBOSE);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected!");
  
  // Initialize OTA
  // This will produce different log output based on build flags
  OTAManager::initialize("ota-debug-demo", "password");
  
  // Set callbacks to show progress logging
  OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
    // This demonstrates the difference between:
    // - Basic progress (always shown as INFO)
    // - Detailed progress (only with OTAMANAGER_DEBUG_PROGRESS)
  });
  
  Serial.println("\nOTA Manager initialized. Monitor logs to see the difference.");
  Serial.println("\nTo see more logs, add these to platformio.ini:");
  Serial.println("build_flags = ");
  Serial.println("    -DOTAMANAGER_DEBUG");
  Serial.println("    -DOTAMANAGER_DEBUG_NETWORK");
  Serial.println("    -DOTAMANAGER_DEBUG_PROGRESS");
}

void loop() {
  // Handle OTA updates
  // Network checks will produce debug logs if OTAMANAGER_DEBUG_NETWORK is enabled
  OTAManager::handleUpdates();
  
  // Simulate some activity
  static unsigned long lastActivity = 0;
  if (millis() - lastActivity > 30000) {
    Serial.println("Still running...");
    lastActivity = millis();
  }
  
  delay(10);
}

/* Example platformio.ini configurations:

[env:basic]
; Production mode - minimal logging
platform = espressif32
board = esp32dev
framework = arduino

[env:debug]
; Development mode - all debug enabled
platform = espressif32
board = esp32dev
framework = arduino
build_flags = 
    -DOTAMANAGER_DEBUG
    -DOTAMANAGER_DEBUG_NETWORK
    -DOTAMANAGER_DEBUG_PROGRESS

[env:custom_logger_debug]
; Using custom logger with debug
platform = espressif32
board = esp32dev
framework = arduino
build_flags = 
    -DUSE_CUSTOM_LOGGER
    -DOTAMANAGER_DEBUG
lib_deps = 
    Logger

*/