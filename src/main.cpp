/**
 * @file main.cpp
 * @brief ESP32 GLA WiFi Auto-Login - Main Application
 * 
 * Automatically authenticates to GLA college WiFi captive portal and maintains
 * persistent internet connectivity through continuous monitoring.
 */

#include <Arduino.h>
#include "AppConfig.h"
#include "Logger.h"
#include "CredentialStore.h"
#include "WiFiManager.h"
#include "HTTPClientLayer.h"
#include "PortalDetector.h"
#include "PortalAuthEngine.h"
#include "SessionManager.h"
#include "ConnectivityMonitor.h"
#include "APManager.h"
#include "Dashboard.h"
#include <esp_system.h>
#include <time.h>

Config config;
CredentialStore credStore;
WiFiManager wifiMgr;
HTTPClientLayer httpClient;
PortalDetector portalDetector(&httpClient);
PortalAuthEngine authEngine(&httpClient, &portalDetector);
SessionManager sessionMgr;
ConnectivityMonitor connMonitor(&portalDetector);
APManager setupAP;
Dashboard dashboard;

static void serviceDashboard() {
    dashboard.handleClient();
    if (dashboard.consumeRestartRequest()) {
        delay(1000);
        ESP.restart();
    }
}

static void synchronizeClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    const unsigned long timeoutMs = 10000;
    unsigned long startedAt = millis();

    while (millis() - startedAt < timeoutMs) {
        dashboard.handleClient();
        if (time(nullptr) >= 1700000000) {
            httpClient.setClockReady(true);
            logger.info("Main", "System clock synchronized for TLS verification");
            return;
        }
        delay(250);
    }

    httpClient.setClockReady(false);
    logger.warning("Main", "Clock sync unavailable; HTTPS requests will fail closed");
}

SystemState currentState = STATE_BOOT;
SystemStatus systemStatus;
bool credentialStoreReady = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32 GLA WiFi Auto-Login v1.0.0");
    Serial.println("========================================\n");
    
    logger.begin(INFO);
    logger.info("Main", "System starting...");

    credentialStoreReady = credStore.begin();
    if (!credentialStoreReady) {
        logger.error("Main", "Failed to initialize credential store");
        currentState = STATE_ERROR;
    }

    const String setupSSID = "SetuNet-Setup-" + String(static_cast<uint16_t>(esp_random()), HEX);
    const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
    char setupPassword[17];
    for (size_t i = 0; i < sizeof(setupPassword) - 1; i++) {
        setupPassword[i] = alphabet[esp_random() % (sizeof(alphabet) - 1)];
    }
    setupPassword[sizeof(setupPassword) - 1] = '\0';

    if (!setupAP.begin(setupSSID.c_str(), setupPassword) || !setupAP.startAP()) {
        logger.critical("Main", "Could not start the protected setup access point");
        currentState = STATE_ERROR;
        return;
    }

    Serial.printf("Dashboard Wi-Fi: %s\nDashboard password: %s\nDashboard URL: http://%s\n",
                  setupSSID.c_str(), setupPassword, setupAP.getAPIP().toString().c_str());
    memset(setupPassword, 0, sizeof(setupPassword));

    if (!dashboard.begin(config, credStore, setupAP, systemStatus)) {
        logger.critical("Main", "Could not start the local dashboard");
        currentState = STATE_ERROR;
        return;
    }

    wifiMgr.setServiceCallback(serviceDashboard);
    currentState = credentialStoreReady ? STATE_LOAD_CONFIG : STATE_ERROR;
}

void loop() {
    dashboard.handleClient();
    systemStatus.currentState = currentState;

    if (dashboard.consumeRestartRequest()) {
        delay(1000);
        ESP.restart();
    }

    if (dashboard.consumeRetryRequest()) {
        logger.info("Main", "Dashboard requested a connection retry");
        systemStatus.wifiConnected = false;
        systemStatus.authenticated = false;
        systemStatus.authFailures = 0;
        currentState = STATE_WIFI_CONNECT;
    }

    switch (currentState) {
        case STATE_BOOT:
            currentState = STATE_LOAD_CONFIG;
            break;
            
        case STATE_LOAD_CONFIG:
            logger.info("Main", "Loading configuration...");
            
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

            httpClient.setCACertificate(config.portalCACertificate);
            if (!credStore.validateCredentials(config)) {
                logger.warning("Main", "Network settings are missing or invalid; use the setup dashboard");
                currentState = STATE_SETUP;
                break;
            }

            logger.logf(INFO, "Main", "Config loaded: SSID=%s, Portal=%s",
                       config.wifiSSID.c_str(), config.portalURL.c_str());

            if (config.routerEnabled) {
                logger.warning("Main", "Router mode is disabled in this firmware build");
                config.routerEnabled = false;
            }
            
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
                synchronizeClock();
                
                currentState = STATE_PORTAL_DETECT;
            } else {
                logger.error("Main", "WiFi connection failed");
                systemStatus.wifiConnected = false;
                systemStatus.authenticated = false;
                systemStatus.ipAddress = "";
                systemStatus.signalStrength = 0;
                currentState = STATE_SETUP;
            }
            break;
            
        case STATE_PORTAL_DETECT:
            logger.info("Main", "Detecting captive portal...");
            
            httpClient.begin();
            httpClient.setTimeout(config.httpTimeoutMs);
            
            if (portalDetector.testConnectivity()) {
                logger.info("Main", "No portal detected, internet accessible");
                systemStatus.authenticated = true;
                systemStatus.wifiConnected = true;
                currentState = STATE_AUTHENTICATED;
            } else {
                logger.info("Main", "Captive portal detected");
                currentState = STATE_PORTAL_AUTH;
            }
            break;
            
        case STATE_PORTAL_AUTH:
            logger.info("Main", "Authenticating to portal...");
            
            authEngine.begin(config.portalUsername, config.portalPassword, config.portalURL,
                             config.portalUsernameField, config.portalPasswordField);
            
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
                
                for (int waitedMs = 0; waitedMs < 10000; waitedMs += 100) {
                    dashboard.handleClient();
                    delay(100);
                }
                
                if (systemStatus.authFailures >= config.maxAuthRetries) {
                    logger.critical("Main", "Max auth retries exceeded");
                    currentState = STATE_SETUP;
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
                systemStatus.authenticated = false;
                
                authEngine.clearSession();
                sessionMgr.clearSession();
                connMonitor.resetFailures();
                
                currentState = STATE_PORTAL_DETECT;
            }
            
            if (!wifiMgr.isConnected()) {
                logger.warning("Main", "WiFi disconnected, reconnecting...");
                systemStatus.wifiConnected = false;
                systemStatus.authenticated = false;
                systemStatus.ipAddress = "";
                systemStatus.signalStrength = 0;
                currentState = STATE_WIFI_CONNECT;
            }

            if (wifiMgr.isConnected()) {
                systemStatus.signalStrength = wifiMgr.getSignalStrength();
            }
            
            if (connMonitor.getFailureCount() >= 10) {
                logger.critical("Main", "Too many failures, restarting...");
                currentState = STATE_RESTART;
            }
            
            delay(1000);
            break;

        case STATE_SETUP:
            delay(10);
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
