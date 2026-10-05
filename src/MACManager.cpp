#include "MACManager.h"
#include "Logger.h"

MACManager::MACManager() : factoryMACStored(false) {
    storeFactoryMAC();
}

bool MACManager::setCustomMAC(const uint8_t* mac) {
    if (!validateMAC(mac)) {
        logger.error("MACMgr", "Invalid MAC address");
        return false;
    }
    
    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, mac);
    
    if (err == ESP_OK) {
        logger.logf(INFO, "MACMgr", "MAC set to: %s", macToString(mac).c_str());
        return true;
    } else {
        logger.error("MACMgr", "Failed to set MAC address");
        return false;
    }
}

bool MACManager::setCustomMAC(const char* macStr) {
    uint8_t mac[6];
    
    if (!parseMACString(macStr, mac)) {
        logger.error("MACMgr", "Invalid MAC string format");
        return false;
    }
    
    return setCustomMAC(mac);
}

bool MACManager::validateMAC(const uint8_t* mac) {
    if ((mac[0] & 0x01) != 0) {
        logger.error("MACMgr", "Multicast MAC not allowed");
        return false;
    }
    
    bool allZero = true;
    bool allFF = true;
    
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0x00) allZero = false;
        if (mac[i] != 0xFF) allFF = false;
    }
    
    if (allZero || allFF) {
        logger.error("MACMgr", "Invalid MAC (all zeros or all FFs)");
        return false;
    }
    
    return true;
}

bool MACManager::validateMAC(const char* macStr) {
    uint8_t mac[6];
    
    if (!parseMACString(macStr, mac)) {
        return false;
    }
    
    return validateMAC(mac);
}

bool MACManager::generateRandomMAC(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = random(256);
    }
    
    mac[0] &= 0xFE;
    mac[0] |= 0x02;
    
    logger.logf(INFO, "MACMgr", "Generated random MAC: %s", macToString(mac).c_str());
    
    return setCustomMAC(mac);
}

bool MACManager::cloneMAC(const uint8_t* sourceMac) {
    if (!validateMAC(sourceMac)) {
        logger.error("MACMgr", "Invalid source MAC for cloning");
        return false;
    }
    
    logger.logf(INFO, "MACMgr", "Cloning MAC: %s", macToString(sourceMac).c_str());
    
    return setCustomMAC(sourceMac);
}

String MACManager::getCurrentMAC() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    return macToString(mac);
}

bool MACManager::restoreFactoryMAC() {
    if (!factoryMACStored) {
        logger.error("MACMgr", "Factory MAC not stored");
        return false;
    }
    
    logger.logf(INFO, "MACMgr", "Restoring factory MAC: %s", macToString(factoryMAC).c_str());
    
    return setCustomMAC(factoryMAC);
}

void MACManager::storeFactoryMAC() {
    esp_wifi_get_mac(WIFI_IF_STA, factoryMAC);
    factoryMACStored = true;
    
    logger.logf(INFO, "MACMgr", "Factory MAC stored: %s", macToString(factoryMAC).c_str());
}

bool MACManager::parseMACString(const char* macStr, uint8_t* mac) {
    int values[6];
    
    int count = sscanf(macStr, "%x:%x:%x:%x:%x:%x",
                      &values[0], &values[1], &values[2],
                      &values[3], &values[4], &values[5]);
    
    if (count != 6) {
        count = sscanf(macStr, "%x-%x-%x-%x-%x-%x",
                      &values[0], &values[1], &values[2],
                      &values[3], &values[4], &values[5]);
    }
    
    if (count != 6) {
        return false;
    }
    
    for (int i = 0; i < 6; i++) {
        if (values[i] < 0 || values[i] > 255) {
            return false;
        }
        mac[i] = (uint8_t)values[i];
    }
    
    return true;
}

String MACManager::macToString(const uint8_t* mac) {
    char buffer[18];
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buffer);
}
