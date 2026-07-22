# OTAManager Test Suite

This directory contains comprehensive unit tests for the OTAManager library, with a focus on thread safety and concurrent access scenarios.

## Test Coverage

### Thread Safety Tests (`test_thread_safety.cpp`)

1. **Concurrent Initialization Test**
   - Verifies multiple threads can safely call `initialize()` simultaneously
   - Tests mutex protection of static members
   - Ensures no race conditions during initialization

2. **Concurrent Handle Updates Test**
   - Tests multiple threads calling `handleUpdates()` concurrently
   - Verifies thread-safe access to internal state
   - Checks initialization state remains consistent

3. **Concurrent Callback Modifications**
   - Tests setting callbacks from multiple threads simultaneously
   - Verifies no corruption of callback pointers
   - Ensures callbacks are properly protected by mutex

4. **Init/Handle Race Conditions**
   - Tests rapid initialization while other threads call `handleUpdates()`
   - Verifies no deadlocks or race conditions
   - Tests mixed operations from different threads

5. **Parameter Validation Thread Safety**
   - Tests null parameter handling with concurrent access
   - Verifies error paths are thread-safe
   - Ensures invalid parameters don't cause crashes

### Stress Tests (`test_stress.cpp`)

1. **Concurrent Stress Test**
   - Runs 10 concurrent tasks for 30 seconds
   - Performs random OTA operations continuously
   - Monitors memory usage and operation success rate
   - Verifies no memory leaks under heavy load

2. **Rapid Init/Deinit Cycles**
   - Tests 1000 rapid initialization cycles
   - Varies parameters to test different code paths
   - Monitors heap usage for memory leaks
   - Ensures stable operation over many cycles

## Running the Tests

### Prerequisites

- PlatformIO Core installed (`pip install platformio`)
- ESP32 board connected via USB
- Serial monitor for viewing test output

### Quick Start

```bash
# Run all tests
./run_tests.sh

# Run specific test suite
pio test -e esp32-thread-safety-tests
pio test -e esp32-stress-tests

# Run with verbose output
pio test -e esp32-thread-safety-tests -v
```

### Test Environments

- `esp32-thread-safety-tests`: Runs thread safety unit tests
- `esp32-stress-tests`: Runs stress tests with heavy concurrent load
- `esp32s3-tests`: Tests on ESP32-S3 variant
- `esp32-minimal`: Tests with minimal configuration

## Test Output

Tests use the Unity test framework and provide detailed output:

```
=== OTAManager Thread Safety Tests ===

Testing concurrent initialization from multiple threads...
✓ Concurrent initialization test passed

Testing concurrent handleUpdates calls...
✓ Concurrent handleUpdates test passed

Testing concurrent callback modifications...
✓ Concurrent callback modification test passed

Testing initialization/handle race conditions...
✓ Init/handle race condition test passed

Testing parameter validation with concurrent access...
✓ Parameter validation thread safety test passed

-----------------------
5 Tests 0 Failures 0 Ignored
OK
```

## Interpreting Results

### Success Indicators
- All tests show "✓" checkmarks
- No memory leaks detected
- Stress test completes without crashes
- Operations per second remains stable

### Common Issues
- **Memory Leaks**: Check heap difference in stress tests
- **Deadlocks**: Test will timeout if mutex deadlock occurs
- **Race Conditions**: Random failures or inconsistent results
- **Low Heap**: Minimum heap drops below 10KB threshold

## Adding New Tests

To add new thread safety tests:

1. Create a test function in the appropriate test file
2. Use FreeRTOS tasks to simulate concurrent access
3. Use Unity assertions to verify expected behavior
4. Add the test to the test runner with `RUN_TEST()`

Example:
```cpp
void test_new_concurrent_scenario() {
    TEST_MESSAGE("Testing new concurrent scenario...");
    
    // Create multiple tasks
    // Perform concurrent operations
    // Verify results with TEST_ASSERT_*
    
    TEST_MESSAGE("✓ New concurrent scenario passed");
}
```

## Continuous Integration

These tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
test:
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v2
    - name: Set up Python
      uses: actions/setup-python@v2
    - name: Install PlatformIO
      run: pip install platformio
    - name: Run tests
      run: cd test && ./run_tests.sh
```

## Troubleshooting

### Test Failures
1. Check serial output for detailed error messages
2. Increase stack size if stack overflow occurs
3. Verify ESP32 has sufficient free heap (>50KB recommended)
4. Check mutex initialization in OTAManager

### Performance Issues
1. Reduce `STRESS_TEST_TASKS` if board struggles
2. Increase task delays in stress tests
3. Use ESP32-S3 or newer for better performance
4. Monitor temperature during stress tests