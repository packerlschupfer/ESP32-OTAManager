/**
 * @file test_stress.cpp
 * @brief Stress tests for OTAManager under heavy concurrent load
 * 
 * These tests push the OTAManager to its limits with many concurrent
 * tasks performing various operations simultaneously.
 */

#include <Arduino.h>
#include <unity.h>
#include <OTAManager.h>
#include <esp_task_wdt.h>

// Stress test configuration
#define STRESS_TEST_TASKS 10
#define STRESS_TEST_DURATION_MS 30000  // 30 seconds
#define STRESS_OPERATIONS_PER_TASK 10000

// Test metrics
static volatile uint32_t totalOperations = 0;
static volatile uint32_t successfulOperations = 0;
static volatile uint32_t failedOperations = 0;
static volatile bool stressTestRunning = true;
static SemaphoreHandle_t metricsMutex = NULL;

// Memory tracking
static uint32_t initialFreeHeap = 0;
static uint32_t minFreeHeap = UINT32_MAX;

void updateMetrics(bool success) {
    xSemaphoreTake(metricsMutex, portMAX_DELAY);
    totalOperations++;
    if (success) {
        successfulOperations++;
    } else {
        failedOperations++;
    }
    
    // Track minimum free heap
    uint32_t currentHeap = ESP.getFreeHeap();
    if (currentHeap < minFreeHeap) {
        minFreeHeap = currentHeap;
    }
    xSemaphoreGive(metricsMutex);
}

/**
 * @brief Stress test task - performs random OTA operations
 */
void stressTestTask(void *pvParameters) {
    int taskId = (int)pvParameters;
    uint32_t operationCount = 0;
    
    // Feed watchdog
    esp_task_wdt_add(NULL);
    
    while (stressTestRunning && operationCount < STRESS_OPERATIONS_PER_TASK) {
        // Randomly choose an operation
        int operation = random(0, 10);
        
        switch (operation) {
            case 0:
            case 1:
                // Most common: handleUpdates
                OTAManager::handleUpdates();
                updateMetrics(true);
                break;
                
            case 2:
                // Check initialization state
                bool isInit = OTAManager::isInitialized();
                updateMetrics(true);
                break;
                
            case 3:
                // Try to initialize (should be idempotent)
                OTAManager::initialize(
                    "stress-test",
                    "password123",
                    3232,
                    []() { return true; }
                );
                updateMetrics(true);
                break;
                
            case 4:
                // Set start callback
                OTAManager::setStartCallback([]() {
                    // Empty callback
                });
                updateMetrics(OTAManager::isInitialized());
                break;
                
            case 5:
                // Set end callback
                OTAManager::setEndCallback([]() {
                    // Empty callback
                });
                updateMetrics(OTAManager::isInitialized());
                break;
                
            case 6:
                // Set progress callback
                OTAManager::setProgressCallback([](unsigned int p, unsigned int t) {
                    // Empty callback
                });
                updateMetrics(OTAManager::isInitialized());
                break;
                
            case 7:
                // Set error callback
                OTAManager::setErrorCallback([](ota_error_t e) {
                    // Empty callback
                });
                updateMetrics(OTAManager::isInitialized());
                break;
                
            case 8:
                // Invalid initialization attempt
                OTAManager::initialize(nullptr, "pass", 3232, nullptr);
                updateMetrics(true); // Should handle gracefully
                break;
                
            case 9:
                // Invalid port initialization
                OTAManager::initialize("test", "pass", 0, nullptr);
                updateMetrics(true); // Should handle gracefully
                break;
        }
        
        operationCount++;
        
        // Feed watchdog
        esp_task_wdt_reset();
        
        // Minimal delay to allow other tasks
        if (operationCount % 100 == 0) {
            vTaskDelay(1);
        }
    }
    
    esp_task_wdt_delete(NULL);
    vTaskDelete(NULL);
}

/**
 * @brief Monitor task - reports stress test progress
 */
void monitorTask(void *pvParameters) {
    uint32_t lastReport = millis();
    uint32_t reportInterval = 5000; // Report every 5 seconds
    
    while (stressTestRunning) {
        if (millis() - lastReport >= reportInterval) {
            xSemaphoreTake(metricsMutex, portMAX_DELAY);
            
            Serial.printf("\n=== Stress Test Progress ===\n");
            Serial.printf("Total Operations: %u\n", totalOperations);
            Serial.printf("Successful: %u\n", successfulOperations);
            Serial.printf("Failed: %u\n", failedOperations);
            Serial.printf("Operations/sec: %.1f\n", 
                         (float)totalOperations / ((millis() - 2000) / 1000.0));
            Serial.printf("Free Heap: %u bytes (min: %u)\n", 
                         ESP.getFreeHeap(), minFreeHeap);
            Serial.printf("Largest Free Block: %u bytes\n", 
                         ESP.getMaxAllocHeap());
            
            xSemaphoreGive(metricsMutex);
            
            lastReport = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    vTaskDelete(NULL);
}

// Unity test functions

void test_concurrent_stress() {
    TEST_MESSAGE("\n=== Starting Concurrent Stress Test ===");
    TEST_MESSAGE("This test will run for 30 seconds...\n");
    
    // Initialize metrics
    totalOperations = 0;
    successfulOperations = 0;
    failedOperations = 0;
    stressTestRunning = true;
    initialFreeHeap = ESP.getFreeHeap();
    minFreeHeap = initialFreeHeap;
    
    metricsMutex = xSemaphoreCreateMutex();
    TEST_ASSERT_NOT_NULL(metricsMutex);
    
    TaskHandle_t taskHandles[STRESS_TEST_TASKS];
    TaskHandle_t monitorHandle;
    
    // Create monitor task
    xTaskCreate(
        monitorTask,
        "Monitor",
        4096,
        NULL,
        2,
        &monitorHandle
    );
    
    // Create stress test tasks
    for (int i = 0; i < STRESS_TEST_TASKS; i++) {
        char taskName[16];
        snprintf(taskName, sizeof(taskName), "Stress_%d", i);
        
        BaseType_t result = xTaskCreate(
            stressTestTask,
            taskName,
            3072,
            (void*)i,
            1,
            &taskHandles[i]
        );
        
        TEST_ASSERT_EQUAL(pdPASS, result);
    }
    
    // Let the stress test run
    vTaskDelay(pdMS_TO_TICKS(STRESS_TEST_DURATION_MS));
    
    // Stop the test
    stressTestRunning = false;
    vTaskDelay(pdMS_TO_TICKS(1000)); // Allow tasks to finish
    
    // Final report
    Serial.printf("\n=== Stress Test Results ===\n");
    Serial.printf("Total Operations: %u\n", totalOperations);
    Serial.printf("Successful: %u (%.1f%%)\n", 
                 successfulOperations, 
                 (float)successfulOperations * 100 / totalOperations);
    Serial.printf("Failed: %u (%.1f%%)\n", 
                 failedOperations,
                 (float)failedOperations * 100 / totalOperations);
    Serial.printf("Operations/sec: %.1f\n", 
                 (float)totalOperations / (STRESS_TEST_DURATION_MS / 1000.0));
    Serial.printf("Initial Free Heap: %u bytes\n", initialFreeHeap);
    Serial.printf("Final Free Heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %u bytes\n", minFreeHeap);
    Serial.printf("Memory Leaked: %d bytes\n", 
                 (int)initialFreeHeap - (int)ESP.getFreeHeap());
    
    // Verify results
    TEST_ASSERT_GREATER_THAN(0, totalOperations);
    TEST_ASSERT_GREATER_THAN(0, successfulOperations);
    
    // Check for memory leaks (allow small variance)
    int memoryLeak = (int)initialFreeHeap - (int)ESP.getFreeHeap();
    TEST_ASSERT_LESS_THAN(1024, abs(memoryLeak)); // Less than 1KB leak
    
    // Ensure minimum heap never got critically low
    TEST_ASSERT_GREATER_THAN(10240, minFreeHeap); // At least 10KB free
    
    vSemaphoreDelete(metricsMutex);
    TEST_MESSAGE("✓ Concurrent stress test passed");
}

void test_rapid_init_deinit_cycles() {
    TEST_MESSAGE("\n=== Testing Rapid Init/Deinit Cycles ===");
    
    uint32_t startHeap = ESP.getFreeHeap();
    const int cycles = 1000;
    
    for (int i = 0; i < cycles; i++) {
        // Initialize with different parameters each time
        char hostname[32];
        snprintf(hostname, sizeof(hostname), "device-%d", i);
        
        OTAManager::initialize(
            hostname,
            i % 2 ? "password" : nullptr,  // Alternate password
            3232 + (i % 10),               // Vary port
            i % 3 ? nullptr : []() { return true; }  // Vary callback
        );
        
        // Perform some operations
        OTAManager::handleUpdates();
        bool isInit = OTAManager::isInitialized();
        TEST_ASSERT_TRUE(isInit);
        
        // Set random callbacks
        if (i % 4 == 0) {
            OTAManager::setStartCallback([]() {});
        }
        if (i % 4 == 1) {
            OTAManager::setEndCallback([]() {});
        }
        if (i % 4 == 2) {
            OTAManager::setProgressCallback([](unsigned int p, unsigned int t) {});
        }
        if (i % 4 == 3) {
            OTAManager::setErrorCallback([](ota_error_t e) {});
        }
        
        // Progress report
        if (i % 100 == 0) {
            Serial.printf("Cycle %d/%d, Heap: %u\n", i, cycles, ESP.getFreeHeap());
        }
    }
    
    uint32_t endHeap = ESP.getFreeHeap();
    int heapDiff = (int)startHeap - (int)endHeap;
    
    Serial.printf("Heap difference after %d cycles: %d bytes\n", cycles, heapDiff);
    
    // Allow small heap difference due to fragmentation
    TEST_ASSERT_LESS_THAN(2048, abs(heapDiff));
    TEST_MESSAGE("✓ Rapid init/deinit cycles test passed");
}

// Main test runner
void runStressTests() {
    UNITY_BEGIN();
    
    RUN_TEST(test_concurrent_stress);
    RUN_TEST(test_rapid_init_deinit_cycles);
    
    UNITY_END();
}

// For PlatformIO native testing
#ifdef UNIT_TEST
void setup() {
    delay(2000); // Wait for serial
    
    Serial.begin(115200);
    Serial.println("\n=== OTAManager Stress Tests ===\n");
    
    // Increase watchdog timeout for stress tests
    esp_task_wdt_init(30, true);
    
    runStressTests();
}

void loop() {
    // Nothing to do
}
#endif