# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-12-04

### Added
- Initial public release
- ArduinoOTA-based firmware update manager
- Static interface with thread-safe operations
- WiFi and Ethernet network support
- Event callbacks (onStart, onProgress, onEnd, onError)
- Password-protected updates for security
- Progress tracking with percentage reporting
- Automatic network type detection
- Thread-safe using FreeRTOS mutex
- Configurable port (default: 3232) and hostname
- Update validation and error handling
- WiFi can be disabled via CONFIG_WIFI_ENABLED macro
- Network state awareness with custom callbacks

Platform: ESP32 (Arduino framework)
License: MIT
Dependencies: MutexGuard

### Notes
- Production-tested for remote firmware updates over Ethernet
- Previous internal versions (v1.x) not publicly released
- Reset to v0.1.0 for clean public release start
