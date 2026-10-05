#include "ConnectivityMonitor.h"
#include "Logger.h"

ConnectivityMonitor::ConnectivityMonitor(PortalDetector* detector) 
    : detector(detector), checkIntervalMs(60000), lastCheckTime(0), 
      lastSuccessTime(0), consecutiveFailures(0) {}

void ConnectivityMonitor::begin(unsigned long checkIntervalMs) {
    this->checkIntervalMs = checkIntervalMs;
    lastCheckTime = millis();
    lastSuccessTime = millis();
    consecutiveFailures = 0;
    
    logger.logf(INFO, "ConnMonitor", "Initialized with %lu ms interval", checkIntervalMs);
}

bool ConnectivityMonitor::checkConnectivity() {
    bool connected = detector->testConnectivity();
    
    if (connected) {
        consecutiveFailures = 0;
        lastSuccessTime = millis();
        logger.info("ConnMonitor", "Connectivity check: OK");
    } else {
        consecutiveFailures++;
        logger.logf(WARNING, "ConnMonitor", "Connectivity check: FAILED (count: %d)", 
                   consecutiveFailures);
        
        if (consecutiveFailures >= MAX_FAILURES) {
            logger.critical("ConnMonitor", "Max failures reached, restart required");
        }
    }
    
    lastCheckTime = millis();
    return connected;
}

void ConnectivityMonitor::update() {
    unsigned long now = millis();
    
    if (now - lastCheckTime >= checkIntervalMs) {
        checkConnectivity();
    }
}

unsigned long ConnectivityMonitor::getTimeSinceLastCheck() {
    return millis() - lastCheckTime;
}

int ConnectivityMonitor::getFailureCount() {
    return consecutiveFailures;
}

void ConnectivityMonitor::resetFailures() {
    consecutiveFailures = 0;
    logger.info("ConnMonitor", "Failure count reset");
}
