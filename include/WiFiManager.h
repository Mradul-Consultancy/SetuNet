#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "AppConfig.h"

class WiFiManager {
public:
    WiFiManager();
    
    bool begin(const char* ssid, const char* password);
    bool connect();
    bool reconnect();
    bool isConnected();
    
    IPAddress getIP();
    IPAddress getGateway();
    int getSignalStrength();
    String getSSID();
    
    void disconnect();
    
private:
    String ssid;
    String password;
    int retryCount;
    const int MAX_RETRIES = 20;
    const int RETRY_DELAYS[4] = {5000, 10000, 15000, 20000};
    
    int getRetryDelay(int attempt);
};

#endif
