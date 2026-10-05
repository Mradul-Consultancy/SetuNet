#include "DNSForwarder.h"
#include "Logger.h"

DNSForwarder::DNSForwarder() : dnsActive(false) {
    primaryDNS = IPAddress(8, 8, 8, 8);
    secondaryDNS = IPAddress(1, 1, 1, 1);
    
    stats.queriesReceived = 0;
    stats.queriesForwarded = 0;
    stats.cacheHits = 0;
    stats.cacheMisses = 0;
    stats.errors = 0;
    stats.lastUpdate = 0;
}

bool DNSForwarder::begin() {
    logger.info("DNSFwd", "Initialized");
    return true;
}

bool DNSForwarder::startDNSServer() {
    if (!udpServer.begin(DNS_PORT)) {
        logger.error("DNSFwd", "Failed to start DNS server");
        return false;
    }
    
    dnsActive = true;
    logger.logf(INFO, "DNSFwd", "DNS server started on port %d", DNS_PORT);
    logger.logf(INFO, "DNSFwd", "Upstream DNS: %s, %s", 
               primaryDNS.toString().c_str(), secondaryDNS.toString().c_str());
    
    return true;
}

bool DNSForwarder::stopDNSServer() {
    if (!dnsActive) {
        return true;
    }
    
    udpServer.stop();
    dnsActive = false;
    
    logger.info("DNSFwd", "DNS server stopped");
    return true;
}

bool DNSForwarder::isDNSActive() {
    return dnsActive;
}

void DNSForwarder::handleDNSQuery() {
    if (!dnsActive) {
        return;
    }
    
    int packetSize = udpServer.parsePacket();
    if (packetSize == 0) {
        return;
    }
    
    uint8_t buffer[512];
    int len = udpServer.read(buffer, sizeof(buffer));
    
    if (len <= 0) {
        return;
    }
    
    String domain = parseDNSQuery(buffer, len);
    
    if (domain.length() == 0) {
        stats.errors++;
        sendDNSError(buffer, len);
        return;
    }
    
    stats.queriesReceived++;
    
    logger.logf(DEBUG, "DNSFwd", "Query for: %s", domain.c_str());
    
    if (dnsCache.find(domain) != dnsCache.end()) {
        DNSCacheEntry& entry = dnsCache[domain];
        
        if (millis() - entry.timestamp < entry.ttl * 1000) {
            stats.cacheHits++;
            logger.logf(DEBUG, "DNSFwd", "Cache hit: %s -> %s", 
                       domain.c_str(), entry.resolvedIP.toString().c_str());
            sendDNSResponse(buffer, len, entry.resolvedIP);
            return;
        } else {
            dnsCache.erase(domain);
        }
    }
    
    stats.cacheMisses++;
    
    IPAddress resolvedIP = forwardDNSQuery(domain);
    
    if (resolvedIP != IPAddress(0, 0, 0, 0)) {
        DNSCacheEntry entry;
        entry.resolvedIP = resolvedIP;
        entry.timestamp = millis();
        entry.ttl = 300;
        
        if (dnsCache.size() >= MAX_CACHE_ENTRIES) {
            evictOldestCacheEntry();
        }
        
        dnsCache[domain] = entry;
        
        sendDNSResponse(buffer, len, resolvedIP);
        stats.queriesForwarded++;
        
        logger.logf(DEBUG, "DNSFwd", "Resolved: %s -> %s", 
                   domain.c_str(), resolvedIP.toString().c_str());
    } else {
        stats.errors++;
        sendDNSError(buffer, len);
        logger.logf(WARNING, "DNSFwd", "Failed to resolve: %s", domain.c_str());
    }
}

void DNSForwarder::setUpstreamDNS(IPAddress primary, IPAddress secondary) {
    primaryDNS = primary;
    secondaryDNS = secondary;
    
    logger.logf(INFO, "DNSFwd", "Upstream DNS updated: %s, %s",
               primary.toString().c_str(), secondary.toString().c_str());
}

DNSStats DNSForwarder::getStats() {
    stats.lastUpdate = millis();
    return stats;
}

void DNSForwarder::clearCache() {
    int count = dnsCache.size();
    dnsCache.clear();
    logger.logf(INFO, "DNSFwd", "Cache cleared: %d entries removed", count);
}

void DNSForwarder::update() {
    handleDNSQuery();
}

String DNSForwarder::parseDNSQuery(const uint8_t* buffer, int length) {
    if (length < 12) {
        return "";
    }
    
    String domain = "";
    int pos = 12;
    
    while (pos < length && buffer[pos] != 0) {
        int labelLen = buffer[pos];
        pos++;
        
        if (pos + labelLen > length) {
            return "";
        }
        
        for (int i = 0; i < labelLen; i++) {
            domain += (char)buffer[pos + i];
        }
        domain += ".";
        pos += labelLen;
    }
    
    if (domain.length() > 0 && domain.endsWith(".")) {
        domain.remove(domain.length() - 1);
    }
    
    return domain;
}

IPAddress DNSForwarder::forwardDNSQuery(const String& domain) {
    WiFiUDP udpClient;
    
    uint8_t queryBuffer[512];
    int queryLen = 0;
    
    queryBuffer[queryLen++] = random(256);
    queryBuffer[queryLen++] = random(256);
    queryBuffer[queryLen++] = 0x01;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x01;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    
    int start = 0;
    int end = domain.indexOf('.');
    
    while (end >= 0) {
        String label = domain.substring(start, end);
        queryBuffer[queryLen++] = label.length();
        for (int i = 0; i < label.length(); i++) {
            queryBuffer[queryLen++] = label[i];
        }
        start = end + 1;
        end = domain.indexOf('.', start);
    }
    
    String lastLabel = domain.substring(start);
    queryBuffer[queryLen++] = lastLabel.length();
    for (int i = 0; i < lastLabel.length(); i++) {
        queryBuffer[queryLen++] = lastLabel[i];
    }
    
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x01;
    queryBuffer[queryLen++] = 0x00;
    queryBuffer[queryLen++] = 0x01;
    
    udpClient.beginPacket(primaryDNS, DNS_PORT);
    udpClient.write(queryBuffer, queryLen);
    udpClient.endPacket();
    
    unsigned long start_time = millis();
    while (millis() - start_time < DNS_TIMEOUT) {
        int packetSize = udpClient.parsePacket();
        if (packetSize > 0) {
            uint8_t response[512];
            int len = udpClient.read(response, sizeof(response));
            
            IPAddress resolvedIP = parseDNSResponse(response, len);
            udpClient.stop();
            return resolvedIP;
        }
        delay(10);
    }
    
    udpClient.stop();
    return IPAddress(0, 0, 0, 0);
}

void DNSForwarder::sendDNSResponse(const uint8_t* queryBuffer, int queryLength, IPAddress resolvedIP) {
    uint8_t response[512];
    memcpy(response, queryBuffer, queryLength);
    
    response[2] = 0x81;
    response[3] = 0x80;
    response[6] = 0x00;
    response[7] = 0x01;
    
    int pos = queryLength;
    response[pos++] = 0xC0;
    response[pos++] = 0x0C;
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    response[pos++] = 0x2C;
    response[pos++] = 0x00;
    response[pos++] = 0x04;
    response[pos++] = resolvedIP[0];
    response[pos++] = resolvedIP[1];
    response[pos++] = resolvedIP[2];
    response[pos++] = resolvedIP[3];
    
    udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
    udpServer.write(response, pos);
    udpServer.endPacket();
}

void DNSForwarder::sendDNSError(const uint8_t* queryBuffer, int queryLength) {
    uint8_t response[512];
    memcpy(response, queryBuffer, min(queryLength, 512));
    
    response[2] = 0x81;
    response[3] = 0x83;
    
    udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
    udpServer.write(response, min(queryLength, 512));
    udpServer.endPacket();
}

IPAddress DNSForwarder::parseDNSResponse(const uint8_t* buffer, int length) {
    if (length < 12) {
        return IPAddress(0, 0, 0, 0);
    }
    
    int pos = 12;
    
    while (pos < length && buffer[pos] != 0) {
        if ((buffer[pos] & 0xC0) == 0xC0) {
            pos += 2;
            break;
        }
        pos += buffer[pos] + 1;
    }
    
    if (buffer[pos] == 0) {
        pos++;
    }
    
    pos += 4;
    
    while (pos + 12 <= length) {
        if ((buffer[pos] & 0xC0) == 0xC0) {
            pos += 2;
        } else {
            while (pos < length && buffer[pos] != 0) {
                pos += buffer[pos] + 1;
            }
            pos++;
        }
        
        uint16_t type = (buffer[pos] << 8) | buffer[pos + 1];
        pos += 8;
        uint16_t dataLen = (buffer[pos] << 8) | buffer[pos + 1];
        pos += 2;
        
        if (type == 1 && dataLen == 4) {
            return IPAddress(buffer[pos], buffer[pos + 1], buffer[pos + 2], buffer[pos + 3]);
        }
        
        pos += dataLen;
    }
    
    return IPAddress(0, 0, 0, 0);
}

void DNSForwarder::evictOldestCacheEntry() {
    if (dnsCache.empty()) {
        return;
    }
    
    auto oldest = dnsCache.begin();
    unsigned long oldestTime = oldest->second.timestamp;
    
    for (auto it = dnsCache.begin(); it != dnsCache.end(); ++it) {
        if (it->second.timestamp < oldestTime) {
            oldest = it;
            oldestTime = it->second.timestamp;
        }
    }
    
    logger.logf(DEBUG, "DNSFwd", "Evicting cache entry: %s", oldest->first.c_str());
    dnsCache.erase(oldest);
}
