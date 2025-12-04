// OTAWithLogger.ino
// Example of using OTAManager with custom Logger through LogInterface

// IMPORTANT: Define USE_CUSTOM_LOGGER to enable custom Logger for all libraries
#define USE_CUSTOM_LOGGER
#define OTAMANAGER_DEBUG  // Enable debug logging (optional)

#include <Arduino.h>
#include <WiFi.h>
#include <Logger.h>            // Include Logger
#include <LogInterfaceImpl.h>  // Include implementation
#include <OTAManager.h>     // Will use Logger through LogInterface

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
  
  // Initialize Logger (assuming it has an init method)
  // Logger::getInstance().init(...);
  
  Serial.println("OTAManager Example - With Logger.h Integration");
  
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
  // All OTA logs will now go through Logger instead of being silent
  OTAManager::initialize(
    "esp32-logger",     // Custom hostname
    "ota-password",     // Custom password
    3232,               // Default port
    checkNetworkConnection  // Custom network check function
  );
  
  // Set custom callbacks that also use Logger
  OTAManager::setStartCallback([]() {
    Logger::getInstance().log(ESP_LOG_INFO, "MyApp", "OTA Update starting!");
    // You might want to stop other tasks here
  });
  
  OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
    static unsigned int lastProgress = 0;
    unsigned int currentProgress = (progress * 100) / total;
    
    // Log every 10% to avoid flooding
    if (currentProgress >= lastProgress + 10) {
      Logger::getInstance().log(ESP_LOG_INFO, "MyApp", 
                               "OTA Progress: %u%%", currentProgress);
      lastProgress = currentProgress - (currentProgress % 10);
    }
  });
  
  OTAManager::setEndCallback([]() {
    Logger::getInstance().log(ESP_LOG_INFO, "MyApp", "OTA Update complete!");
    // System will restart automatically
  });
  
  OTAManager::setErrorCallback([](ota_error_t error) {
    Logger::getInstance().log(ESP_LOG_ERROR, "MyApp", 
                             "OTA Error occurred: %d", error);
  });
  
  Serial.println("Setup complete. OTA Manager initialized with Logger support.");
  Serial.printf("Flash this device using IP: %s or hostname: esp32-logger.local\n", 
                WiFi.localIP().toString().c_str());
  
  // With USE_CUSTOM_LOGGER and OTAMANAGER_DEBUG defined, you'll see logs like:
  // [DEBUG] OTAMgr: Initializing OTA Manager
  // [INFO] OTAMgr: OTA password protection enabled
  // [INFO] OTAMgr: OTA Manager initialized successfully
  // [DEBUG] OTAMgr: Waiting for OTA updates on 192.168.1.100:3232...
  // All logs go through your custom Logger with its formatting
}

void loop() {
  // Handle OTA updates
  OTAManager::handleUpdates();
  
  // Your application code here
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 30000) {
    Logger::getInstance().log(ESP_LOG_INFO, "MyApp", 
                             "Application running normally...");
    lastLog = millis();
  }
  
  delay(10);
}