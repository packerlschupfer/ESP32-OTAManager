# Update Plan for ESP32-Ethernet-OTA-TaskManager-Example

## Goal
Bring this example to the same level of improvements as the EthernetManager example.

## Required Updates

### 1. Error Handling for Network Disconnection
- [ ] Add network event callbacks (onEthernetConnected, onEthernetDisconnected, onEthernetStateChange)
- [ ] Add auto-reconnect configuration with exponential backoff
- [ ] Add periodic connection state verification
- [ ] Update LED patterns for connection states

### 2. Clean Shutdown Support
- [ ] Add global shutdown flag
- [ ] Add serial command interface (shutdown, status, reboot, help)
- [ ] Add handleShutdown() function
- [ ] Add checkSerialCommands() function
- [ ] Implement graceful task termination
- [ ] Use EthernetManager::disconnect()

### 3. Network Event Handling
- [ ] Add EthernetManager callbacks in setup()
- [ ] Implement callback functions with proper logging
- [ ] Update LED based on connection state

### 4. Configuration Validation
- [ ] Add static_assert for stack sizes (minimum 2048)
- [ ] Add static_assert for task priorities (1-24)
- [ ] Validate compile-time configuration

### 5. LED Support Made Optional
- [ ] Wrap all LED code in #ifdef ENABLE_STATUS_LED
- [ ] Add ENABLE_STATUS_LED define to ProjectConfig.h
- [ ] Update StatusLed class for optional compilation

### 6. Additional Updates
- [ ] Add LogInterfaceImpl.h include
- [ ] Use TaskManagerConfig.h instead of TaskManager.h
- [ ] Add EthernetManager::earlyInit()
- [ ] Update watchdog registration to avoid duplicate loopTask registration
- [ ] Add proper connection state tracking

### 7. OTA-Specific Considerations
- [ ] Ensure OTA task handles shutdown gracefully
- [ ] Stop OTA operations during shutdown
- [ ] Add OTA status to serial commands

## Implementation Order
1. Update ProjectConfig.h with new defines and validation
2. Update StatusLed to support optional compilation
3. Update main.cpp with all improvements
4. Test compilation and fix any issues
5. Document changes