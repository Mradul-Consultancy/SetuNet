#include "WiFiManager.h"
#include "Logger.h"

WiFiManager::WiFiManager() : retryCount(0) {}

bool WiFiManager::begin(const char* ssid, const char* password) {
    this->ssid = String(ssid);
    this->password = String(password);
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    
    logger.logf(INFO, "WiFiMgr", "Initialized with SSID: %s", ssid);
    return true;
}

bool WiFiManager::connect() {
    logger.info("WiFiMgr", "Connecting to WiFi...");
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < MAX_RETRIES) {
        int retryDelayMs = getRetryDelay(retryCount);
        logger.logf(INFO, "WiFiMgr", "Attempt %d/%d, waiting %dms...", 
                   retryCount + 1, MAX_RETRIES, retryDelayMs);
        
        delay(retryDelayMs);
        retryCount++;
        
        if (WiFi.status() == WL_CONNECTED) {
            break;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        logger.logf(INFO, "WiFiMgr", "Connected! IP: %s, Signal: %d dBm", 
                   WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    } else {
        logger.error("WiFiMgr", "Failed to connect after max retries");
        return false;
    }
}

bool WiFiManager::reconnect() {
    logger.info("WiFiMgr", "Attempting to reconnect...");
    WiFi.disconnect();
    delay(1000);
    return connect();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiManager::getIP() {
    return WiFi.localIP();
}

IPAddress WiFiManager::getGateway() {
    return WiFi.gatewayIP();
}

int WiFiManager::getSignalStrength() {
    return WiFi.RSSI();
}

String WiFiManager::getSSID() {
    return WiFi.SSID();
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    logger.info("WiFiMgr", "Disconnected from WiFi");
}

int WiFiManager::getRetryDelay(int attempt) {
    if (attempt < 4) {
        return RETRY_DELAYS[attempt];
    }
    return RETRY_DELAYS[3];
}
