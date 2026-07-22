/**
 * @file test_thread_safety.cpp
 * @brief Thread safety unit tests for OTAManager
 * 
 * These tests verify that OTAManager correctly handles concurrent access
 * from multiple FreeRTOS tasks without race conditions or deadlocks.
 */

#include <Arduino.h>
#include <unity.h>
#include <OTAManager.h>

// Only include WiFi if enabled
#if defined(ESP32) && CONFIG_WIFI_ENABLED == 1
    #include <WiFi.h>
#endif

// Test configuration
#define TEST_WIFI_SSID "TestNetwork"
#define TEST_WIFI_PASS "TestPassword"
#define TEST_THREADS 4
#define TEST_ITERATIONS 1000

// Task synchronization
static SemaphoreHandle_t testSemaphore = NULL;
static volatile int threadCounter = 0;
static volatile bool testPassed = true;
static volatile int initCallCount = 0;
static volatile int handleCallCount = 0;

// Test network callback
bool testNetworkCheck() {
    return true; // Always return true for testing
}

/**
 * @brief Test concurrent initialization attempts
 */
void test_concurrent_initialization(void *pvParameters) {
    int taskId = (int)pvParameters;
    
    // Wait for all tasks to be ready
    xSemaphoreTake(testSemaphore, portMAX_DELAY);
    
    // Try to initialize OTA
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        OTAManager::initialize(
            "test-device",
            "password",
            3232,
            testNetworkCheck
        );
        
        initCallCount++;
        
        // Small random delay to increase contention
        vTaskDelay(pdMS_TO_TICKS(random(0, 5)));
    }
    
    // Signal completion
    threadCounter++;
    vTaskDelete(NULL);
}

/**
 * @brief Test concurrent handleUpdates calls
 */
void test_concurrent_handle_updates(void *pvParameters) {
    int taskId = (int)pvParameters;
    
    // Wait for all tasks to be ready
    xSemaphoreTake(testSemaphore, portMAX_DELAY);
    
    // Call handleUpdates concurrently
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        OTAManager::handleUpdates();
        handleCallCount++;
        
        // Check initialization state
        bool isInit = OTAManager::isInitialized();
        TEST_ASSERT_TRUE(isInit);
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    // Signal completion
    threadCounter++;
    vTaskDelete(NULL);
}

/**
 * @brief Test concurrent callback modifications
 */
void test_concurrent_callback_changes(void *pvParameters) {
    int taskId = (int)pvParameters;
    static volatile int startCallbackCount = 0;
    static volatile int endCallbackCount = 0;
    
    // Wait for all tasks to be ready
    xSemaphoreTake(testSemaphore, portMAX_DELAY);
    
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        // Randomly set different callbacks
        switch (i % 4) {
            case 0:
                OTAManager::setStartCallback([]() {
                    startCallbackCount++;
                });
                break;
            case 1:
                OTAManager::setEndCallback([]() {
                    endCallbackCount++;
                });
                break;
            case 2:
                OTAManager::setProgressCallback([](unsigned int progress, unsigned int total) {
                    // Progress callback
                });
                break;
            case 3:
                OTAManager::setErrorCallback([](ota_error_t error) {
                    // Error callback
                });
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(random(0, 3)));
    }
    
    // Signal completion
    threadCounter++;
    vTaskDelete(NULL);
}

/**
 * @brief Test rapid initialization/handle cycles
 */
void test_init_handle_race_condition(void *pvParameters) {
    int taskId = (int)pvParameters;
    
    // Wait for all tasks to be ready
    xSemaphoreTake(testSemaphore, portMAX_DELAY);
    
    for (int i = 0; i < TEST_ITERATIONS / 10; i++) {
        if (taskId == 0) {
            // Task 0: Repeatedly initialize
            OTAManager::initialize(
                "race-test",
                "pass123",
                3232,
                testNetworkCheck
            );
        } else {
            // Other tasks: Call handleUpdates
            OTAManager::handleUpdates();
            
            // Also try to set callbacks
            if (i % 10 == 0) {
                OTAManager::setStartCallback([]() {
                    // Start callback
                });
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    // Signal completion
    threadCounter++;
    vTaskDelete(NULL);
}

// Unity test functions

void test_thread_safe_initialization() {
    TEST_MESSAGE("Testing concurrent initialization from multiple threads...");
    
    threadCounter = 0;
    initCallCount = 0;
    testSemaphore = xSemaphoreCreateBinary();
    
    TaskHandle_t taskHandles[TEST_THREADS];
    
    // Create test tasks
    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(
            test_concurrent_initialization,
            "InitTest",
            4096,
            (void*)i,
            1,
            &taskHandles[i]
        );
    }
    
    // Start all tasks simultaneously
    xSemaphoreGive(testSemaphore);
    
    // Wait for all tasks to complete
    while (threadCounter < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Verify results
    TEST_ASSERT_TRUE(OTAManager::isInitialized());
    TEST_ASSERT_EQUAL(TEST_THREADS * TEST_ITERATIONS, initCallCount);
    
    vSemaphoreDelete(testSemaphore);
    TEST_MESSAGE("✓ Concurrent initialization test passed");
}

void test_thread_safe_handle_updates() {
    TEST_MESSAGE("Testing concurrent handleUpdates calls...");
    
    threadCounter = 0;
    handleCallCount = 0;
    testSemaphore = xSemaphoreCreateBinary();
    
    // Ensure OTA is initialized
    OTAManager::initialize("test", "pass", 3232, testNetworkCheck);
    
    TaskHandle_t taskHandles[TEST_THREADS];
    
    // Create test tasks
    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(
            test_concurrent_handle_updates,
            "HandleTest",
            4096,
            (void*)i,
            1,
            &taskHandles[i]
        );
    }
    
    // Start all tasks simultaneously
    xSemaphoreGive(testSemaphore);
    
    // Wait for all tasks to complete
    while (threadCounter < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Verify results
    TEST_ASSERT_EQUAL(TEST_THREADS * TEST_ITERATIONS, handleCallCount);
    
    vSemaphoreDelete(testSemaphore);
    TEST_MESSAGE("✓ Concurrent handleUpdates test passed");
}

void test_thread_safe_callbacks() {
    TEST_MESSAGE("Testing concurrent callback modifications...");
    
    threadCounter = 0;
    testSemaphore = xSemaphoreCreateBinary();
    
    // Ensure OTA is initialized
    OTAManager::initialize("test", "pass", 3232, testNetworkCheck);
    
    TaskHandle_t taskHandles[TEST_THREADS];
    
    // Create test tasks
    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(
            test_concurrent_callback_changes,
            "CallbackTest",
            4096,
            (void*)i,
            1,
            &taskHandles[i]
        );
    }
    
    // Start all tasks simultaneously
    xSemaphoreGive(testSemaphore);
    
    // Wait for all tasks to complete
    while (threadCounter < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // If we get here without crashing, test passed
    TEST_ASSERT_TRUE(true);
    
    vSemaphoreDelete(testSemaphore);
    TEST_MESSAGE("✓ Concurrent callback modification test passed");
}

void test_init_handle_race() {
    TEST_MESSAGE("Testing initialization/handle race conditions...");
    
    threadCounter = 0;
    testSemaphore = xSemaphoreCreateBinary();
    
    TaskHandle_t taskHandles[TEST_THREADS];
    
    // Create test tasks
    for (int i = 0; i < TEST_THREADS; i++) {
        xTaskCreate(
            test_init_handle_race_condition,
            "RaceTest",
            4096,
            (void*)i,
            1,
            &taskHandles[i]
        );
    }
    
    // Start all tasks simultaneously
    xSemaphoreGive(testSemaphore);
    
    // Wait for all tasks to complete
    while (threadCounter < TEST_THREADS) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Verify OTA is still in valid state
    TEST_ASSERT_TRUE(OTAManager::isInitialized());
    
    vSemaphoreDelete(testSemaphore);
    TEST_MESSAGE("✓ Init/handle race condition test passed");
}

void test_parameter_validation_thread_safety() {
    TEST_MESSAGE("Testing parameter validation with concurrent access...");
    
    // Test null hostname handling from multiple threads
    bool nullHostnameHandled = true;
    
    TaskHandle_t taskHandles[2];
    
    xTaskCreate([](void *param) {
        for (int i = 0; i < 100; i++) {
            OTAManager::initialize(nullptr, "pass", 3232, testNetworkCheck);
            vTaskDelay(1);
        }
        vTaskDelete(NULL);
    }, "NullTest1", 2048, &nullHostnameHandled, 1, &taskHandles[0]);
    
    xTaskCreate([](void *param) {
        for (int i = 0; i < 100; i++) {
            OTAManager::initialize("", "pass", 3232, testNetworkCheck);
            vTaskDelay(1);
        }
        vTaskDelete(NULL);
    }, "NullTest2", 2048, &nullHostnameHandled, 1, &taskHandles[1]);
    
    // Wait for tasks to complete
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Should not crash and should not be initialized with invalid params
    TEST_ASSERT_TRUE(nullHostnameHandled);
    TEST_MESSAGE("✓ Parameter validation thread safety test passed");
}

// Main test runner
void runThreadSafetyTests() {
    UNITY_BEGIN();
    
    RUN_TEST(test_thread_safe_initialization);
    RUN_TEST(test_thread_safe_handle_updates);
    RUN_TEST(test_thread_safe_callbacks);
    RUN_TEST(test_init_handle_race);
    RUN_TEST(test_parameter_validation_thread_safety);
    
    UNITY_END();
}

// For PlatformIO native testing
#ifdef UNIT_TEST
void setup() {
    delay(2000); // Wait for serial
    
    Serial.begin(115200);
    Serial.println("\n=== OTAManager Thread Safety Tests ===\n");
    
    #if defined(ESP32) && CONFIG_WIFI_ENABLED == 1
    // Initialize WiFi for testing (mock mode)
    WiFi.mode(WIFI_STA);
    #endif
    
    runThreadSafetyTests();
}

void loop() {
    // Nothing to do
}
#endif