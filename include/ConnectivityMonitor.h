#ifndef CONNECTIVITY_MONITOR_H
#define CONNECTIVITY_MONITOR_H

#include <Arduino.h>
#include "PortalDetector.h"

class ConnectivityMonitor {
public:
    ConnectivityMonitor(PortalDetector* detector);
    
    void begin(unsigned long checkIntervalMs);
    bool checkConnectivity();
    void update();
    
    unsigned long getTimeSinceLastCheck();
    int getFailureCount();
    void resetFailures();
    
private:
    PortalDetector* detector;
    
    unsigned long checkIntervalMs;
    unsigned long lastCheckTime;
    unsigned long lastSuccessTime;
    int consecutiveFailures;
    
    const int MAX_FAILURES = 10;
};

#endif
