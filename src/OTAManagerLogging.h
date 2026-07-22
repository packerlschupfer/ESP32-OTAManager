/*
 * OTAManagerLogging.h - part of the ESP32-OTAManager library
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

#ifndef OTA_MANAGER_LOGGING_H
#define OTA_MANAGER_LOGGING_H

#define OTAM_LOG_TAG "OTAMgr"

// Define log levels based on debug flag
#ifdef OTAMANAGER_DEBUG
    // Debug mode: Show all levels
    #define OTAM_LOG_LEVEL_E ESP_LOG_ERROR
    #define OTAM_LOG_LEVEL_W ESP_LOG_WARN
    #define OTAM_LOG_LEVEL_I ESP_LOG_INFO
    #define OTAM_LOG_LEVEL_D ESP_LOG_DEBUG
    #define OTAM_LOG_LEVEL_V ESP_LOG_VERBOSE
#else
    // Release mode: Only Error, Warn, Info
    #define OTAM_LOG_LEVEL_E ESP_LOG_ERROR
    #define OTAM_LOG_LEVEL_W ESP_LOG_WARN
    #define OTAM_LOG_LEVEL_I ESP_LOG_INFO
    #define OTAM_LOG_LEVEL_D ESP_LOG_NONE  // Suppress
    #define OTAM_LOG_LEVEL_V ESP_LOG_NONE  // Suppress
#endif

// Route to custom logger or ESP-IDF
#ifdef USE_CUSTOM_LOGGER
    #include <LogInterface.h>
    #define OTAM_LOG_E(...) LOG_WRITE(OTAM_LOG_LEVEL_E, OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_W(...) LOG_WRITE(OTAM_LOG_LEVEL_W, OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_I(...) LOG_WRITE(OTAM_LOG_LEVEL_I, OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_D(...) LOG_WRITE(OTAM_LOG_LEVEL_D, OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_V(...) LOG_WRITE(OTAM_LOG_LEVEL_V, OTAM_LOG_TAG, __VA_ARGS__)
#else
    // ESP-IDF logging with compile-time suppression
    #include <esp_log.h>
    #define OTAM_LOG_E(...) ESP_LOGE(OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_W(...) ESP_LOGW(OTAM_LOG_TAG, __VA_ARGS__)
    #define OTAM_LOG_I(...) ESP_LOGI(OTAM_LOG_TAG, __VA_ARGS__)
    #ifdef OTAMANAGER_DEBUG
        #define OTAM_LOG_D(...) ESP_LOGD(OTAM_LOG_TAG, __VA_ARGS__)
        #define OTAM_LOG_V(...) ESP_LOGV(OTAM_LOG_TAG, __VA_ARGS__)
    #else
        #define OTAM_LOG_D(...) ((void)0)
        #define OTAM_LOG_V(...) ((void)0)
    #endif
#endif

// Optional: Network-specific debug logging
#ifdef OTAMANAGER_DEBUG_NETWORK
    #define OTAM_LOG_NET(...) OTAM_LOG_D("NET: " __VA_ARGS__)
#else
    #define OTAM_LOG_NET(...) ((void)0)
#endif

// Optional: Progress tracking debug
#ifdef OTAMANAGER_DEBUG_PROGRESS
    #define OTAM_LOG_PROG(...) OTAM_LOG_D("PROG: " __VA_ARGS__)
#else
    #define OTAM_LOG_PROG(...) ((void)0)
#endif

#endif // OTA_MANAGER_LOGGING_H