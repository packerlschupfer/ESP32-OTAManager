# PlatformIO Configuration Examples for OTAManager

This directory contains various PlatformIO configuration examples demonstrating different use cases and deployment scenarios for the OTAManager library.

## Configuration Files Overview

### 1. Basic Configuration (`platformio-basic.ini`)
- Minimal setup for WiFi-based OTA updates
- Simple configuration with hardcoded credentials
- Good starting point for development

**Key Features:**
- Basic WiFi connection
- Simple OTA setup
- Debug output enabled

**Usage:**
```bash
pio run -e esp32-ota-basic
pio run -e esp32-ota-basic --target upload
pio run -e esp32-ota-basic --target uploadota
```

### 2. Ethernet Configuration (`platformio-ethernet.ini`)
- Optimized for LAN8720 Ethernet PHY
- Static IP configuration options
- Reliable wired network updates

**Key Features:**
- Ethernet PHY pin configuration
- Optional static IP setup
- Extended timeout for stability

**Usage:**
```bash
pio run -e esp32-ota-ethernet
```

### 3. Advanced Configuration (`platformio-advanced.ini`)
- Multiple environment configurations
- Development, production, and test environments
- Support for different ESP32 variants (S3, C3)

**Key Features:**
- Environment-specific builds
- Custom logging macros
- Build-time variable injection
- Multiple board support

**Usage:**
```bash
# Development build
pio run -e esp32-ota-dev

# Production build
OTA_PROD_PASSWORD=secret123 pio run -e esp32-ota-prod

# Run tests
pio test -e esp32-ota-test
```

### 4. FreeRTOS Configuration (`platformio-freertos.ini`)
- Optimized for multi-threaded applications
- Task priority and core affinity settings
- Stack size optimization

**Key Features:**
- FreeRTOS configuration macros
- Task monitoring and debugging
- Core affinity settings
- Stack overflow protection

**Usage:**
```bash
# Standard FreeRTOS build
pio run -e esp32-ota-freertos

# Debug build with task monitoring
pio run -e esp32-ota-freertos-debug

# Memory-optimized build
pio run -e esp32-ota-freertos-minimal
```

### 5. Multi-Network Configuration (`platformio-multi-network.ini`)
- Support for multiple network types
- WiFi, Ethernet, and dual-network setups
- Access Point and Mesh network examples

**Key Features:**
- Network failover support
- Access Point mode for direct connection
- Mesh network OTA propagation
- Dual-network redundancy

**Usage:**
```bash
# WiFi primary
pio run -e esp32-ota-wifi-primary

# Ethernet primary
pio run -e esp32-ota-eth-primary

# Dual network with failover
pio run -e esp32-ota-dual-network

# Mesh network node
NODE_ID=node1 pio run -e esp32-ota-mesh
```

### 6. Production Configuration (`platformio-production.ini`)
- Security-hardened settings
- Monitoring and telemetry integration
- Fleet deployment support

**Key Features:**
- Environment variable configuration
- MQTT telemetry integration
- Secure boot and encryption
- A/B testing support
- Staggered update windows

**Usage:**
```bash
# Set environment variables
export PROD_WIFI_SSID="ProductionWiFi"
export PROD_WIFI_PASS="SecurePassword"
export PROD_OTA_PASS="OTAUpdatePassword"
export DEVICE_ID="prod-001"
export BUILD_TIMESTAMP=$(date +%s)
export GIT_COMMIT=$(git rev-parse --short HEAD)

# Build for production
pio run -e esp32-ota-production

# Deploy OTA update
DEVICE_IP=192.168.1.100 pio run -e esp32-ota-production --target uploadota
```

## Common Configuration Patterns

### Setting WiFi Credentials

**Option 1: In platformio.ini**
```ini
build_flags = 
    -D WIFI_SSID=\"YourNetwork\"
    -D WIFI_PASSWORD=\"YourPassword\"
```

**Option 2: Environment Variables**
```ini
build_flags = 
    -D WIFI_SSID=\"${sysenv.WIFI_SSID}\"
    -D WIFI_PASSWORD=\"${sysenv.WIFI_PASSWORD}\"
```

**Option 3: Separate Secrets File**
Create `platformio-secrets.ini`:
```ini
[env]
build_flags = 
    -D WIFI_SSID=\"SecretNetwork\"
    -D WIFI_PASSWORD=\"SecretPassword\"
```

### OTA Upload Configuration

**Basic OTA Upload:**
```ini
upload_protocol = espota
upload_port = device-hostname.local  ; or IP address
upload_flags = 
    --port=3232
    --auth=password123
```

**Advanced OTA Upload:**
```ini
upload_flags = 
    --port=3232
    --auth=${sysenv.OTA_PASSWORD}
    --timeout=60          ; Timeout in seconds
    --retry=3             ; Number of retries
    --progress            ; Show progress bar
```

### Partition Tables

**Standard OTA Partition:**
```csv
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000
otadata,  data, ota,     0xe000,  0x2000
app0,     app,  ota_0,   0x10000, 0x1E0000
app1,     app,  ota_1,   0x1F0000,0x1E0000
```

**Production Partition with Factory:**
```csv
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000
otadata,  data, ota,     0xe000,  0x2000
app0,     app,  ota_0,   0x10000, 0x1A0000
app1,     app,  ota_1,   0x1B0000,0x1A0000
factory,  app,  factory, 0x350000,0x80000
```

## Best Practices

1. **Security:**
   - Never commit credentials to version control
   - Use environment variables for sensitive data
   - Enable password protection for OTA updates
   - Consider secure boot in production

2. **Reliability:**
   - Implement network failover for critical systems
   - Use static IP for Ethernet deployments
   - Add health checks and monitoring
   - Configure appropriate timeouts

3. **Performance:**
   - Optimize build flags for production (-O2, -DNDEBUG)
   - Adjust FreeRTOS settings for your application
   - Monitor heap and stack usage
   - Use appropriate partition sizes

4. **Maintenance:**
   - Version your firmware properly
   - Keep build configuration in version control
   - Document environment-specific settings
   - Use CI/CD for automated builds

## Troubleshooting

### Common Issues:

1. **OTA Upload Fails:**
   - Check network connectivity
   - Verify hostname/IP address
   - Confirm password matches
   - Ensure port 3232 is not blocked

2. **Build Errors:**
   - Update PlatformIO Core: `pio upgrade`
   - Clean build: `pio run -t clean`
   - Check library dependencies
   - Verify environment variables are set

3. **Memory Issues:**
   - Reduce stack sizes if needed
   - Optimize partition table
   - Use -Os flag for size optimization
   - Monitor free heap during operation

## Integration with CI/CD

Example GitHub Actions workflow:
```yaml
name: Build and Deploy

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: Set up Python
      uses: actions/setup-python@v2
    - name: Install PlatformIO
      run: pip install platformio
    - name: Build firmware
      env:
        PROD_WIFI_SSID: ${{ secrets.WIFI_SSID }}
        PROD_WIFI_PASS: ${{ secrets.WIFI_PASS }}
        PROD_OTA_PASS: ${{ secrets.OTA_PASS }}
      run: |
        export BUILD_TIMESTAMP=$(date +%s)
        export GIT_COMMIT=${GITHUB_SHA::8}
        pio run -e esp32-ota-production
```

## Additional Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ArduinoOTA Library](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA)
- [OTAManager Documentation](../../README.md)