# ESP32 GLA WiFi Auto-Login - API Reference

> Draft reference. It may describe APIs or behaviors that are planned but not currently integrated. The current firmware scope and verified limitations are in `../README.md`; this project has not passed hardware or live-portal validation.

## Table of Contents

1. [Logger](#logger)
2. [CredentialStore](#credentialstore)
3. [WiFiManager](#wifimanager)
4. [HTTPClientLayer](#httpclientlayer)
5. [PortalDetector](#portaldetector)
6. [PortalAuthEngine](#portalauthengine)
7. [SessionManager](#sessionmanager)
8. [ConnectivityMonitor](#connectivitymonitor)
9. [APManager](#apmanager)
10. [NATRouter](#natrouter)
11. [DNSForwarder](#dnsforwarder)
12. [MACManager](#macmanager)

---

## Logger

Provides logging functionality with multiple log levels and circular buffer storage.

### Methods

#### `void begin(LogLevel minLevel = INFO)`
Initialize the logger with minimum log level.

**Parameters**:
- `minLevel`: Minimum log level to display (DEBUG, INFO, WARNING, ERROR, CRITICAL)

**Example**:
```cpp
logger.begin(INFO);
```

#### `void log(LogLevel level, const char* component, const char* message)`
Log a message with specified level.

**Parameters**:
- `level`: Log level
- `component`: Component name (e.g., "WiFiMgr", "PortalAuth")
- `message`: Log message

**Example**:
```cpp
logger.log(INFO, "Main", "System starting");
```

#### `void logf(LogLevel level, const char* component, const char* format, ...)`
Log a formatted message.

**Parameters**:
- `level`: Log level
- `component`: Component name
- `format`: Printf-style format string
- `...`: Format arguments

**Example**:
```cpp
logger.logf(INFO, "WiFiMgr", "Connected: IP=%s", ip.c_str());
```

#### Convenience Methods

```cpp
void debug(const char* component, const char* message);
void info(const char* component, const char* message);
void warning(const char* component, const char* message);
void error(const char* component, const char* message);
void critical(const char* component, const char* message);
```

---

## CredentialStore

Manages credential storage in ESP32 NVS (Non-Volatile Storage).

### Methods

#### `bool begin()`
Initialize NVS storage.

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
if (!credStore.begin()) {
    Serial.println("Failed to initialize NVS");
}
```

#### `bool saveCredentials(const Config& config)`
Save configuration to NVS.

**Parameters**:
- `config`: Configuration structure to save

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
Config config;
config.wifiSSID = "GLA";
config.wifiPassword = "REDACTED_CREDENTIAL";
credStore.saveCredentials(config);
```

#### `bool loadCredentials(Config& config)`
Load configuration from NVS.

**Parameters**:
- `config`: Configuration structure to populate

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
Config config;
if (credStore.loadCredentials(config)) {
    Serial.println("Credentials loaded");
}
```

#### `bool hasCredentials()`
Check if credentials exist in NVS.

**Returns**: `true` if credentials exist, `false` otherwise

#### `bool clearCredentials()`
Clear all credentials from NVS.

**Returns**: `true` if successful, `false` otherwise

#### `bool validateCredentials(const Config& config)`
Validate configuration before saving.

**Parameters**:
- `config`: Configuration to validate

**Returns**: `true` if valid, `false` otherwise

#### `Config getDefaultConfig()`
Get default configuration for GLA network.

**Returns**: Default configuration structure

---

## WiFiManager

Manages WiFi connection with retry logic and exponential backoff.

### Methods

#### `bool begin(const char* ssid, const char* password)`
Initialize WiFi manager with credentials.

**Parameters**:
- `ssid`: WiFi network SSID
- `password`: WiFi network password

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
wifiMgr.begin("GLA", "REDACTED_CREDENTIAL");
```

#### `bool connect()`
Connect to WiFi network with retry logic.

**Returns**: `true` if connected, `false` after max retries

**Example**:
```cpp
if (wifiMgr.connect()) {
    Serial.println("WiFi connected");
}
```

#### `bool reconnect()`
Disconnect and reconnect to WiFi.

**Returns**: `true` if reconnected, `false` otherwise

#### `bool isConnected()`
Check if currently connected to WiFi.

**Returns**: `true` if connected, `false` otherwise

#### `IPAddress getIP()`
Get assigned IP address.

**Returns**: Current IP address

#### `IPAddress getGateway()`
Get gateway IP address.

**Returns**: Gateway IP address

#### `int getSignalStrength()`
Get WiFi signal strength in dBm.

**Returns**: Signal strength (typically -30 to -90 dBm)

---

## HTTPClientLayer

Handles HTTP/HTTPS requests with cookie management and SSL support.

### Methods

#### `void begin()`
Initialize HTTP client.

**Example**:
```cpp
httpClient.begin();
```

#### `void setTimeout(int timeoutMs)`
Set HTTP request timeout.

**Parameters**:
- `timeoutMs`: Timeout in milliseconds

**Example**:
```cpp
httpClient.setTimeout(15000);  // 15 seconds
```

#### `int httpGET(const String& url, String& response, bool followRedirects = true)`
Perform HTTP GET request.

**Parameters**:
- `url`: URL to request
- `response`: String to store response body
- `followRedirects`: Whether to follow redirects

**Returns**: HTTP status code (200, 302, etc.) or -1 on error

**Example**:
```cpp
String response;
int code = httpClient.httpGET("http://example.com", response);
if (code == 200) {
    Serial.println(response);
}
```

#### `int httpPOST(const String& url, const String& postData, String& response)`
Perform HTTP POST request.

**Parameters**:
- `url`: URL to post to
- `postData`: POST data (URL-encoded)
- `response`: String to store response body

**Returns**: HTTP status code or -1 on error

**Example**:
```cpp
String postData = "username=user&password=pass";
String response;
int code = httpClient.httpPOST("http://example.com/login", postData, response);
```

#### `void setCookie(const String& cookie)`
Set cookie for subsequent requests.

**Parameters**:
- `cookie`: Cookie string

#### `void addHeader(const String& name, const String& value)`
Add custom header to requests.

**Parameters**:
- `name`: Header name
- `value`: Header value

#### `String getResponseHeader(const String& name)`
Get response header value.

**Parameters**:
- `name`: Header name

**Returns**: Header value or empty string

---

## PortalDetector

Detects captive portals using multiple detection methods.

### Methods

#### `String discoverPortalURL(const String& fallbackURL)`
Discover portal URL using detection methods.

**Parameters**:
- `fallbackURL`: URL to use if detection fails

**Returns**: Discovered portal URL or fallback URL

**Example**:
```cpp
String portalURL = portalDetector.discoverPortalURL("https://cirm.onlinegla.com");
```

#### `bool testConnectivity()`
Test if internet is accessible (no portal).

**Returns**: `true` if internet accessible, `false` if portal detected

**Example**:
```cpp
if (portalDetector.testConnectivity()) {
    Serial.println("Internet accessible");
} else {
    Serial.println("Captive portal detected");
}
```

---

## PortalAuthEngine

Performs captive portal authentication.

### Methods

#### `bool begin(const String& username, const String& password, const String& portalURL)`
Initialize authentication engine.

**Parameters**:
- `username`: Portal username
- `password`: Portal password
- `portalURL`: Portal URL

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
authEngine.begin("user123", "pass456", "https://portal.com");
```

#### `bool authenticate()`
Perform full authentication flow.

**Returns**: `true` if authenticated, `false` otherwise

**Example**:
```cpp
if (authEngine.authenticate()) {
    Serial.println("Authentication successful");
}
```

#### `bool isAuthenticated()`
Check if currently authenticated.

**Returns**: `true` if authenticated, `false` otherwise

#### `void clearSession()`
Clear authentication session.

**Example**:
```cpp
authEngine.clearSession();
```

#### `String getSessionCookie()`
Get session cookie from last authentication.

**Returns**: Session cookie string

---

## SessionManager

Manages authentication session state.

### Methods

#### `void storeSession(const String& cookie)`
Store session cookie.

**Parameters**:
- `cookie`: Session cookie to store

**Example**:
```cpp
sessionMgr.storeSession(authEngine.getSessionCookie());
```

#### `String getSessionCookie()`
Get stored session cookie.

**Returns**: Session cookie string

#### `bool isSessionValid()`
Check if session is valid and not expired.

**Returns**: `true` if valid, `false` otherwise

#### `unsigned long getSessionAge()`
Get session age in milliseconds.

**Returns**: Session age

#### `bool isSessionExpired()`
Check if session has expired (15 minutes).

**Returns**: `true` if expired, `false` otherwise

#### `void clearSession()`
Clear session data.

---

## ConnectivityMonitor

Monitors internet connectivity with 60-second intervals.

### Methods

#### `void begin(unsigned long checkIntervalMs)`
Initialize connectivity monitor.

**Parameters**:
- `checkIntervalMs`: Check interval in milliseconds

**Example**:
```cpp
connMonitor.begin(60000);  // 60 seconds
```

#### `bool checkConnectivity()`
Perform single connectivity check.

**Returns**: `true` if connected, `false` otherwise

#### `void update()`
Non-blocking update method (call in loop).

**Example**:
```cpp
void loop() {
    connMonitor.update();
}
```

#### `int getFailureCount()`
Get consecutive failure count.

**Returns**: Number of consecutive failures

#### `void resetFailures()`
Reset failure counter.

---

## APManager

Manages WiFi Access Point in router mode.

### Methods

#### `bool begin(const char* ssid, const char* password, uint8_t channel = 1)`
Initialize AP manager.

**Parameters**:
- `ssid`: AP SSID
- `password`: AP password (8-64 characters)
- `channel`: WiFi channel (1-13)

**Returns**: `true` if successful, `false` otherwise

**Example**:
```cpp
apMgr.begin("ESP32-Router", "password123", 1);
```

#### `bool startAP()`
Start Access Point.

**Returns**: `true` if started, `false` otherwise

#### `bool stopAP()`
Stop Access Point.

**Returns**: `true` if stopped, `false` otherwise

#### `bool isAPActive()`
Check if AP is active.

**Returns**: `true` if active, `false` otherwise

#### `int getClientCount()`
Get number of connected clients.

**Returns**: Client count

#### `void setMaxConnections(uint8_t max)`
Set maximum client connections (1-4).

**Parameters**:
- `max`: Maximum clients

---

## NATRouter

Handles Network Address Translation for router mode.

### Methods

#### `bool begin()`
Initialize NAT router.

**Returns**: `true` if successful, `false` otherwise

#### `bool enableNAT()`
Enable NAT routing.

**Returns**: `true` if enabled, `false` otherwise

**Example**:
```cpp
if (natRouter.enableNAT()) {
    Serial.println("NAT enabled");
}
```

#### `bool disableNAT()`
Disable NAT routing.

**Returns**: `true` if disabled, `false` otherwise

#### `bool isNATActive()`
Check if NAT is active.

**Returns**: `true` if active, `false` otherwise

#### `NATStats getStats()`
Get NAT statistics.

**Returns**: NATStats structure

#### `void update()`
Update NAT (call in loop for cleanup).

---

## DNSForwarder

Forwards DNS queries with caching.

### Methods

#### `bool begin()`
Initialize DNS forwarder.

**Returns**: `true` if successful, `false` otherwise

#### `bool startDNSServer()`
Start DNS server on port 53.

**Returns**: `true` if started, `false` otherwise

**Example**:
```cpp
if (dnsForwarder.startDNSServer()) {
    Serial.println("DNS server started");
}
```

#### `bool stopDNSServer()`
Stop DNS server.

**Returns**: `true` if stopped, `false` otherwise

#### `void setUpstreamDNS(IPAddress primary, IPAddress secondary)`
Set upstream DNS servers.

**Parameters**:
- `primary`: Primary DNS server
- `secondary`: Secondary DNS server

**Example**:
```cpp
dnsForwarder.setUpstreamDNS(IPAddress(8,8,8,8), IPAddress(1,1,1,1));
```

#### `DNSStats getStats()`
Get DNS statistics.

**Returns**: DNSStats structure

#### `void clearCache()`
Clear DNS cache.

#### `void update()`
Handle DNS queries (call in loop).

---

## MACManager

Manages MAC address configuration.

### Methods

#### `bool setCustomMAC(const char* macStr)`
Set custom MAC address.

**Parameters**:
- `macStr`: MAC address string (e.g., "00:11:22:33:44:55")

**Returns**: `true` if set, `false` otherwise

**Example**:
```cpp
macMgr.setCustomMAC("00:11:22:33:44:55");
```

#### `bool generateRandomMAC(uint8_t* mac)`
Generate and set random MAC address.

**Parameters**:
- `mac`: Buffer to store generated MAC

**Returns**: `true` if successful, `false` otherwise

#### `bool validateMAC(const char* macStr)`
Validate MAC address format.

**Parameters**:
- `macStr`: MAC address string

**Returns**: `true` if valid, `false` otherwise

#### `String getCurrentMAC()`
Get current MAC address.

**Returns**: MAC address string

#### `bool restoreFactoryMAC()`
Restore factory MAC address.

**Returns**: `true` if restored, `false` otherwise

---

## Data Structures

### Config

```cpp
struct Config {
    String wifiSSID;
    String wifiPassword;
    String portalUsername;
    String portalPassword;
    String portalURL;
    unsigned long checkIntervalMs;
    unsigned int httpTimeoutMs;
    uint8_t maxAuthRetries;
    uint8_t maxWiFiRetries;
    bool routerEnabled;
    String apSSID;
    String apPassword;
    uint8_t apChannel;
    uint8_t maxClients;
};
```

### SystemStatus

```cpp
struct SystemStatus {
    bool wifiConnected;
    bool authenticated;
    String ipAddress;
    int signalStrength;
    unsigned long uptime;
    unsigned long lastAuthTime;
    uint32_t authAttempts;
    uint32_t authSuccesses;
    uint32_t authFailures;
};
```

### NATStats

```cpp
struct NATStats {
    uint32_t packetsForwarded;
    uint32_t packetsDropped;
    uint32_t bytesForwarded;
    uint32_t activeConnections;
    unsigned long lastUpdate;
};
```

### DNSStats

```cpp
struct DNSStats {
    uint32_t queriesReceived;
    uint32_t queriesForwarded;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    uint32_t errors;
    unsigned long lastUpdate;
};
```
