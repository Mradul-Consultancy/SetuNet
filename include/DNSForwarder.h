#ifndef DNS_FORWARDER_H
#define DNS_FORWARDER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <map>
#include "AppConfig.h"

class DNSForwarder {
public:
    DNSForwarder();
    
    bool begin();
    bool startDNSServer();
    bool stopDNSServer();
    bool isDNSActive();
    
    void handleDNSQuery();
    void setUpstreamDNS(IPAddress primary, IPAddress secondary);
    
    DNSStats getStats();
    void clearCache();
    
    void update();
    
private:
    WiFiUDP udpServer;
    bool dnsActive;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;
    std::map<String, DNSCacheEntry> dnsCache;
    DNSStats stats;
    
    const int DNS_PORT = 53;
    const int MAX_CACHE_ENTRIES = 50;
    const int DNS_TIMEOUT = 1000;
    
    String parseDNSQuery(const uint8_t* buffer, int length);
    IPAddress forwardDNSQuery(const String& domain);
    void sendDNSResponse(const uint8_t* queryBuffer, int queryLength, IPAddress resolvedIP);
    void sendDNSError(const uint8_t* queryBuffer, int queryLength);
    IPAddress parseDNSResponse(const uint8_t* buffer, int length);
    void evictOldestCacheEntry();
};

#endif
