/**
 * @file ThreadSafeOTA.ino
 * @brief Example demonstrating thread-safe OTA updates with FreeRTOS
 * 
 * This example shows how to use OTAManager in a multi-threaded environment
 * with separate tasks for OTA handling and application logic.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <OTAManager.h>

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// Task handles
TaskHandle_t otaTaskHandle = NULL;
TaskHandle_t appTaskHandle = NULL;

// Shared state (protected by OTAManager's internal mutex)
volatile bool updateInProgress = false;

/**
 * @brief Task dedicated to handling OTA updates
 * 
 * This task runs on Core 0 and continuously checks for OTA updates
 */
void otaTask(void *pvParameters) {
    Serial.println("OTA Task started on Core 0");
    
    while (true) {
        // Handle OTA updates - thread-safe due to internal mutex
        OTAManager::handleUpdates();
        
        // Small delay to prevent watchdog issues
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief Main application task
 * 
 * This task runs on Core 1 and handles the main application logic
 */
void applicationTask(void *pvParameters) {
    Serial.println("Application Task started on Core 1");
    
    uint32_t counter = 0;
    
    while (true) {
        // Simulate application work
        if (!updateInProgress) {
            // Normal operation
            counter++;
            if (counter % 100 == 0) {
                Serial.printf("App running: counter = %u, Free heap: %u bytes\n", 
                            counter, ESP.getFreeHeap());
            }
        } else {
            // Pause critical operations during update
            Serial.println("Update in progress, pausing application...");
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nThreadSafeOTA Example - Multi-threaded OTA Updates");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize OTA with thread-safe configuration
    OTAManager::initialize(
        "esp32-threaded",      // Hostname
        "update123",           // Password
        3232,                  // Port
        []() -> bool {         // Network check lambda
            return WiFi.status() == WL_CONNECTED;
        }
    );
    
    // Set custom callbacks with thread-safe state updates
    OTAManager::setStartCallback([]() {
        Serial.println("\n=== OTA Update Starting ===");
        updateInProgress = true;
        
        // Optional: Suspend other tasks
        if (appTaskHandle != NULL) {
            Serial.println("Suspending application task...");
            vTaskSuspend(appTaskHandle);
        }
    });
    
    OTAManager::setEndCallback([]() {
        Serial.println("=== OTA Update Complete ===");
        updateInProgress = false;
        
        // Resume suspended tasks before restart
        if (appTaskHandle != NULL) {
            vTaskResume(appTaskHandle);
        }
        
        Serial.println("Restarting in 2 seconds...");
        delay(2000);
        ESP.restart();
    });
    
    OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
        static uint8_t lastPercent = 0;
        uint8_t percent = (progress * 100) / total;
        
        // Only print on 10% increments to reduce serial spam
        if (percent >= lastPercent + 10) {
            Serial.printf("Update Progress: %u%%\n", percent);
            lastPercent = percent - (percent % 10);
        }
    });
    
    OTAManager::setErrorCallback([](ota_error_t error) {
        Serial.printf("OTA Error [%u]: ", error);
        updateInProgress = false;
        
        // Resume tasks on error
        if (appTaskHandle != NULL) {
            vTaskResume(appTaskHandle);
        }
        
        switch (error) {
            case OTA_AUTH_ERROR:
                Serial.println("Auth Failed");
                break;
            case OTA_BEGIN_ERROR:
                Serial.println("Begin Failed");
                break;
            case OTA_CONNECT_ERROR:
                Serial.println("Connect Failed");
                break;
            case OTA_RECEIVE_ERROR:
                Serial.println("Receive Failed");
                break;
            case OTA_END_ERROR:
                Serial.println("End Failed");
                break;
            default:
                Serial.println("Unknown Error");
                break;
        }
    });
    
    // Create OTA task on Core 0
    xTaskCreatePinnedToCore(
        otaTask,              // Task function
        "OTA_Task",           // Task name
        4096,                 // Stack size
        NULL,                 // Parameters
        1,                    // Priority
        &otaTaskHandle,       // Task handle
        0                     // Core 0
    );
    
    // Create application task on Core 1
    xTaskCreatePinnedToCore(
        applicationTask,      // Task function
        "App_Task",           // Task name
        4096,                 // Stack size
        NULL,                 // Parameters
        1,                    // Priority
        &appTaskHandle,       // Task handle
        1                     // Core 1
    );
    
    Serial.println("\nSetup complete!");
    Serial.printf("Upload firmware to: %s.local or %s\n", 
                  "esp32-threaded", WiFi.localIP().toString().c_str());
    Serial.println("Password: update123");
    
    // Delete the setup task
    vTaskDelete(NULL);
}

void loop() {
    // Empty - all work is done in tasks
}