#!/bin/bash

# ESP32 GLA WiFi Auto-Login - Build Script

echo "========================================="
echo "  ESP32 GLA WiFi Auto-Login Build"
echo "========================================="
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Error: PlatformIO is not installed"
    echo "Please install PlatformIO: https://platformio.org/install"
    exit 1
fi

# Check if config.h exists
if [ ! -f "include/config.h" ]; then
    echo "Warning: include/config.h not found"
    echo "Copying from template..."
    cp include/config.h.template include/config.h
    echo "Please edit include/config.h with your credentials"
    exit 1
fi

# Clean build
echo "Cleaning previous build..."
pio run --target clean

# Build project
echo ""
echo "Building project..."
pio run

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "  Build successful!"
    echo "========================================="
    echo ""
    echo "To upload to ESP32, run:"
    echo "  ./scripts/flash.sh"
    echo ""
    echo "Or manually:"
    echo "  pio run --target upload"
else
    echo ""
    echo "========================================="
    echo "  Build failed!"
    echo "========================================="
    exit 1
fi
