#ifndef AP_MANAGER_H
#define AP_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "AppConfig.h"

class APManager {
public:
    APManager();
    
    bool begin(const char* ssid, const char* password, uint8_t channel = 1);
    bool startAP();
    bool stopAP();
    bool isAPActive();
    
    int getClientCount();
    std::vector<ClientInfo> getConnectedClients();
    
    bool configureAP(IPAddress ip, IPAddress gateway, IPAddress subnet);
    void setMaxConnections(uint8_t max);
    void setSSIDHidden(bool hidden);
    
    IPAddress getAPIP();
    String getAPMAC();
    
private:
    String apSSID;
    String apPassword;
    uint8_t apChannel;
    IPAddress apIP;
    IPAddress apGateway;
    IPAddress apSubnet;
    uint8_t maxClients;
    bool apActive;
    bool ssidHidden;
    
    void onClientConnected(const WiFiEvent_t event, const WiFiEventInfo_t info);
    void onClientDisconnected(const WiFiEvent_t event, const WiFiEventInfo_t info);
};

#endif
