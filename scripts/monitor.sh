#!/bin/bash

# ESP32 GLA WiFi Auto-Login - Serial Monitor Script

echo "========================================="
echo "  ESP32 GLA WiFi Auto-Login Monitor"
echo "========================================="
echo ""
echo "Serial Monitor (115200 baud)"
echo "Press Ctrl+C to exit"
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    exit 1
fi

# Start serial monitor
pio device monitor --baud 115200 --filter esp32_exception_decoder
