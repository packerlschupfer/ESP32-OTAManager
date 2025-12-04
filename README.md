# OTAManager

A PlatformIO library for managing Over-The-Air (OTA) updates on ESP32 devices.

## Features

- Simple interface for initializing and handling OTA updates
- Flexible network integration (works with WiFi, Ethernet, or custom networks)
- WiFi can be completely disabled for Ethernet-only projects
- Customizable callback handling for update events
- Progress reporting and error handling
- Configurable settings
- Thread-safe operations with FreeRTOS mutex protection

## Installation

### PlatformIO

1. Add the library to your `platformio.ini` file:

```ini
lib_deps = 
    yourusername/OTAManager
```

2. Or install through the PlatformIO Library Manager

3. See [PlatformIO Examples](examples/PlatformIO-Examples/) for complete configuration examples

### Arduino IDE

1. Download this repository as a ZIP file
2. In Arduino IDE, go to Sketch > Include Library > Add .ZIP Library...
3. Select the downloaded ZIP file

## Usage

### Basic Example with WiFi

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <OTAManager.h>

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize OTA with default settings
  OTAManager::initialize();
  
  Serial.println("OTA updates enabled");
}

void loop() {
  // Handle OTA updates
  OTAManager::handleUpdates();
  
  // Your application code
  delay(10);
}
```

### Disabling WiFi for Ethernet-Only Projects

If your project only uses Ethernet and you want to completely disable WiFi dependencies:

```cpp
// Define this BEFORE including any libraries
#define CONFIG_WIFI_ENABLED 0

#include <Arduino.h>
#include <ETH.h>
#include <OTAManager.h>

// Custom network check for Ethernet
bool checkEthernetReady() {
    return ETH.linkSpeed() > 0 && ETH.localIP() != IPAddress(0, 0, 0, 0);
}

void setup() {
    // Initialize Ethernet...
    
    // Initialize OTA with Ethernet check
    OTAManager::initialize(
        "esp32-ethernet",
        "password",
        3232,
        checkEthernetReady
    );
}
```

See the [EthernetOnlyOTA example](examples/EthernetOnlyOTA/) for a complete implementation.

### Using with EthernetManager

The OTAManager can be easily integrated with the EthernetManager library:

```cpp
#include <Arduino.h>
#include <EthernetManager.h>
#include <OTAManager.h>

// Custom network check function
bool isEthernetConnected() {
  return EthernetManager::isConnected();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize Ethernet
  EthernetManager::initialize("esp32-device");
  EthernetManager::waitForConnection(10000);
  
  // Initialize OTA with Ethernet connection check
  OTAManager::initialize(
    "esp32-device",        // Hostname
    "update-password",     // OTA password
    3232,                  // Port
    isEthernetConnected    // Network check function
  );
}

void loop() {
  // Handle OTA updates
  OTAManager::handleUpdates();
  
  // Your application code
  delay(10);
}
```

### Custom Callbacks

You can set custom callbacks for OTA events:

```cpp
// Set custom start callback
OTAManager::setStartCallback([]() {
  Serial.println("Update starting, stopping sensors...");
  // Stop sensor readings or other operations
});

// Set custom progress callback
OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
  int percent = (progress / (total / 100));
  Serial.printf("Update progress: %u%%\n", percent);
  // Update a progress bar on a display
});

// Set custom end callback
OTAManager::setEndCallback([]() {
  Serial.println("Update complete, rebooting in 2 seconds");
  delay(2000);
  ESP.restart();
});

// Set custom error callback
OTAManager::setErrorCallback([](ota_error_t error) {
  Serial.printf("Update error: %u\n", error);
  // Handle errors, maybe try to retry
});
```

### Custom Configuration

You can customize the OTA settings by defining configuration macros before including the library:

```cpp
// Custom OTA configuration
#define OTA_HOSTNAME "my-custom-esp32"
#define OTA_PASSWORD "secure-update-password"
#define OTA_PORT 8266
#define OTA_CHECK_INTERVAL_MS 500

#include <OTAManager.h>
```

### Logging Configuration

The library supports flexible logging configuration with multiple debug levels:

#### Using ESP-IDF Logging (Default)
No configuration needed. The library will use ESP-IDF logging:

```cpp
#include <OTAManager.h>

void setup() {
    OTAManager::initialize();  // Logs to ESP-IDF
}
```

#### Using Custom Logger
Define `USE_CUSTOM_LOGGER` in your build flags:

```ini
[env:myproject]
build_flags = -DUSE_CUSTOM_LOGGER
lib_deps = 
    Logger      ; Required when USE_CUSTOM_LOGGER is defined
    OTAManager
```

In your code:
```cpp
#include <Logger.h>
#include <LogInterfaceImpl.h>
#include <OTAManager.h>

void setup() {
    Logger::getInstance().init(1024);
    OTAManager::initialize();  // Now logs through custom Logger
}
```

#### Debug Logging

Enable debug/verbose logging for this library:

```ini
build_flags = -DOTAMANAGER_DEBUG
```

#### Advanced Debug Options

For targeted debugging, you can enable specific debug features:

```ini
build_flags = 
    -DOTAMANAGER_DEBUG           ; Enable all debug logging
    -DOTAMANAGER_DEBUG_NETWORK   ; Network connection details
    -DOTAMANAGER_DEBUG_PROGRESS  ; Detailed progress tracking
```

#### Complete Example

```ini
[env:debug]
build_flags = 
    -DUSE_CUSTOM_LOGGER          ; Use custom logger
    -DOTAMANAGER_DEBUG           ; Enable debug for OTA Manager
    -DOTAMANAGER_DEBUG_NETWORK   ; Show network diagnostics

[env:production]
; No debug flags - only Error, Warning, Info logs
```

#### Log Levels

- **Error** (`OTAM_LOG_E`): Critical failures
- **Warning** (`OTAM_LOG_W`): Important issues that don't stop operation
- **Info** (`OTAM_LOG_I`): Major state changes and progress
- **Debug** (`OTAM_LOG_D`): Detailed operation info (only with OTAMANAGER_DEBUG)
- **Verbose** (`OTAM_LOG_V`): Very detailed traces (only with OTAMANAGER_DEBUG)

#### Legacy Custom Logging (Backward Compatibility)

For backward compatibility, you can still override the default logging macros:

```cpp
// Define custom macros BEFORE including OTAManager.h
#define OTA_LOG_DEBUG(format, ...) Serial.printf("[DEBUG] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_INFO(format, ...)  Serial.printf("[INFO] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_WARN(format, ...)  Serial.printf("[WARN] OTA: " format "\n", ##__VA_ARGS__)
#define OTA_LOG_ERROR(format, ...) Serial.printf("[ERROR] OTA: " format "\n", ##__VA_ARGS__)

#include <OTAManager.h>
```

See the [OTAWithLogger example](examples/OTAWithLogger/) for a complete implementation.

## API Reference

### Static Methods

#### `void initialize(const char* hostname = OTA_HOSTNAME, const char* password = OTA_PASSWORD, uint16_t port = OTA_PORT, NetworkCheckCallback networkCheckCb = OTA_CALLBACK_NONE)`

Initializes the OTA update system.

- **Parameters**
  - `hostname`: Hostname for OTA identification (default: from config)
  - `password`: Password for OTA updates (default: from config)
  - `port`: Port for OTA server (default: from config)
  - `networkCheckCb`: Callback to check network readiness (default: nullptr)

#### `void handleUpdates()`

Checks for and processes pending OTA updates. Should be called frequently in your main loop.

#### `bool isInitialized()`

Returns true if the OTA manager has been initialized, false otherwise. Thread-safe.

#### `void setStartCallback(ArduinoOTAClass::THandlerFunction cb)`

Sets a custom callback for when an OTA update starts.

#### `void setEndCallback(ArduinoOTAClass::THandlerFunction cb)`

Sets a custom callback for when an OTA update ends.

#### `void setProgressCallback(ArduinoOTAClass::THandlerFunction_Progress cb)`

Sets a custom callback for OTA update progress reporting.

#### `void setErrorCallback(ArduinoOTAClass::THandlerFunction_Error cb)`

Sets a custom callback for OTA update errors.

## Testing

The library includes a comprehensive test suite focusing on thread safety and concurrent access scenarios. See the [test directory](test/) for details.

### Running Tests

```bash
cd test
./run_tests.sh
```

### Test Coverage
- Thread safety with multiple concurrent tasks
- Stress testing under heavy load
- Memory leak detection
- Race condition detection
- Parameter validation

## Suggested Improvements

### Critical Priority
1. **Thread Safety**: Add mutex protection for static member variables to prevent race conditions in multi-threaded environments
2. **Null Parameter Validation**: Add null checks for hostname parameter in initialize()
3. **Error Handling**: Check return values from ArduinoOTA operations and handle failures gracefully
4. **Network Detection**: Improve default network check to support WiFi and warn when no network check is available

### High Priority
5. **Initialization State Checks**: Validate OTA is initialized before allowing callback setters
6. **Const Correctness**: Mark methods that don't modify state as const
7. **Static Variable Protection**: Protect static local variables in handleUpdates() from race conditions

### Medium Priority
8. **Flash String Optimization**: Use F() macro for string literals to save RAM
9. **API Documentation**: Add parameter validation documentation
10. **Example Enhancement**: Add thread-safe usage examples for FreeRTOS environments

## License

This library is released under the GPL-3 License.
