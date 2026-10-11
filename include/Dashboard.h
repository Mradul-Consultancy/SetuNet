#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Arduino.h>
#include <WebServer.h>
#include "APManager.h"
#include "AppConfig.h"
#include "CredentialStore.h"

class Dashboard {
public:
    Dashboard();

    bool begin(Config& config, CredentialStore& credentialStore,
               APManager& apManager, SystemStatus& status);
    void handleClient();
    bool consumeRetryRequest();
    bool consumeRestartRequest();

private:
    WebServer server;
    Config* config;
    CredentialStore* credentialStore;
    APManager* apManager;
    SystemStatus* status;
    String csrfToken;
    bool retryRequested;
    bool restartRequested;

    void handleRoot();
    void handleStatus();
    void handleSave();
    void handleRetry();
    bool hasValidToken();
    String escapeJson(const String& value) const;
    String stateName(SystemState state) const;
};

#endif
