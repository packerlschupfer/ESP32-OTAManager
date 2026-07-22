// BasicOTAUpdates.ino
// Example of using the OTAManager library

#include <Arduino.h>
#include <WiFi.h>
#include <OTAManager.h>

// Optional: Override default logging behavior (for backward compatibility)
// By default, OTAManager uses LogInterface which routes to ESP-IDF logging
// You can still override the legacy macros if needed:
#define OTA_LOG_DEBUG(format, ...) Serial.printf("[DEBUG] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_INFO(format, ...)  Serial.printf("[INFO] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_WARN(format, ...)  Serial.printf("[WARN] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_ERROR(format, ...) Serial.printf("[ERROR] OTA: " format "\n", ##__VA_ARGS__)

// Note: The library now uses LogInterface for better flexibility.
// To use custom Logger instead, see the OTAWithLogger example.

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// Optional: Define custom OTA hostname and password
#define OTA_HOSTNAME "my-esp32-project"
#define OTA_PASSWORD "my-secret-password"

// Custom network check function
bool checkNetworkConnection() {
  // You can implement custom logic here
  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  Serial.println("OTAManager Example - Basic OTA Updates");
  
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
  OTAManager::initialize(
    OTA_HOSTNAME,       // Custom hostname
    OTA_PASSWORD,       // Custom password
    3232,               // Default port
    checkNetworkConnection  // Custom network check function
  );
  
  // Optional: Set custom callbacks
  OTAManager::setStartCallback([]() {
    Serial.println("Custom start callback: Update starting!");
    // You might want to stop other tasks here
  });
  
  OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
    Serial.printf("Custom progress callback: %u%%\n", (progress / (total / 100)));
    // You might want to display progress on an LCD/OLED screen
  });
  
  Serial.println("Setup complete. Waiting for OTA updates...");
  Serial.printf("Flash this device using IP: %s or hostname: %s.local\n", 
                WiFi.localIP().toString().c_str(), OTA_HOSTNAME);
}

void loop() {
  // Handle OTA updates
  OTAManager::handleUpdates();
  
  // Your application code here
  // ...
  
  delay(10);
}
