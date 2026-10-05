#ifndef NAT_ROUTER_H
#define NAT_ROUTER_H

#include <Arduino.h>
#include <map>
#include "AppConfig.h"

class NATRouter {
public:
    NATRouter();
    
    bool begin();
    bool enableNAT();
    bool disableNAT();
    bool isNATActive();
    
    NATStats getStats();
    void clearNATTable();
    int getActiveConnections();
    
    void update();
    
private:
    bool natActive;
    std::map<uint32_t, NATEntry> natTable;
    NATStats stats;
    unsigned long lastCleanup;
    
    const int MAX_NAT_ENTRIES = 100;
    const unsigned long NAT_TIMEOUT = 300000;
    const unsigned long CLEANUP_INTERVAL = 60000;
    
    void performCleanup();
};

#endif
