#!/bin/bash

# ESP32 GLA WiFi Auto-Login - Test Script

echo "========================================="
echo "  ESP32 GLA WiFi Auto-Login Tests"
echo "========================================="
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    exit 1
fi

# Run tests
echo "Running unit tests..."
pio test

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "  All tests passed!"
    echo "========================================="
else
    echo ""
    echo "========================================="
    echo "  Tests failed!"
    echo "========================================="
    exit 1
fi
