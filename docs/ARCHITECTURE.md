# ESP32 GLA WiFi Auto-Login - Architecture Document

## Table of Contents

1. [System Overview](#system-overview)
2. [Component Architecture](#component-architecture)
3. [State Machine](#state-machine)
4. [Data Flow](#data-flow)
5. [Router Mode Architecture](#router-mode-architecture)
6. [Design Patterns](#design-patterns)
7. [Memory Management](#memory-management)
8. [Error Handling](#error-handling)

## System Overview

The ESP32 GLA WiFi Auto-Login System is a modular embedded application that automatically authenticates to captive portal WiFi networks. The architecture is inspired by the Tenda N301 Billing System, adapted for ESP32 constraints.

### Implementation and verification status

This is a design/architecture document, not evidence that every described behavior is implemented or validated. The current client-mode firmware compiles for `esp32dev`, but has not been uploaded to hardware or tested against the live GLA portal. `PortalAuthEngine` supports common HTML input forms; it does not execute JavaScript or implement portal-specific challenges. Session expiry is not directly checked. AP/NAT/DNS components exist as prototype code but are not integrated into the main state machine. Treat the performance and memory figures below as estimates, not measured results.

### Key Design Principles

1. **Modularity**: Each component has a single, well-defined responsibility
2. **Resilience**: Automatic recovery from failures with exponential backoff
3. **Efficiency**: Keep memory use suitable for ESP32 (actual usage must be measured on target hardware)
4. **Observability**: Comprehensive logging at all levels
5. **Adaptability**: Configurable for different captive portals

### System Layers

```
┌─────────────────────────────────────────┐
│         Application Layer               │
│  (State Machine, Main Loop)             │
└─────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────┐
│         Component Layer                 │
│  (WiFi, Auth, Monitor, Router)          │
└─────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────┐
│         Transport Layer                 │
│  (HTTP Client, WiFi Stack)              │
└─────────────────────────────────────────┘
           ↓
┌─────────────────────────────────────────┐
│         Hardware Layer                  │
│  (ESP32, WiFi Radio, NVS)               │
└─────────────────────────────────────────┘
```

## Component Architecture

### Core Components (Client Mode)

#### 1. Logger
**Purpose**: Centralized logging with multiple levels and circular buffer

**Responsibilities**:
- Log messages with timestamps
- Filter by log level
- Store last 100 messages in circular buffer
- Format output for serial console

**Dependencies**: None

**Memory**: ~10 KB (circular buffer)

#### 2. CredentialStore
**Purpose**: Persist configuration in ESP32 NVS

**Responsibilities**:
- Save/load credentials from NVS
- Validate configuration before saving
- Provide default configuration
- Clear stored credentials

**Dependencies**: Preferences library, Logger

**Memory**: ~2 KB

#### 3. WiFiManager
**Purpose**: Manage WiFi connection with retry logic

**Responsibilities**:
- Connect to WiFi network
- Implement exponential backoff (5s, 10s, 15s, 20s)
- Monitor connection status
- Report signal strength

**Dependencies**: WiFi library, Logger

**Memory**: ~5 KB

#### 4. HTTPClientLayer
**Purpose**: Handle HTTP/HTTPS requests with cookie management

**Responsibilities**:
- Perform GET/POST requests
- Manage cookies and headers
- Handle SSL/TLS (with certificate bypass)
- Limit response size (16 KB max)
- Retry on timeout (up to 3 times)

**Dependencies**: HTTPClient, WiFiClientSecure, Logger

**Memory**: ~20 KB (buffers)

#### 5. PortalDetector
**Purpose**: Detect captive portals using multiple methods

**Responsibilities**:
- Test standard detection URLs
- Check gateway IP
- Test DNS hijacking
- Verify internet connectivity

**Dependencies**: HTTPClientLayer, Logger

**Memory**: ~2 KB

#### 6. PortalAuthEngine
**Purpose**: Authenticate to captive portal

**Responsibilities**:
- Fetch login page
- Parse HTML forms
- Extract field names and hidden fields
- Build POST data with URL encoding
- Submit authentication request
- Detect success/failure

**Dependencies**: HTTPClientLayer, PortalDetector, Logger

**Memory**: ~15 KB (HTML parsing)

#### 7. SessionManager
**Purpose**: Manage authentication session state

**Responsibilities**:
- Store session cookies
- Track authentication timestamp
- Check session validity
- Detect session expiration (15 minutes)

**Dependencies**: Logger

**Memory**: ~1 KB

#### 8. ConnectivityMonitor
**Purpose**: Monitor internet connectivity with 60-second intervals

**Responsibilities**:
- Perform periodic connectivity tests
- Track consecutive failures
- Trigger re-authentication
- Non-blocking operation

**Dependencies**: PortalDetector, Logger

**Memory**: ~2 KB

### Router Mode Components

#### 9. APManager
**Purpose**: Manage WiFi Access Point

**Responsibilities**:
- Start/stop Access Point
- Configure AP settings (IP, subnet, channel)
- Monitor client connections
- Enforce client limits (max 4)

**Dependencies**: WiFi library, Logger

**Memory**: ~5 KB

#### 10. NATRouter
**Purpose**: Route packets between AP and STA interfaces

**Responsibilities**:
- Enable IP NAT using lwIP
- Maintain NAT translation table
- Clean up stale entries (5 minute timeout)
- Track statistics

**Dependencies**: lwIP NAT library, Logger

**Memory**: ~10 KB (NAT table)

#### 11. DNSForwarder
**Purpose**: Forward DNS queries with caching

**Responsibilities**:
- Run DNS server on port 53
- Parse DNS queries
- Forward to upstream DNS (8.8.8.8, 1.1.1.1)
- Cache responses (50 entries, LRU eviction)
- Respond to clients

**Dependencies**: WiFiUDP, Logger

**Memory**: ~8 KB (cache)

#### 12. MACManager
**Purpose**: Manage MAC address configuration

**Responsibilities**:
- Set custom MAC address
- Generate random MAC
- Clone MAC from another device
- Validate MAC format
- Restore factory MAC

**Dependencies**: esp_wifi library, Logger

**Memory**: ~1 KB

## State Machine

The system uses a finite state machine for main loop control:

### States

1. **BOOT**: Initial state, transition to LOAD_CONFIG
2. **LOAD_CONFIG**: Load credentials from NVS
3. **WIFI_CONNECT**: Connect to WiFi network
4. **PORTAL_DETECT**: Detect captive portal
5. **PORTAL_AUTH**: Authenticate to portal
6. **AUTHENTICATED**: Authentication successful
7. **MONITORING**: Monitor connectivity (60s loop)
8. **ERROR**: Error state, wait 60s then restart
9. **RESTART**: Restart ESP32

### State Transitions

```
BOOT → LOAD_CONFIG
LOAD_CONFIG → WIFI_CONNECT (credentials valid)
LOAD_CONFIG → ERROR (credentials invalid)

WIFI_CONNECT → PORTAL_DETECT (connected)
WIFI_CONNECT → ERROR (max retries)

PORTAL_DETECT → PORTAL_AUTH (portal detected)
PORTAL_DETECT → AUTHENTICATED (no portal)

PORTAL_AUTH → AUTHENTICATED (success)
PORTAL_AUTH → ERROR (max retries)

AUTHENTICATED → MONITORING

MONITORING → PORTAL_DETECT (connectivity lost)
MONITORING → WIFI_CONNECT (WiFi disconnected)
MONITORING → RESTART (max failures)

ERROR → RESTART (after 60s)
```

### Router Mode State Machine

Additional states for router mode:

1. **ROUTER_INIT**: Initialize router mode
2. **AP_SETUP**: Start Access Point
3. **NAT_INIT**: Enable NAT routing
4. **DNS_INIT**: Start DNS server
5. **ROUTER_ACTIVE**: Router mode active

## Data Flow

### Authentication Flow

```
1. WiFiManager connects to GLA network
   ↓
2. PortalDetector tests connectivity
   ↓ (portal detected)
3. PortalDetector discovers portal URL
   ↓
4. PortalAuthEngine fetches login page
   ↓
5. PortalAuthEngine parses HTML form
   ↓
6. PortalAuthEngine builds POST data
   ↓
7. PortalAuthEngine submits credentials
   ↓
8. HTTPClientLayer extracts Set-Cookie
   ↓
9. SessionManager stores session cookie
   ↓
10. PortalDetector verifies connectivity
    ↓ (success)
11. ConnectivityMonitor starts 60s loop
```

### Monitoring Flow

```
Every 60 seconds:
1. ConnectivityMonitor triggers check
   ↓
2. PortalDetector tests connectivity
   ↓
3a. Success: Reset failure counter
   ↓
3b. Failure: Increment failure counter
    ↓
    If failures > 0:
      - Clear session
      - Trigger re-authentication
    ↓
    If failures >= 10:
      - Restart ESP32
```

### Router Mode Packet Flow

```
Client → AP → NAT → STA → Internet
  ↑                           ↓
  └───────────────────────────┘
       (return path)

DNS Query Flow:
Client → AP → DNS Forwarder → Upstream DNS
  ↑                                ↓
  └────────────────────────────────┘
       (cached or forwarded)
```

## Router Mode Architecture

### Dual Interface Operation

The ESP32 operates in WIFI_AP_STA mode:

- **STA Interface**: Connects to GLA WiFi, authenticates to portal
- **AP Interface**: Hosts WiFi network for client devices

### NAT Implementation

Uses lwIP NAT (NAPT - Network Address and Port Translation):

```cpp
ip_napt_enable(WiFi.softAPIP(), 1);
```

**Translation**:
- Source IP: 192.168.4.x (AP subnet) → 172.16.92.x (STA IP)
- Source Port: Client port → Translated port
- Maintains translation table for return packets

**Limitations**:
- Max 100 concurrent connections
- 5 minute idle timeout
- Single radio shared between STA and AP (time-division)

### DNS Forwarding

Custom DNS server implementation:

1. Listen on UDP port 53
2. Parse DNS queries
3. Check cache (50 entries, 5 minute TTL)
4. Forward to upstream DNS if cache miss
5. Parse response and extract IP
6. Cache result and respond to client

**Performance**:
- Cache hit: < 1ms
- Cache miss: 50-200ms (upstream query)

## Design Patterns

### 1. Singleton Pattern
Used for global instances (Logger, CredentialStore, etc.)

```cpp
Logger logger;  // Global singleton
```

### 2. State Pattern
Main loop implements state machine

```cpp
switch (currentState) {
    case WIFI_CONNECT:
        // Handle WiFi connection
        break;
    case PORTAL_AUTH:
        // Handle authentication
        break;
}
```

### 3. Strategy Pattern
Multiple portal detection methods

```cpp
// Try multiple detection strategies
for (int i = 0; i < 3; i++) {
    if (testDetectionURL(DETECTION_URLS[i])) {
        return true;
    }
}
```

### 4. Observer Pattern
WiFi event callbacks

```cpp
WiFi.onEvent([](WiFiEvent_t event) {
    logger.info("WiFi", "Client connected");
}, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
```

### 5. Retry Pattern with Exponential Backoff
WiFi connection retries

```cpp
int delays[] = {5000, 10000, 15000, 20000};
for (int i = 0; i < MAX_RETRIES; i++) {
    delay(delays[min(i, 3)]);
    if (connect()) break;
}
```

## Memory Management

### Memory Budget

**Client Mode** (~80 KB total):
- Logger: 10 KB (circular buffer)
- HTTPClientLayer: 20 KB (request/response buffers)
- PortalAuthEngine: 15 KB (HTML parsing)
- WiFiManager: 5 KB
- Other components: 30 KB

**Router Mode** (~120 KB total):
- Client mode: 80 KB
- NATRouter: 10 KB (translation table)
- DNSForwarder: 8 KB (cache)
- APManager: 5 KB
- Other: 17 KB

### Memory Optimization Strategies

1. **Fixed-size buffers**: Avoid dynamic allocation
2. **Response size limiting**: Max 16 KB HTTP responses
3. **Circular buffers**: Fixed-size log storage
4. **LRU eviction**: DNS cache and NAT table
5. **String pooling**: Reuse String objects

### Memory Monitoring

```cpp
uint32_t freeHeap = ESP.getFreeHeap();
if (freeHeap < 20000) {
    logger.critical("Memory", "Low memory warning");
}
```

## Error Handling

### Error Categories

1. **Recoverable Errors**: Retry with backoff
   - WiFi connection failures
   - HTTP timeouts
   - Authentication failures

2. **Transient Errors**: Wait and retry
   - Network congestion
   - Portal server errors (5xx)

3. **Fatal Errors**: Restart ESP32
   - Memory allocation failures
   - NVS corruption
   - Max retries exceeded

### Error Recovery Strategies

#### WiFi Connection
- Exponential backoff: 5s, 10s, 15s, 20s
- Max 20 retries
- Restart after max retries

#### Authentication
- Rate limiting: Min 5s between attempts
- Max 5 retries
- Wait 60s after max retries

#### Connectivity Monitoring
- Track consecutive failures
- Re-authenticate after 1 failure
- Restart after 10 failures

### Logging Strategy

All errors logged with context:

```cpp
logger.logf(ERROR, "Component", "Error: %s (code: %d)", 
           errorMsg, errorCode);
```

Log levels guide recovery:
- **WARNING**: Retry automatically
- **ERROR**: Retry with backoff
- **CRITICAL**: Restart required

## Performance Characteristics

### Client Mode
- Boot time: 20-30 seconds
- Authentication time: 10-15 seconds
- Connectivity check: 2-5 seconds
- Memory usage: 70-80 KB
- Power consumption: ~150 mA average

### Router Mode
- Boot time: 30-40 seconds
- Throughput: 5-10 Mbps
- Latency: 20-50ms additional
- Max clients: 4 simultaneous
- Memory usage: 110-120 KB
- Power consumption: ~200 mA average

### Bottlenecks

1. **Single WiFi radio**: Shared between STA and AP (time-division)
2. **HTTP parsing**: HTML form parsing is CPU-intensive
3. **DNS forwarding**: Upstream query latency
4. **Memory**: Limited to ~200 KB free heap

## Security Considerations

### Threat Model

**Threats**:
- Credential theft from firmware or unencrypted NVS
- Man-in-the-middle attacks
- MAC address tracking
- Unauthorized AP access

**Mitigations**:
- No application-level NVS encryption is enabled; platform flash/NVS encryption must be configured separately
- HTTPS certificate validation is disabled by default for compatibility; this is vulnerable to man-in-the-middle attacks on untrusted Wi-Fi
- WPA2-PSK for AP
- MAC address randomization option
- Rate limiting on authentication

### Security Best Practices

1. Do not deploy with the default AP credentials; router mode is not currently integrated or supported.
2. Do not use the firmware with sensitive credentials on untrusted Wi-Fi while certificate validation is disabled.
3. Protect `include/config.h` and firmware binaries because credentials are compiled into the image.
4. Configure flash/NVS encryption in the ESP32 build/deployment environment if credential-at-rest protection is required.
5. Use MAC address management responsibly; network policies may prohibit address changes.

## Future Enhancements

1. **Multi-portal support**: Generic portal detection and authentication
2. **Web interface**: Configure via web UI instead of serial
3. **OTA updates**: Over-the-air firmware updates
4. **Metrics dashboard**: Real-time statistics display
5. **Power management**: Sleep modes for battery operation
6. **IPv6 support**: Dual-stack networking
7. **VPN support**: Encrypted tunnel for router mode
