/**
 * @file main.cpp
 * @brief ESP32 GLA WiFi Auto-Login - Main Application
 * 
 * Automatically authenticates to GLA college WiFi captive portal and maintains
 * persistent internet connectivity through continuous monitoring.
 */

#include <Arduino.h>
#include "Config.h"
#include "Logger.h"
#include "CredentialStore.h"
#include "WiFiManager.h"
#include "HTTPClientLayer.h"
#include "PortalDetector.h"
#include "PortalAuthEngine.h"
#include "SessionManager.h"
#include "ConnectivityMonitor.h"

Config config;
CredentialStore credStore;
WiFiManager wifiMgr;
HTTPClientLayer httpClient;
PortalDetector portalDetector(&httpClient);
PortalAuthEngine authEngine(&httpClient, &portalDetector);
SessionManager sessionMgr;
ConnectivityMonitor connMonitor(&portalDetector);

SystemState currentState = STATE_BOOT;
SystemStatus systemStatus;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32 GLA WiFi Auto-Login v1.0.0");
    Serial.println("========================================\n");
    
    logger.begin(INFO);
    logger.info("Main", "System starting...");
    
    currentState = STATE_LOAD_CONFIG;
}

void loop() {
    switch (currentState) {
        case STATE_BOOT:
            currentState = STATE_LOAD_CONFIG;
            break;
            
        case STATE_LOAD_CONFIG:
            logger.info("Main", "Loading configuration...");
            
            if (!credStore.begin()) {
                logger.error("Main", "Failed to initialize credential store");
                currentState = STATE_ERROR;
                break;
            }
            
            if (credStore.hasCredentials()) {
                if (!credStore.loadCredentials(config)) {
                    logger.error("Main", "Failed to load credentials");
                    currentState = STATE_ERROR;
                    break;
                }
            } else {
                logger.info("Main", "No saved credentials; using include/config.h settings");
                config = credStore.getDefaultConfig();
            }

            if (!credStore.validateCredentials(config)) {
                logger.error("Main", "Invalid credentials; configure include/config.h or saved credentials");
                currentState = STATE_ERROR;
                break;
            }
            
            logger.logf(INFO, "Main", "Config loaded: SSID=%s, Portal=%s", 
                       config.wifiSSID.c_str(), config.portalURL.c_str());
            
            currentState = STATE_WIFI_CONNECT;
            break;
            
        case STATE_WIFI_CONNECT:
            logger.info("Main", "Connecting to WiFi...");
            
            wifiMgr.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
            
            if (wifiMgr.connect()) {
                logger.logf(INFO, "Main", "WiFi connected: IP=%s, Signal=%d dBm", 
                           wifiMgr.getIP().toString().c_str(), 
                           wifiMgr.getSignalStrength());
                
                systemStatus.wifiConnected = true;
                systemStatus.ipAddress = wifiMgr.getIP().toString();
                systemStatus.signalStrength = wifiMgr.getSignalStrength();
                
                currentState = STATE_PORTAL_DETECT;
            } else {
                logger.error("Main", "WiFi connection failed");
                systemStatus.wifiConnected = false;
                currentState = STATE_ERROR;
            }
            break;
            
        case STATE_PORTAL_DETECT:
            logger.info("Main", "Detecting captive portal...");
            
            httpClient.begin();
            httpClient.setTimeout(config.httpTimeoutMs);
            
            if (portalDetector.testConnectivity()) {
                logger.info("Main", "No portal detected, internet accessible");
                currentState = STATE_AUTHENTICATED;
            } else {
                logger.info("Main", "Captive portal detected");
                currentState = STATE_PORTAL_AUTH;
            }
            break;
            
        case STATE_PORTAL_AUTH:
            logger.info("Main", "Authenticating to portal...");
            
            authEngine.begin(config.portalUsername, config.portalPassword, config.portalURL);
            
            systemStatus.authAttempts++;
            
            if (authEngine.authenticate()) {
                logger.info("Main", "Authentication successful!");
                
                sessionMgr.storeSession(authEngine.getSessionCookie());
                systemStatus.authenticated = true;
                systemStatus.authSuccesses++;
                systemStatus.authTimestamp = millis();
                
                currentState = STATE_AUTHENTICATED;
            } else {
                logger.error("Main", "Authentication failed");
                systemStatus.authenticated = false;
                systemStatus.authFailures++;
                
                delay(10000);
                
                if (systemStatus.authFailures >= config.maxAuthRetries) {
                    logger.critical("Main", "Max auth retries exceeded");
                    currentState = STATE_ERROR;
                } else {
                    currentState = STATE_PORTAL_AUTH;
                }
            }
            break;
            
        case STATE_AUTHENTICATED:
            logger.info("Main", "Authenticated! Starting monitoring...");
            
            connMonitor.begin(config.checkIntervalMs);
            systemStatus.uptime = millis();
            
            currentState = STATE_MONITORING;
            break;
            
        case STATE_MONITORING:
            connMonitor.update();
            
            if (connMonitor.getFailureCount() > 0) {
                logger.warning("Main", "Connectivity lost, re-authenticating...");
                
                authEngine.clearSession();
                sessionMgr.clearSession();
                connMonitor.resetFailures();
                
                currentState = STATE_PORTAL_DETECT;
            }
            
            if (!wifiMgr.isConnected()) {
                logger.warning("Main", "WiFi disconnected, reconnecting...");
                systemStatus.wifiConnected = false;
                currentState = STATE_WIFI_CONNECT;
            }
            
            if (connMonitor.getFailureCount() >= 10) {
                logger.critical("Main", "Too many failures, restarting...");
                currentState = STATE_RESTART;
            }
            
            delay(1000);
            break;
            
        case STATE_ERROR:
            logger.error("Main", "System in error state, waiting 60s before restart...");
            delay(60000);
            currentState = STATE_RESTART;
            break;
            
        case STATE_RESTART:
            logger.info("Main", "Restarting ESP32...");
            delay(1000);
            ESP.restart();
            break;
    }
}
