#include "CredentialStore.h"
#include "Logger.h"

CredentialStore::CredentialStore() {}

bool CredentialStore::begin() {
    bool success = preferences.begin(NAMESPACE, false);
    if (success) {
        logger.info("CredStore", "Initialized NVS storage");
    } else {
        logger.error("CredStore", "Failed to initialize NVS");
    }
    return success;
}

bool CredentialStore::saveCredentials(const Config& config) {
    if (!validateCredentials(config)) {
        logger.error("CredStore", "Invalid credentials, cannot save");
        return false;
    }
    
    preferences.putString("wifi_ssid", config.wifiSSID);
    preferences.putString("wifi_pass", config.wifiPassword);
    preferences.putString("portal_user", config.portalUsername);
    preferences.putString("portal_pass", config.portalPassword);
    preferences.putString("portal_url", config.portalURL);
    preferences.putUInt("check_interval", config.checkIntervalMs);
    preferences.putUInt("http_timeout", config.httpTimeoutMs);
    preferences.putUChar("max_retries", config.maxAuthRetries);
    preferences.putUChar("max_wifi_retries", config.maxWiFiRetries);
    preferences.putBool("router_enabled", config.routerEnabled);
    
    if (config.routerEnabled) {
        preferences.putString("ap_ssid", config.apSSID);
        preferences.putString("ap_pass", config.apPassword);
        preferences.putUChar("ap_channel", config.apChannel);
        preferences.putUChar("max_clients", config.maxClients);
    }
    
    logger.info("CredStore", "Credentials saved to NVS");
    return true;
}

bool CredentialStore::loadCredentials(Config& config) {
    if (!hasCredentials()) {
        logger.warning("CredStore", "No credentials found in NVS");
        return false;
    }
    
    config.wifiSSID = preferences.getString("wifi_ssid", "");
    config.wifiPassword = preferences.getString("wifi_pass", "");
    config.portalUsername = preferences.getString("portal_user", "");
    config.portalPassword = preferences.getString("portal_pass", "");
    config.portalURL = preferences.getString("portal_url", "");
    config.checkIntervalMs = preferences.getUInt("check_interval", 60000);
    config.httpTimeoutMs = preferences.getUInt("http_timeout", 15000);
    config.maxAuthRetries = preferences.getUChar("max_retries", 5);
    config.maxWiFiRetries = preferences.getUChar("max_wifi_retries", 20);
    config.routerEnabled = preferences.getBool("router_enabled", false);

    if (config.routerEnabled) {
        logger.warning("CredStore", "Saved router-mode setting ignored; router mode is disabled");
        config.routerEnabled = false;
    }
    
    logger.info("CredStore", "Credentials loaded from NVS");
    return true;
}

bool CredentialStore::hasCredentials() {
    return preferences.isKey("wifi_ssid") && 
           preferences.isKey("portal_user");
}

bool CredentialStore::clearCredentials() {
    bool success = preferences.clear();
    if (success) {
        logger.info("CredStore", "Credentials cleared from NVS");
    } else {
        logger.error("CredStore", "Failed to clear credentials");
    }
    return success;
}

bool CredentialStore::validateCredentials(const Config& config) {
    if (config.routerEnabled) {
        logger.error("CredStore", "Router mode is not supported by this firmware");
        return false;
    }

    if (!validateSSID(config.wifiSSID.c_str())) {
        logger.error("CredStore", "Invalid WiFi SSID");
        return false;
    }
    
    if (!validateURL(config.portalURL.c_str())) {
        logger.error("CredStore", "Invalid portal URL");
        return false;
    }
    
    if (!validateNonEmpty(config.portalUsername.c_str())) {
        logger.error("CredStore", "Portal username cannot be empty");
        return false;
    }
    
    if (!validateNonEmpty(config.portalPassword.c_str())) {
        logger.error("CredStore", "Portal password cannot be empty");
        return false;
    }
    
    if (config.routerEnabled) {
        if (!validateSSID(config.apSSID.c_str())) {
            logger.error("CredStore", "Invalid AP SSID");
            return false;
        }
        
        if (config.apPassword.length() < 8 || config.apPassword.length() > 64) {
            logger.error("CredStore", "AP password must be 8-64 characters");
            return false;
        }
    }
    
    return true;
}

Config CredentialStore::getDefaultConfig() {
    return Config();
}

bool CredentialStore::validateSSID(const char* ssid) {
    if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(ssid); i++) {
        if (!isprint(ssid[i])) {
            return false;
        }
    }
    
    return true;
}

bool CredentialStore::validateURL(const char* url) {
    if (!url || strlen(url) == 0) {
        return false;
    }
    
    String urlStr(url);
    int schemeEnd = urlStr.indexOf("://");
    if (schemeEnd < 0 || !urlStr.startsWith("https://")) {
        return false;
    }

    int authorityStart = schemeEnd + 3;
    int authorityEnd = urlStr.indexOf('/', authorityStart);
    String authority = authorityEnd < 0
        ? urlStr.substring(authorityStart)
        : urlStr.substring(authorityStart, authorityEnd);
    if (authority.length() == 0 || authority.indexOf('@') >= 0) {
        return false;
    }
    for (size_t i = 0; i < authority.length(); i++) {
        if (isspace(authority.charAt(i))) {
            return false;
        }
    }
    return true;
}

bool CredentialStore::validateNonEmpty(const char* str) {
    return str && strlen(str) > 0;
}
