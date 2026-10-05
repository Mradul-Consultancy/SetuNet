#include "APManager.h"
#include "Logger.h"
#include <esp_wifi.h>

APManager::APManager() 
    : apChannel(1), maxClients(4), apActive(false), ssidHidden(false) {
    apIP = IPAddress(192, 168, 4, 1);
    apGateway = IPAddress(192, 168, 4, 1);
    apSubnet = IPAddress(255, 255, 255, 0);
}

bool APManager::begin(const char* ssid, const char* password, uint8_t channel) {
    apSSID = String(ssid);
    apPassword = String(password);
    apChannel = channel;
    
    if (apPassword.length() < 8 || apPassword.length() > 64) {
        logger.error("APMgr", "AP password must be 8-64 characters");
        return false;
    }
    
    logger.logf(INFO, "APMgr", "Initialized with SSID: %s", ssid);
    return true;
}

bool APManager::startAP() {
    logger.info("APMgr", "Starting Access Point...");
    
    WiFi.mode(WIFI_AP_STA);
    
    if (!WiFi.softAPConfig(apIP, apGateway, apSubnet)) {
        logger.error("APMgr", "Failed to configure AP");
        return false;
    }
    
    if (!WiFi.softAP(apSSID.c_str(), apPassword.c_str(), apChannel, ssidHidden, maxClients)) {
        logger.error("APMgr", "Failed to start AP");
        return false;
    }
    
    apActive = true;
    
    logger.logf(INFO, "APMgr", "AP started: SSID=%s, IP=%s, Channel=%d", 
               apSSID.c_str(), WiFi.softAPIP().toString().c_str(), apChannel);
    
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        logger.logf(INFO, "APMgr", "Client connected: MAC=%02X:%02X:%02X:%02X:%02X:%02X",
                   info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
                   info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
                   info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
    }, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        logger.logf(INFO, "APMgr", "Client disconnected: MAC=%02X:%02X:%02X:%02X:%02X:%02X",
                   info.wifi_ap_stadisconnected.mac[0], info.wifi_ap_stadisconnected.mac[1],
                   info.wifi_ap_stadisconnected.mac[2], info.wifi_ap_stadisconnected.mac[3],
                   info.wifi_ap_stadisconnected.mac[4], info.wifi_ap_stadisconnected.mac[5]);
    }, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    
    return true;
}

bool APManager::stopAP() {
    if (!apActive) {
        return true;
    }
    
    WiFi.softAPdisconnect(true);
    apActive = false;
    
    logger.info("APMgr", "AP stopped");
    return true;
}

bool APManager::isAPActive() {
    return apActive;
}

int APManager::getClientCount() {
    return WiFi.softAPgetStationNum();
}

std::vector<ClientInfo> APManager::getConnectedClients() {
    std::vector<ClientInfo> clients;
    wifi_sta_list_t stationList;
    
    if (esp_wifi_ap_get_sta_list(&stationList) != ESP_OK) {
        logger.error("APMgr", "Failed to read connected station list");
        return clients;
    }
    
    for (int i = 0; i < stationList.num; i++) {
        ClientInfo client;
        memcpy(client.mac, stationList.sta[i].mac, sizeof(client.mac));
        client.connectedAt = millis();
        client.active = true;
        clients.push_back(client);
    }
    
    return clients;
}

bool APManager::configureAP(IPAddress ip, IPAddress gateway, IPAddress subnet) {
    apIP = ip;
    apGateway = gateway;
    apSubnet = subnet;
    
    logger.logf(INFO, "APMgr", "AP configured: IP=%s, Gateway=%s, Subnet=%s",
               ip.toString().c_str(), gateway.toString().c_str(), subnet.toString().c_str());
    
    return true;
}

void APManager::setMaxConnections(uint8_t max) {
    if (max > 4) {
        logger.warning("APMgr", "Max clients limited to 4 (ESP32 hardware limit)");
        maxClients = 4;
    } else {
        maxClients = max;
    }
}

void APManager::setSSIDHidden(bool hidden) {
    ssidHidden = hidden;
    logger.logf(INFO, "APMgr", "SSID hidden: %s", hidden ? "true" : "false");
}

IPAddress APManager::getAPIP() {
    return WiFi.softAPIP();
}

String APManager::getAPMAC() {
    return WiFi.softAPmacAddress();
}
