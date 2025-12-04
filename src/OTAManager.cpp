// OTAManager.cpp
#include "OTAManager.h"

#ifdef ESP32
    #if CONFIG_WIFI_ENABLED == 1
        #include <WiFi.h>
    #endif
#endif

// Initialize static members
bool OTAManager::initialized = false;
OTAManager::NetworkCheckCallback OTAManager::networkCheckCallback = nullptr;
SemaphoreHandle_t OTAManager::mutex = nullptr;

void OTAManager::initialize(const char* hostname, const char* password, uint16_t port,
                            NetworkCheckCallback networkCheckCb) {
    // Create mutex on first initialization
    if (!mutex) {
        mutex = xSemaphoreCreateMutex();
        if (!mutex) {
            OTAM_LOG_E("Failed to create mutex for OTA Manager");
            return;
        }
    }
    
    MutexGuard lock(mutex);
    
    OTAM_LOG_D("Initializing OTA Manager");
    
    // Validate parameters
    if (!hostname || strlen(hostname) == 0) {
        OTAM_LOG_E("Hostname cannot be null or empty");
        return;
    }
    
    // Validate port range (1-65535, with common OTA ports)
    if (port == 0) {
        OTAM_LOG_E("Port cannot be 0");
        return;
    }

    // Store the network check callback
    networkCheckCallback = networkCheckCb;

    // Configure ArduinoOTA
    ArduinoOTA.setHostname(hostname);

    if (password && strlen(password) > 0) {
        ArduinoOTA.setPassword(password);
        OTAM_LOG_I("OTA password protection enabled");
    } else {
        OTAM_LOG_W("OTA running without password protection");
    }

    if (port != 3232) {
        ArduinoOTA.setPort(port);
        OTAM_LOG_D("OTA port set to %u", port);
    }

    // Set default callbacks
    ArduinoOTA
        .onStart([]() {
            const char* type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
            OTAM_LOG_I("Start updating %s", type);
            (void)type; // Suppress unused warning when logging is disabled
        })
        .onEnd([]() {
            OTAM_LOG_I("Update complete. Rebooting...");
            delay(1000);
            ESP.restart();
        })
        .onProgress(handleOTAProgress)
        .onError([](ota_error_t error) { handleOTAError(error); });

    // Begin OTA server and check for success
    ArduinoOTA.begin();
    initialized = true;  // ArduinoOTA.begin() doesn't return error status

    OTAM_LOG_I("OTA Manager initialized successfully");
}

bool OTAManager::isInitialized() {
    MutexGuard lock(mutex);
    return initialized;
}

void OTAManager::handleUpdates() {
    // Quick check without lock for performance
    if (!initialized) {
        return;
    }
    
    // Protected static variables to avoid race conditions
    static unsigned long lastLog = 0;
    static unsigned long lastErrorLog = 0;

    if (isNetworkReady()) {
        MutexGuard lock(mutex);
        if (initialized) {  // Double-check with lock held
            ArduinoOTA.handle();  // must be called frequently (every few hundred ms)
        }

        unsigned long now = millis();
        if (now - lastLog >= OTA_LOG_INTERVAL_MS) {
            // Get IP address based on available network
            String ipAddr = "unknown";
            #if defined(ESP32) && defined(ETH)
                if (ETH.linkSpeed() > 0 && ETH.localIP() != IPAddress(0, 0, 0, 0)) {
                    ipAddr = ETH.localIP().toString();
                }
            #endif
            #if defined(ESP32) && CONFIG_WIFI_ENABLED == 1
                if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
                    ipAddr = WiFi.localIP().toString();
                }
            #endif
            
            OTAM_LOG_D("Waiting for OTA updates on %s:%u...", 
                         ipAddr.c_str(), OTA_PORT);
            lastLog = now;
        }
    } else {
        unsigned long now = millis();
        if (now - lastErrorLog >= OTA_ERROR_LOG_INTERVAL_MS) {
            OTAM_LOG_E("Network not connected, skipping OTA check");
            lastErrorLog = now;
        }
    }
}

void OTAManager::setStartCallback(ArduinoOTAClass::THandlerFunction cb) {
    MutexGuard lock(mutex);
    if (!initialized) {
        OTAM_LOG_W("Cannot set callback - OTA not initialized");
        return;
    }
    if (cb) {
        ArduinoOTA.onStart(cb);
        OTAM_LOG_D("Custom OTA start callback set");
    }
}

void OTAManager::setEndCallback(ArduinoOTAClass::THandlerFunction cb) {
    MutexGuard lock(mutex);
    if (!initialized) {
        OTAM_LOG_W("Cannot set callback - OTA not initialized");
        return;
    }
    if (cb) {
        ArduinoOTA.onEnd(cb);
        OTAM_LOG_D("Custom OTA end callback set");
    }
}

void OTAManager::setProgressCallback(ArduinoOTAClass::THandlerFunction_Progress cb) {
    MutexGuard lock(mutex);
    if (!initialized) {
        OTAM_LOG_W("Cannot set callback - OTA not initialized");
        return;
    }
    if (cb) {
        ArduinoOTA.onProgress(cb);
        OTAM_LOG_D("Custom OTA progress callback set");
    }
}

void OTAManager::setErrorCallback(ArduinoOTAClass::THandlerFunction_Error cb) {
    MutexGuard lock(mutex);
    if (!initialized) {
        OTAM_LOG_W("Cannot set callback - OTA not initialized");
        return;
    }
    if (cb) {
        ArduinoOTA.onError(cb);
        OTAM_LOG_D("Custom OTA error callback set");
    }
}

bool OTAManager::isNetworkReady() {
    // If user provided a custom network check function, use it
    MutexGuard lock(mutex);
    if (networkCheckCallback) {
        bool ready = networkCheckCallback();
        OTAM_LOG_NET("Custom network check returned: %s", ready ? "ready" : "not ready");
        return ready;
    }

    // Default implementation - check for common network types
#if defined(ESP32)
    #if defined(ETH)
    // Check Ethernet
    if (ETH.linkSpeed() > 0 && ETH.localIP() != IPAddress(0, 0, 0, 0)) {
        OTAM_LOG_NET("Ethernet connected: %s, speed: %d Mbps", 
                     ETH.localIP().toString().c_str(), ETH.linkSpeed());
        return true;
    }
    #endif
    
    #if CONFIG_WIFI_ENABLED == 1
    // Check WiFi
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
        OTAM_LOG_NET("WiFi connected: %s, RSSI: %d dBm", 
                     WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }
    #endif
#endif

    // No connection detected - warn once
    static bool warnedOnce = false;
    if (!warnedOnce) {
        OTAM_LOG_W("No network check callback provided and no network detected");
        warnedOnce = true;
    }
    return false;
}

void OTAManager::handleOTAError(const ota_error_t error) {
    switch (error) {
        case OTA_AUTH_ERROR:
            OTAM_LOG_E("Error[%u]: Auth Failed", error);
            break;
        case OTA_BEGIN_ERROR:
            OTAM_LOG_E("Error[%u]: Begin Failed", error);
            break;
        case OTA_CONNECT_ERROR:
            OTAM_LOG_E("Error[%u]: Connect Failed", error);
            break;
        case OTA_RECEIVE_ERROR:
            OTAM_LOG_E("Error[%u]: Receive Failed", error);
            break;
        case OTA_END_ERROR:
            OTAM_LOG_E("Error[%u]: End Failed", error);
            break;
        default:
            OTAM_LOG_E("Error[%u]: Unknown Error", error);
            break;
    }
}

// === PRIVATE STATIC ===

void OTAManager::handleOTAProgress(unsigned int progress, unsigned int total) {
    static int lastPrintedStep = -5;
    int currentProgress = (progress * 100) / total;

    if (currentProgress >= lastPrintedStep + 5) {
        OTAM_LOG_I("Progress: %u%%", currentProgress);
        lastPrintedStep = currentProgress - (currentProgress % 5);
    }
    
    // Detailed progress tracking for debugging
    OTAM_LOG_PROG("Bytes: %u/%u, Progress: %u%%, Speed: ~%.1f KB/s", 
                  progress, total, currentProgress, 
                  progress > 0 ? (float)progress / (millis() / 1000.0) / 1024.0 : 0);
}
