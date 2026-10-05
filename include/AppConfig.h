/**
 * @file AppConfig.h
 * @brief Configuration data structures for ESP32 GLA WiFi Auto-Login
 *
 * Contains all configuration structures, enums, and constants used throughout
 * the system. Adapted from Tenda N301 Billing System patterns.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>
#include <map>
#include <vector>

#if __has_include("config.h")
#include "config.h"
#endif

#if __has_include("portal_ca.h")
#include "portal_ca.h"
#endif

// ============================================================================
// System State Enums
// ============================================================================

enum SystemState {
    STATE_BOOT,
    STATE_LOAD_CONFIG,
    STATE_WIFI_CONNECT,
    STATE_WIFI_CONNECTED,
    STATE_PORTAL_DETECT,
    STATE_PORTAL_AUTH,
    STATE_AUTHENTICATED,
    STATE_MONITORING,
    STATE_ERROR,
    STATE_RESTART,
    // Router mode states
    STATE_ROUTER_INIT,
    STATE_AP_SETUP,
    STATE_NAT_INIT,
    STATE_DNS_INIT,
    STATE_ROUTER_ACTIVE
};

// ============================================================================
// MAC Configuration Structure
// ============================================================================

struct MACConfig {
    bool customMACEnabled;      // Use custom MAC address
    uint8_t customMAC[6];       // Custom MAC address
    bool macRandomization;      // Enable MAC randomization
    bool macCloning;            // Clone another device's MAC
    uint8_t clonedMAC[6];       // MAC address to clone

    MACConfig() : customMACEnabled(false), macRandomization(false), macCloning(false) {
        memset(customMAC, 0, 6);
        memset(clonedMAC, 0, 6);
    }
};

// ============================================================================
// Main Configuration Structure
// ============================================================================

struct Config {
    // WiFi credentials
    String wifiSSID;
    String wifiPassword;

    // Portal credentials
    String portalUsername;
    String portalPassword;

    // Portal configuration
    String portalURL;
    String portalCACertificate;
    String portalUsernameField;
    String portalPasswordField;

    // Timing configuration
    uint32_t checkIntervalMs;
    uint32_t httpTimeoutMs;

    // Retry configuration
    uint8_t maxAuthRetries;     // Default: 5
    uint8_t maxWiFiRetries;     // Default: 20

    // Feature flags
    bool verboseLogging;        // Default: false
    bool ledEnabled;            // Default: false
    uint8_t ledPin;             // Default: 2 (built-in LED)

    // Router mode configuration
    bool routerEnabled;         // Enable router mode
    String apSSID;              // AP SSID
    String apPassword;          // AP password (WPA2-PSK)
    IPAddress apIP;             // AP IP address
    IPAddress apGateway;        // AP gateway
    IPAddress apSubnet;         // AP subnet mask
    uint8_t apChannel;          // WiFi channel (1-13)
    uint8_t maxClients;         // Max simultaneous clients (1-4)
    bool ssidHidden;            // Hide SSID broadcast

    // NAT configuration
    bool natEnabled;            // Enable NAT routing
    int natTimeout;             // NAT entry timeout (seconds)
    int maxNATConnections;      // Max concurrent NAT entries

    // DNS configuration
    bool dnsEnabled;            // Enable DNS server
    IPAddress primaryDNS;       // Primary upstream DNS
    IPAddress secondaryDNS;     // Secondary upstream DNS
    int dnsCacheSize;           // Max DNS cache entries
    int dnsCacheTTL;            // Default cache TTL (seconds)

    // MAC configuration
    MACConfig macConfig;

    // Performance limits
    int maxThroughputKbps;      // Throughput limit (0 = unlimited)
    bool rateLimitingEnabled;   // Enable per-client rate limiting

    // Security
    int maxAuthAttempts;        // Max failed auth attempts
    int authAttemptWindow;      // Time window for attempts (seconds)
    bool stealthMode;           // Hide AP from casual scans

    Config() {
        // Initialize with defaults
#ifdef WIFI_SSID
        wifiSSID = WIFI_SSID;
#else
        wifiSSID = "";
#endif
#ifdef WIFI_PASSWORD
        wifiPassword = WIFI_PASSWORD;
#else
        wifiPassword = "";
#endif
#ifdef PORTAL_USER
        portalUsername = PORTAL_USER;
#else
        portalUsername = "";
#endif
#ifdef PORTAL_PASS
        portalPassword = PORTAL_PASS;
#else
        portalPassword = "";
#endif
#ifdef PORTAL_URL
        portalURL = PORTAL_URL;
#else
        portalURL = "https://cirm.onlinegla.com";
#endif
#ifdef PORTAL_CA_CERTIFICATE
        portalCACertificate = PORTAL_CA_CERTIFICATE;
#else
        portalCACertificate = "";
#endif
#ifdef PORTAL_USERNAME_FIELD
        portalUsernameField = PORTAL_USERNAME_FIELD;
#else
        portalUsernameField = "";
#endif
#ifdef PORTAL_PASSWORD_FIELD
        portalPasswordField = PORTAL_PASSWORD_FIELD;
#else
        portalPasswordField = "";
#endif

        checkIntervalMs = 60000;
        httpTimeoutMs = 15000;

        maxAuthRetries = 5;
        maxWiFiRetries = 20;

        verboseLogging = false;
        ledEnabled = false;
        ledPin = 2;

        // Router defaults
        routerEnabled = false;
        apSSID = "ESP32-Router";
        apPassword = "";
        apIP = IPAddress(192, 168, 4, 1);
        apGateway = IPAddress(192, 168, 4, 1);
        apSubnet = IPAddress(255, 255, 255, 0);
        apChannel = 1;
        maxClients = 4;
        ssidHidden = false;

        natEnabled = true;
        natTimeout = 300;
        maxNATConnections = 100;

        dnsEnabled = true;
        primaryDNS = IPAddress(8, 8, 8, 8);
        secondaryDNS = IPAddress(1, 1, 1, 1);
        dnsCacheSize = 50;
        dnsCacheTTL = 300;

        maxThroughputKbps = 0;
        rateLimitingEnabled = false;

        maxAuthAttempts = 3;
        authAttemptWindow = 300;
        stealthMode = false;
    }
};

// ============================================================================
// Login Form Data Structure
// ============================================================================

struct LoginFormData {
    String action;              // Form action URL
    String method;              // POST or GET
    String usernameField;       // Name of username input field
    String passwordField;       // Name of password input field
    std::map<String, String> hiddenFields;  // Hidden form fields

    LoginFormData() : method("POST") {}
};

// ============================================================================
// System Status Structure
// ============================================================================

struct SystemStatus {
    SystemState currentState;
    unsigned long stateEntryTime;
    String lastError;
    int consecutiveFailures;

    // WiFi status
    bool wifiConnected;
    String ipAddress;
    String gateway;
    int signalStrength;

    // Authentication status
    bool authenticated;
    unsigned long authTimestamp;
    String sessionCookie;

    // Statistics
    unsigned long uptime;
    int authAttempts;
    int authSuccesses;
    int authFailures;
    int connectivityChecks;
    int connectivityFailures;

    SystemStatus() {
        currentState = STATE_BOOT;
        stateEntryTime = 0;
        consecutiveFailures = 0;
        wifiConnected = false;
        signalStrength = 0;
        authenticated = false;
        authTimestamp = 0;
        uptime = 0;
        authAttempts = 0;
        authSuccesses = 0;
        authFailures = 0;
        connectivityChecks = 0;
        connectivityFailures = 0;
    }
};

// ============================================================================
// Authentication Session Structure
// ============================================================================

struct AuthSession {
    bool active;                // Session is active
    String cookies;             // All session cookies
    unsigned long createdAt;    // Session creation time
    unsigned long lastUsed;     // Last successful use
    unsigned long expiresAt;    // Expected expiration time
    int requestCount;           // Requests made with this session
    String cookie;              // Primary session cookie
    unsigned long timestamp;    // Session creation time (compatibility)
    bool isValid;               // Session validity flag

    AuthSession() {
        active = false;
        createdAt = 0;
        lastUsed = 0;
        expiresAt = 0;
        requestCount = 0;
        timestamp = 0;
        isValid = false;
    }
};

// ============================================================================
// Router Status Structure
// ============================================================================

struct RouterStatus {
    bool apActive;
    bool natActive;
    bool dnsActive;

    // AP status
    String apSSID;
    IPAddress apIP;
    int connectedClients;
    int maxClients;

    // NAT status
    int activeNATConnections;
    uint32_t packetsForwarded;
    uint32_t bytesForwarded;

    // DNS status
    uint32_t dnsQueriesHandled;
    uint32_t dnsCacheHits;
    int dnsCacheSize;

    // Performance metrics
    unsigned long routerUptime;
    float throughputMbps;
    int avgLatencyMs;

    // Memory usage
    uint32_t heapUsed;
    uint32_t heapFree;
    float heapUsagePercent;

    RouterStatus() {
        apActive = false;
        natActive = false;
        dnsActive = false;
        connectedClients = 0;
        maxClients = 4;
        activeNATConnections = 0;
        packetsForwarded = 0;
        bytesForwarded = 0;
        dnsQueriesHandled = 0;
        dnsCacheHits = 0;
        dnsCacheSize = 0;
        routerUptime = 0;
        throughputMbps = 0.0;
        avgLatencyMs = 0;
        heapUsed = 0;
        heapFree = 0;
        heapUsagePercent = 0.0;
    }
};

// ============================================================================
// Client Info Structure
// ============================================================================

struct ClientInfo {
    uint8_t mac[6];             // Client MAC address
    IPAddress ip;               // Assigned IP address
    unsigned long connectedAt;  // Connection timestamp
    uint32_t bytesReceived;     // Bytes received from client
    uint32_t bytesSent;         // Bytes sent to client
    int signalStrength;         // RSSI
    bool active;                // Currently connected

    ClientInfo() {
        memset(mac, 0, 6);
        connectedAt = 0;
        bytesReceived = 0;
        bytesSent = 0;
        signalStrength = 0;
        active = false;
    }
};

// ============================================================================
// NAT Entry Structure
// ============================================================================

struct NATEntry {
    IPAddress clientIP;
    uint16_t clientPort;
    IPAddress externalIP;
    uint16_t externalPort;
    unsigned long lastActivity;
    uint32_t packetsForwarded;

    NATEntry() {
        clientPort = 0;
        externalPort = 0;
        lastActivity = 0;
        packetsForwarded = 0;
    }
};

// ============================================================================
// NAT Statistics Structure
// ============================================================================

struct NATStats {
    uint32_t packetsForwarded;
    uint32_t packetsDropped;
    uint32_t bytesForwarded;
    uint32_t activeConnections;
    unsigned long lastUpdate;

    NATStats() {
        packetsForwarded = 0;
        packetsDropped = 0;
        bytesForwarded = 0;
        activeConnections = 0;
        lastUpdate = 0;
    }
};

// ============================================================================
// DNS Cache Entry Structure
// ============================================================================

struct DNSCacheEntry {
    IPAddress resolvedIP;
    unsigned long timestamp;
    uint32_t ttl;

    DNSCacheEntry() {
        timestamp = 0;
        ttl = 300;
    }
};

// ============================================================================
// DNS Statistics Structure
// ============================================================================

struct DNSStats {
    uint32_t queriesReceived;
    uint32_t queriesForwarded;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    uint32_t errors;
    unsigned long lastUpdate;

    DNSStats() {
        queriesReceived = 0;
        queriesForwarded = 0;
        cacheHits = 0;
        cacheMisses = 0;
        errors = 0;
        lastUpdate = 0;
    }
};

// ============================================================================
// Constants
// ============================================================================

// Detection URLs
#define DETECTION_URL_PRIMARY "http://clients3.google.com/generate_204"
#define DETECTION_URL_FALLBACK1 "http://detectportal.firefox.com/"
#define DETECTION_URL_FALLBACK2 "http://captive.apple.com/hotspot-detect.html"
#define DETECTION_URL_FALLBACK3 "http://connectivitycheck.gstatic.com/generate_204"

// NVS Namespace
#define NVS_NAMESPACE "portal_auth"

// Version
#define FIRMWARE_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

#endif // CONFIG_H
