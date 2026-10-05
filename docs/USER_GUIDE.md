# ESP32 GLA WiFi Auto-Login - User Guide

> Historical draft. Some sections below describe planned serial commands, router behavior, session handling, and security protections that are not implemented or verified. Use `../README.md` and `../GETTING_STARTED.md` for current instructions. The firmware has not been tested on hardware or against the live GLA portal.

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Configuration](#configuration)
4. [Operation Modes](#operation-modes)
5. [Monitoring](#monitoring)
6. [Troubleshooting](#troubleshooting)
7. [Advanced Features](#advanced-features)

## Introduction

The ESP32 GLA WiFi Auto-Login System automatically authenticates to captive portal WiFi networks and maintains persistent internet connectivity. It can also function as a portable WiFi router, sharing the authenticated connection with other devices.

### Key Features

- Automatic captive portal detection and authentication
- Persistent session management with automatic re-authentication
- 60-second connectivity monitoring
- Optional WiFi router mode with NAT and DNS forwarding
- MAC address management
- Comprehensive logging and error recovery

## Getting Started

### Hardware Requirements

- ESP32 Development Board (ESP32-WROOM-32 or compatible)
- USB cable for programming and power
- 5V power supply (500mA minimum)

### Software Requirements

- PlatformIO IDE or Arduino IDE
- USB drivers for ESP32 (CH340/CP2102)

### Initial Setup

1. **Install PlatformIO** (recommended):
   - Download and install VS Code
   - Install PlatformIO extension from VS Code marketplace

2. **Clone the project**:
   ```bash
   git clone <repository-url>
   cd esp32-gla-wifi-autologin
   ```

3. **Configure credentials**:
   - Copy `include/config.h.template` to `include/config.h`
   - Edit `include/config.h` with your credentials

4. **Build and upload**:
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

## Configuration

### Basic Configuration

Edit `include/config.h` or use serial commands:

```cpp
// WiFi Network Settings
config.wifiSSID = "GLA";              // Network SSID
config.wifiPassword = "REDACTED_CREDENTIAL";     // Network password

// Portal Credentials
config.portalUsername = "your_username";  // Your portal username
config.portalPassword = "your_password";  // Your portal password
config.portalURL = "https://cirm.onlinegla.com";  // Portal URL

// Timing Settings
config.checkIntervalMs = 60000;  // Connectivity check interval (60 seconds)
config.httpTimeoutMs = 15000;    // HTTP request timeout (15 seconds)

// Retry Settings
config.maxAuthRetries = 5;       // Max authentication attempts
config.maxWiFiRetries = 20;      // Max WiFi connection attempts
```

### Router Mode Configuration

To enable router mode:

```cpp
config.routerEnabled = true;
config.apSSID = "ESP32-Router";           // Access Point SSID
config.apPassword = "esp32router123";     // AP password (min 8 chars)
config.apChannel = 1;                     // WiFi channel (1-13)
config.maxClients = 4;                    // Max simultaneous clients
```

### Serial Configuration Commands

Connect via serial monitor (115200 baud) and use these commands:

```
show config              - Display current configuration
set wifi <ssid> <pass>   - Update WiFi credentials
set portal <user> <pass> - Update portal credentials
set url <url>            - Update portal URL
test auth                - Test authentication manually
restart                  - Restart ESP32
save                     - Save configuration to NVS
```

## Operation Modes

### Client Mode (Default)

In client mode, the ESP32 connects to the GLA WiFi network and authenticates to the captive portal.

**Operation Flow**:
1. Boot and load configuration
2. Connect to WiFi network
3. Detect captive portal
4. Authenticate with credentials
5. Monitor connectivity every 60 seconds
6. Re-authenticate when session expires

**Status Indicators**:
- Serial output shows current state
- LED blinks during connection (if configured)
- Solid LED when authenticated

### Router Mode

In router mode, the ESP32 creates a WiFi access point and routes traffic through the authenticated GLA connection.

**Operation Flow**:
1. Connect to GLA WiFi (STA mode)
2. Authenticate to captive portal
3. Start WiFi Access Point
4. Enable NAT routing
5. Start DNS server
6. Route client traffic through authenticated connection

**Connecting Devices**:
1. Search for WiFi network with configured SSID
2. Connect using configured password
3. Devices automatically get IP via DHCP
4. Internet traffic routed through ESP32

**Performance**:
- Throughput: 5-10 Mbps
- Latency: 20-50ms additional
- Max clients: 4 simultaneous connections

## Monitoring

### Serial Output

The system logs all activities to serial output at 115200 baud:

```
[1234] [INFO] [Main] System starting...
[2345] [INFO] [WiFiMgr] Connected! IP: 172.16.92.86
[3456] [INFO] [PortalAuth] Authentication successful!
[4567] [INFO] [ConnMonitor] Connectivity check: OK
```

### Log Levels

- **DEBUG**: Detailed debugging information (HTTP headers, form parsing)
- **INFO**: Normal operation messages (connection status, auth success)
- **WARNING**: Potential issues (connectivity lost, retry attempts)
- **ERROR**: Recoverable errors (auth failed, HTTP timeout)
- **CRITICAL**: System failures (max retries exceeded, restart required)

### System Statistics

View statistics via serial command:

```
show stats
```

Output includes:
- Uptime
- WiFi signal strength
- Authentication attempts/successes/failures
- Connectivity checks
- Memory usage
- Router mode statistics (if enabled)

## Troubleshooting

### WiFi Connection Issues

**Problem**: Cannot connect to WiFi network

**Solutions**:
1. Verify SSID and password are correct
2. Check WiFi signal strength (should be > -70 dBm)
3. Ensure ESP32 is within range
4. Try different WiFi channel
5. Check for MAC address filtering on network

**Serial Output**:
```
[ERROR] [WiFiMgr] Failed to connect after max retries
```

### Authentication Failures

**Problem**: Authentication to portal fails

**Solutions**:
1. Verify portal username and password
2. Check portal URL is correct
3. Test manual authentication via browser first
4. Check if portal has changed authentication method
5. Review serial output for specific error messages

**Serial Output**:
```
[ERROR] [PortalAuth] Authentication failed
[ERROR] [PortalAuth] Failed to fetch login page
```

### Connectivity Lost

**Problem**: Internet access lost after successful authentication

**Solutions**:
1. System automatically re-authenticates
2. Check for network outages
3. Verify portal session hasn't expired
4. Check WiFi signal strength
5. Review connectivity monitor logs

**Serial Output**:
```
[WARNING] [ConnMonitor] Connectivity check: FAILED (count: 3)
[WARNING] [Main] Connectivity lost, re-authenticating...
```

### Router Mode Issues

**Problem**: Devices cannot connect to ESP32 AP

**Solutions**:
1. Verify router mode is enabled in config
2. Check AP password is at least 8 characters
3. Ensure AP SSID is visible (not hidden)
4. Try different WiFi channel
5. Check max clients limit not exceeded

**Problem**: Slow internet speed in router mode

**Solutions**:
1. Expected throughput is 5-10 Mbps (hardware limitation)
2. Reduce number of connected clients
3. Check WiFi signal strength to GLA network
4. Avoid bandwidth-intensive applications
5. Consider using client mode for single device

### Memory Issues

**Problem**: System restarts due to low memory

**Solutions**:
1. Disable router mode if not needed
2. Reduce DNS cache size
3. Reduce NAT table size
4. Check for memory leaks in custom code
5. Monitor memory usage via serial output

## Advanced Features

### MAC Address Management

Change MAC address for privacy or network compatibility:

**Serial Commands**:
```
set mac <MAC>           - Set custom MAC address
set mac random          - Generate random MAC
set mac clone <MAC>     - Clone another device's MAC
set mac restore         - Restore factory MAC
show mac                - Display current MAC
```

**Example**:
```
set mac 00:11:22:33:44:55
```

**Warning**: MAC spoofing may violate network policies. Use responsibly.

### Stealth Mode

Hide AP SSID in router mode:

```cpp
config.ssidHidden = true;
```

Clients must manually enter SSID to connect.

### Custom Portal Detection

Add custom detection URLs:

Edit `PortalDetector.cpp` and add URLs to `DETECTION_URLS` array.

### Verbose Logging

Enable detailed HTTP logging:

```cpp
logger.setLogLevel(DEBUG);
```

Shows HTTP headers, form fields, and response bodies.

### Performance Tuning

Adjust timing parameters:

```cpp
config.checkIntervalMs = 30000;   // Check every 30 seconds (faster)
config.httpTimeoutMs = 10000;     // 10 second timeout (faster)
```

**Trade-offs**:
- Shorter intervals = faster detection, more battery usage
- Longer timeouts = more reliable, slower response

## Security Considerations

### Credential Storage

- Credentials stored in ESP32 NVS (Non-Volatile Storage)
- NVS is encrypted by ESP32 hardware
- Credentials not visible in serial output (masked)

### Network Security

- WPA2-PSK encryption for router mode AP
- SSL certificate validation disabled for portal compatibility
- Rate limiting on authentication attempts
- MAC address validation

### Best Practices

1. Use strong AP password (min 12 characters)
2. Change default AP SSID
3. Enable stealth mode if needed
4. Regularly update firmware
5. Monitor serial output for security events
6. Use MAC address management responsibly

## Support

For issues or questions:

1. Check serial output for error messages
2. Review troubleshooting section
3. Verify configuration is correct
4. Test manual authentication via browser
5. Check GitHub issues for similar problems

## Appendix

### Serial Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `show config` | Display configuration | `show config` |
| `show stats` | Display statistics | `show stats` |
| `show mac` | Display MAC address | `show mac` |
| `set wifi <ssid> <pass>` | Update WiFi credentials | `set wifi GLA REDACTED_CREDENTIAL` |
| `set portal <user> <pass>` | Update portal credentials | `set portal user123 pass456` |
| `set url <url>` | Update portal URL | `set url https://portal.com` |
| `set mac <mac>` | Set custom MAC | `set mac 00:11:22:33:44:55` |
| `test auth` | Test authentication | `test auth` |
| `restart` | Restart ESP32 | `restart` |
| `save` | Save configuration | `save` |

### LED Status Codes

| Pattern | Meaning |
|---------|---------|
| Blinking 1 Hz | Connecting to WiFi |
| Blinking 2 Hz | Authenticating to portal |
| Solid on | Authenticated and monitoring |
| Blinking 5 Hz | Critical error |
| Off | Idle or powered off |

### Error Codes

| Code | Description | Action |
|------|-------------|--------|
| E001 | WiFi connection failed | Check credentials |
| E002 | Portal authentication failed | Check portal credentials |
| E003 | HTTP timeout | Check network connectivity |
| E004 | Memory allocation failed | Restart ESP32 |
| E005 | NVS initialization failed | Format NVS partition |
