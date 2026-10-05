#ifndef MAC_MANAGER_H
#define MAC_MANAGER_H

#include <Arduino.h>
#include <esp_wifi.h>
#include "AppConfig.h"

class MACManager {
public:
    MACManager();
    
    bool setCustomMAC(const uint8_t* mac);
    bool setCustomMAC(const char* macStr);
    bool validateMAC(const uint8_t* mac);
    bool validateMAC(const char* macStr);
    
    bool generateRandomMAC(uint8_t* mac);
    bool cloneMAC(const uint8_t* sourceMac);
    
    String getCurrentMAC();
    bool restoreFactoryMAC();
    
private:
    uint8_t factoryMAC[6];
    bool factoryMACStored;
    
    void storeFactoryMAC();
    bool parseMACString(const char* macStr, uint8_t* mac);
    String macToString(const uint8_t* mac);
};

#endif
