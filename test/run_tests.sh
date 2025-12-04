#!/bin/bash
# Test runner script for OTAManager library

echo "==================================="
echo "OTAManager Library Test Suite"
echo "==================================="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}Error: PlatformIO is not installed${NC}"
    echo "Install with: pip install platformio"
    exit 1
fi

# Function to run a test environment
run_test() {
    local env=$1
    local desc=$2
    
    echo -e "\n${YELLOW}Running: $desc${NC}"
    echo "Environment: $env"
    echo "-----------------------------------"
    
    if pio test -e $env; then
        echo -e "${GREEN}✓ $desc passed${NC}"
        return 0
    else
        echo -e "${RED}✗ $desc failed${NC}"
        return 1
    fi
}

# Keep track of test results
FAILED=0

# Run thread safety tests
if ! run_test "esp32-thread-safety-tests" "Thread Safety Tests"; then
    ((FAILED++))
fi

# Run stress tests
if ! run_test "esp32-stress-tests" "Stress Tests"; then
    ((FAILED++))
fi

# Summary
echo -e "\n==================================="
echo "Test Summary"
echo "==================================="

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}$FAILED test(s) failed${NC}"
    exit 1
fi