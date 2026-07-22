/*
 * OTAManager.h - part of the ESP32-OTAManager library
 *
 * Copyright (C) 2025-2026 packerlschupfer
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file OTAManager.h
 * @brief Thread-safe Over-The-Air update manager for ESP32
 * @version 1.0.0
 * @date 2025-06-12
 * 
 * @details This library provides a simplified, thread-safe wrapper around ArduinoOTA
 * for ESP32 devices running FreeRTOS. It includes automatic network detection,
 * comprehensive error handling, and flexible callback configuration.
 * 
 * @note Thread Safety: All public methods are protected by mutex for safe use
 * in multi-threaded FreeRTOS environments.
 * 
 * @note Logging: This library uses LogInterface for zero-overhead logging.
 * By default, logs go to ESP-IDF. To use custom Logger, define USE_CUSTOM_LOGGER
 * in your application before including any libraries. For debug logging, define
 * OTAMANAGER_DEBUG.
 * 
 * @warning Requires FreeRTOS (included with ESP32 Arduino Core)
 * 
 * @copyright MIT License
 */
#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <MutexGuard.h>

// Include the configuration file
#include "OTAManagerConfig.h"

/**
 * @brief A manager class for ESP32 Over-The-Air updates
 *
 * This class provides methods to initialize, monitor, and manage OTA updates
 * on ESP32. It handles update events, progress reporting, and error handling.
 */
class OTAManager {
   public:
    /**
     * @brief User-provided function to check if network is available
     *
     * This type defines a function that the OTAManager can call to check
     * if the network is ready for OTA updates.
     */
    typedef bool (*NetworkCheckCallback)();

    /**
     * @brief Initialize the OTA update system
     *
     * @param hostname Optional hostname for OTA identification (defaults to OTA_HOSTNAME from
     * config)
     * @param password Optional password for OTA updates (defaults to OTA_PASSWORD from config)
     * @param port Optional port for OTA server (defaults to OTA_PORT from config)
     * @param networkCheckCb Optional callback to check network readiness (defaults to nullptr)
     */
    static void initialize(const char* hostname = OTA_HOSTNAME, const char* password = OTA_PASSWORD,
                           uint16_t port = OTA_PORT,
                           NetworkCheckCallback networkCheckCb = OTA_CALLBACK_NONE);

    /**
     * @brief Check for and process pending OTA updates
     *
     * This method should be called frequently in your main loop.
     * @note Thread-safe: Can be called from multiple tasks
     */
    static void handleUpdates();
    
    /**
     * @brief Check if OTA manager has been initialized
     *
     * @return true if initialized, false otherwise
     */
    static bool isInitialized();

    /**
     * @brief Set custom start callback
     *
     * @param cb Function to call when OTA update starts
     */
    static void setStartCallback(ArduinoOTAClass::THandlerFunction cb);

    /**
     * @brief Set custom end callback
     *
     * @param cb Function to call when OTA update ends
     */
    static void setEndCallback(ArduinoOTAClass::THandlerFunction cb);

    /**
     * @brief Set custom progress callback
     *
     * @param cb Function to call to report OTA update progress
     */
    static void setProgressCallback(ArduinoOTAClass::THandlerFunction_Progress cb);

    /**
     * @brief Set custom error callback
     *
     * @param cb Function to call when OTA update encounters an error
     */
    static void setErrorCallback(ArduinoOTAClass::THandlerFunction_Error cb);

   private:
    /**
     * @brief Check if the network is ready for OTA updates
     *
     * @return true if network is ready, false otherwise
     */
    static bool isNetworkReady();

    /**
     * @brief Handle errors that occur during OTA updates
     *
     * @param error The error code from ArduinoOTA
     */
    static void handleOTAError(const ota_error_t error);

    // Whether OTA has been initialized
    static bool initialized;

    // User-provided network check callback
    static NetworkCheckCallback networkCheckCallback;
    
    // Mutex for thread safety
    static SemaphoreHandle_t mutex;

    static void handleOTAProgress(unsigned int progress, unsigned int total);
};