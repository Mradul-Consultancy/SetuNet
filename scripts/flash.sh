#!/bin/bash

# ESP32 GLA WiFi Auto-Login - Flash Script

echo "========================================="
echo "  ESP32 GLA WiFi Auto-Login Flash"
echo "========================================="
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    exit 1
fi

# List available ports
echo "Available serial ports:"
pio device list
echo ""

# Upload firmware
echo "Uploading firmware to ESP32..."
pio run --target upload

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "  Upload successful!"
    echo "========================================="
    echo ""
    echo "To monitor serial output, run:"
    echo "  ./scripts/monitor.sh"
    echo ""
    echo "Or manually:"
    echo "  pio device monitor"
else
    echo ""
    echo "========================================="
    echo "  Upload failed!"
    echo "========================================="
    echo ""
    echo "Troubleshooting:"
    echo "  1. Check ESP32 is connected via USB"
    echo "  2. Check correct port in platformio.ini"
    echo "  3. Try pressing BOOT button during upload"
    exit 1
fi
