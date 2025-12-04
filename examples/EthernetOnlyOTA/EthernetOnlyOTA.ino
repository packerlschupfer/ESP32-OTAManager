/**
 * @file EthernetOnlyOTA.ino
 * @brief Example of using OTAManager with Ethernet only (WiFi disabled)
 * 
 * This example demonstrates how to use the OTAManager library in projects
 * where WiFi is disabled and only Ethernet connectivity is available.
 * 
 * Hardware: ESP32 with LAN8720 Ethernet PHY
 */

// IMPORTANT: Define this BEFORE including any libraries to disable WiFi
#define CONFIG_WIFI_ENABLED 0

#include <Arduino.h>
#include <ETH.h>
#include <OTAManager.h>

// Ethernet pins configuration for LAN8720
#define ETH_PHY_TYPE    ETH_PHY_LAN8720
#define ETH_PHY_ADDR    0
#define ETH_PHY_MDC     23
#define ETH_PHY_MDIO    18
#define ETH_PHY_POWER   -1
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT

// Static IP configuration (optional - remove for DHCP)
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Ethernet event handler
void onEthEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("Ethernet Started");
            ETH.setHostname("esp32-ethernet-ota");
            break;
            
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet Connected");
            break;
            
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("Ethernet Got IP: ");
            Serial.println(ETH.localIP());
            break;
            
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet Disconnected");
            break;
            
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("Ethernet Stopped");
            break;
    }
}

// Custom network check function for OTAManager
bool checkEthernetReady() {
    return ETH.linkSpeed() > 0 && ETH.localIP() != IPAddress(0, 0, 0, 0);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\nEthernet-Only OTA Example");
    Serial.println("WiFi is disabled in this build");
    
    // Register Ethernet event handler
    WiFiClass::onEvent(onEthEvent);
    
    // Start Ethernet
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, 
              ETH_PHY_TYPE, ETH_CLK_MODE);
    
    // Optional: Configure static IP
    // ETH.config(local_IP, gateway, subnet);
    
    // Wait for Ethernet to connect
    Serial.println("Waiting for Ethernet connection...");
    unsigned long startTime = millis();
    while (!checkEthernetReady() && millis() - startTime < 10000) {
        delay(100);
    }
    
    if (checkEthernetReady()) {
        Serial.println("Ethernet connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(ETH.localIP());
        
        // Initialize OTA with custom network check function
        OTAManager::initialize(
            "esp32-ethernet-ota",     // hostname
            "your-ota-password",      // password (optional)
            3232,                     // port
            checkEthernetReady        // network check callback
        );
        
        // Optional: Set custom callbacks
        OTAManager::setStartCallback([]() {
            Serial.println("\nOTA Update Starting...");
        });
        
        OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress * 100) / total);
        });
        
        OTAManager::setEndCallback([]() {
            Serial.println("\nOTA Update Complete! Rebooting...");
        });
        
        Serial.println("OTA Manager initialized");
        Serial.println("Ready for OTA updates via Ethernet");
    } else {
        Serial.println("Failed to connect Ethernet!");
    }
}

void loop() {
    // Handle OTA updates
    OTAManager::handleUpdates();
    
    // Your application code here
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 30000) {
        lastPrint = millis();
        if (checkEthernetReady()) {
            Serial.print("Ethernet OK - IP: ");
            Serial.println(ETH.localIP());
        } else {
            Serial.println("Ethernet disconnected");
        }
    }
    
    delay(10);
}