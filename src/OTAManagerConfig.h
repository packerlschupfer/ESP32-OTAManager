// OTAManagerConfig.h
#pragma once

#include <Arduino.h>

// Use this file to customize your OTAManager settings
// You can copy this to your project folder and modify the values

#ifndef OTA_MANAGER_CONFIG_H
#define OTA_MANAGER_CONFIG_H

// WiFi support configuration
// Set CONFIG_WIFI_ENABLED to 0 in your project to disable WiFi dependencies
// This is useful for Ethernet-only projects
#ifndef CONFIG_WIFI_ENABLED
    #ifdef ARDUINO_ARCH_ESP32
        // Default to WiFi enabled for ESP32
        #define CONFIG_WIFI_ENABLED 1
    #else
        #define CONFIG_WIFI_ENABLED 0
    #endif
#endif

// Default OTA update check interval in milliseconds
#ifndef OTA_CHECK_INTERVAL_MS
#define OTA_CHECK_INTERVAL_MS 250
#endif

// Default interval for logging status messages
#ifndef OTA_LOG_INTERVAL_MS
#define OTA_LOG_INTERVAL_MS 60000
#endif

// Default interval for logging error messages
#ifndef OTA_ERROR_LOG_INTERVAL_MS
#define OTA_ERROR_LOG_INTERVAL_MS 10000
#endif

// Default timeout waiting for initialization
#ifndef OTA_INIT_TIMEOUT_MS
#define OTA_INIT_TIMEOUT_MS 5000
#endif

// Default password for OTA updates (empty = no password)
#ifndef OTA_PASSWORD
#define OTA_PASSWORD ""
#endif

// Default port for OTA updates
#ifndef OTA_PORT
#define OTA_PORT 3232
#endif

// Default hostname (can be overridden)
#ifndef OTA_HOSTNAME
#define OTA_HOSTNAME "esp32-ota"
#endif

// Callback function types - set to nullptr if not used
#ifndef OTA_CALLBACK_NONE
#define OTA_CALLBACK_NONE nullptr
#endif

// Allow user to provide their own network checker
#ifndef OTA_CUSTOM_NETWORK_CHECK
#define OTA_CUSTOM_NETWORK_CHECK 0
#endif

// Include the dedicated logging configuration
#include "OTAManagerLogging.h"

// Legacy macro support for backward compatibility
// These will be deprecated in a future version
#ifndef OTA_LOG_DEBUG
#define OTA_LOG_DEBUG(...) OTAM_LOG_D(__VA_ARGS__)
#endif

#ifndef OTA_LOG_INFO
#define OTA_LOG_INFO(...) OTAM_LOG_I(__VA_ARGS__)
#endif

#ifndef OTA_LOG_WARN
#define OTA_LOG_WARN(...) OTAM_LOG_W(__VA_ARGS__)
#endif

#ifndef OTA_LOG_ERROR
#define OTA_LOG_ERROR(...) OTAM_LOG_E(__VA_ARGS__)
#endif

#endif // OTA_MANAGER_CONFIG_H
