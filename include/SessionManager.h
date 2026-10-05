#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <Arduino.h>
#include "Config.h"

class SessionManager {
public:
    SessionManager();
    
    void storeSession(const String& cookie);
    String getSessionCookie();
    bool isSessionValid();
    unsigned long getSessionAge();
    bool isSessionExpired();
    void clearSession();
    
private:
    AuthSession session;
    const unsigned long SESSION_TIMEOUT = 900000;
};

#endif
