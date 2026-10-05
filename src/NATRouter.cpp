#include "NATRouter.h"
#include "Logger.h"
#include <WiFi.h>
#include <lwip/lwip_napt.h>

NATRouter::NATRouter() : natActive(false), lastCleanup(0) {
    stats.packetsForwarded = 0;
    stats.packetsDropped = 0;
    stats.bytesForwarded = 0;
    stats.activeConnections = 0;
    stats.lastUpdate = 0;
}

bool NATRouter::begin() {
    logger.info("NATRouter", "Initialized");
    return true;
}

bool NATRouter::enableNAT() {
    logger.info("NATRouter", "Enabling NAT...");
    
    IPAddress apIP = WiFi.softAPIP();
    
    ip_napt_enable(apIP, 1);
    
    natActive = true;
    logger.logf(INFO, "NATRouter", "NAT enabled for AP IP: %s", apIP.toString().c_str());
    
    return true;
}

bool NATRouter::disableNAT() {
    if (!natActive) {
        return true;
    }
    
    ip_napt_enable(WiFi.softAPIP(), 0);
    
    natActive = false;
    logger.info("NATRouter", "NAT disabled");
    
    return true;
}

bool NATRouter::isNATActive() {
    return natActive;
}

NATStats NATRouter::getStats() {
    stats.lastUpdate = millis();
    stats.activeConnections = natTable.size();
    return stats;
}

void NATRouter::clearNATTable() {
    unsigned long now = millis();
    
    auto it = natTable.begin();
    while (it != natTable.end()) {
        if (now - it->second.lastActivity > NAT_TIMEOUT) {
            logger.logf(DEBUG, "NATRouter", "Removing stale entry: %s:%d",
                       it->second.clientIP.toString().c_str(), it->second.clientPort);
            it = natTable.erase(it);
        } else {
            ++it;
        }
    }
}

int NATRouter::getActiveConnections() {
    return natTable.size();
}

void NATRouter::update() {
    unsigned long now = millis();
    
    if (now - lastCleanup >= CLEANUP_INTERVAL) {
        performCleanup();
        lastCleanup = now;
    }
}

void NATRouter::performCleanup() {
    int beforeCount = natTable.size();
    clearNATTable();
    int afterCount = natTable.size();
    
    if (beforeCount != afterCount) {
        logger.logf(INFO, "NATRouter", "Cleanup: removed %d stale entries, %d active",
                   beforeCount - afterCount, afterCount);
    }
    
    if (natTable.size() > MAX_NAT_ENTRIES * 0.8) {
        logger.warning("NATRouter", "NAT table approaching capacity");
    }
}
