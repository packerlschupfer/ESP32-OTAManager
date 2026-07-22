# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
OTAManager is a thread-safe ESP32 library for managing Over-The-Air updates via WiFi or Ethernet. The library uses FreeRTOS mutex protection and provides a simple static interface.

## Build and Test Commands

### Testing
```bash
# Run all tests
cd test
./run_tests.sh

# Run specific test suites
pio test -e esp32-thread-safety-tests
pio test -e esp32-stress-tests

# Run minimal tests
pio test -e esp32-minimal
```

### Building Examples
```bash
# Basic WiFi OTA
pio run -e esp32-ota-basic

# Ethernet-only OTA
pio run -e esp32-ota-ethernet

# Production build (requires environment variables)
export PROD_WIFI_SSID="YourSSID"
export PROD_WIFI_PASS="YourPassword"
export PROD_OTA_PASS="OTAPassword"
pio run -e esp32-ota-production

# OTA Upload
pio run -e esp32-ota-basic --target uploadota
```

## Architecture

### Core Components
- **OTAManager class** (`src/OTAManager.h/cpp`): Static singleton with thread-safe operations using FreeRTOS mutex
- **Configuration** (`src/OTAManagerConfig.h`): Compile-time settings and defaults
- **Network abstraction**: Supports WiFi, Ethernet, or custom network implementations

### Key Design Patterns
1. **Static Interface**: All methods are static, no instantiation needed
2. **Thread Safety**: All public methods are protected by mutex (except `isInitialized()`)
3. **Callback System**: Event-driven architecture for OTA progress, errors, and completion
4. **Network Flexibility**: WiFi can be completely disabled via `CONFIG_WIFI_ENABLED 0`

### Testing Strategy
- Unity framework for unit testing
- Thread safety tests with concurrent task execution
- Stress tests with 10 parallel OTA operations
- Memory leak detection
- Multiple ESP32 variant support

### Configuration Approaches
1. **Basic**: Simple WiFi setup in Arduino IDE
2. **Advanced**: Multi-environment PlatformIO configurations
3. **Production**: Environment variables for sensitive data
4. **Ethernet-only**: Complete WiFi disabling for wired-only deployments

## Development Notes

### Adding New Features
- Maintain thread safety by using the existing mutex pattern
- Add corresponding unit tests in `test/`
- Update examples to demonstrate new functionality
- Follow existing error handling patterns with callbacks

### Testing Changes
- Always run thread safety tests after modifications
- Test on both ESP32 and ESP32-S3 if possible
- Verify memory usage doesn't increase (check stress tests)

### Common Tasks
- **Add new callback**: Update header, add setter method, invoke in appropriate location
- **Modify network behavior**: Check both WiFi and Ethernet examples
- **Debug OTA issues**: Enable verbose logging, check callback implementations
- **Production deployment**: Use environment-based configurations, not hardcoded values