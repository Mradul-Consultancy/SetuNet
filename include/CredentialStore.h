#ifndef CREDENTIAL_STORE_H
#define CREDENTIAL_STORE_H

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

class CredentialStore {
public:
    CredentialStore();
    
    bool begin();
    bool saveCredentials(const Config& config);
    bool loadCredentials(Config& config);
    bool hasCredentials();
    bool clearCredentials();
    
    bool validateCredentials(const Config& config);
    Config getDefaultConfig();
    
private:
    Preferences preferences;
    const char* NAMESPACE = "portal_auth";
    
    bool validateSSID(const char* ssid);
    bool validateURL(const char* url);
    bool validateNonEmpty(const char* str);
};

#endif
