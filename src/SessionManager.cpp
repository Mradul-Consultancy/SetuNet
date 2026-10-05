#include "SessionManager.h"
#include "Logger.h"

SessionManager::SessionManager() {
    session.cookie = "";
    session.timestamp = 0;
    session.isValid = false;
}

void SessionManager::storeSession(const String& cookie) {
    session.cookie = cookie;
    session.timestamp = millis();
    session.isValid = true;
    
    logger.logf(INFO, "SessionMgr", "Session stored: %s", cookie.substring(0, 20).c_str());
}

String SessionManager::getSessionCookie() {
    return session.cookie;
}

bool SessionManager::isSessionValid() {
    return session.isValid && !isSessionExpired();
}

unsigned long SessionManager::getSessionAge() {
    if (!session.isValid) {
        return 0;
    }
    return millis() - session.timestamp;
}

bool SessionManager::isSessionExpired() {
    if (!session.isValid) {
        return true;
    }
    return getSessionAge() > SESSION_TIMEOUT;
}

void SessionManager::clearSession() {
    session.cookie = "";
    session.timestamp = 0;
    session.isValid = false;
    
    logger.info("SessionMgr", "Session cleared");
}
