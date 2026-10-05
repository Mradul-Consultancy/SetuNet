#ifndef PORTAL_AUTH_ENGINE_H
#define PORTAL_AUTH_ENGINE_H

#include <Arduino.h>
#include "HTTPClientLayer.h"
#include "PortalDetector.h"
#include "Config.h"

class PortalAuthEngine {
public:
    PortalAuthEngine(HTTPClientLayer* httpClient, PortalDetector* detector);
    
    bool begin(const String& username, const String& password, const String& portalURL);
    bool authenticate();
    bool isAuthenticated();
    void clearSession();
    
    String getSessionCookie();
    
private:
    HTTPClientLayer* http;
    PortalDetector* detector;
    
    String username;
    String password;
    String portalURL;
    String sessionCookie;
    bool authenticated;
    unsigned long lastAuthAttempt;
    
    const int MIN_AUTH_INTERVAL = 5000;
    
    LoginFormData parseHTMLForm(const String& html);
    String buildPOSTData(const LoginFormData& formData);
    String urlEncode(const String& str);
};

#endif
