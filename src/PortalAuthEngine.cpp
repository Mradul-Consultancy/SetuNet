#include "PortalAuthEngine.h"
#include "Logger.h"

static String decodeHtmlEntities(String value) {
    value.replace("&amp;", "&");
    value.replace("&quot;", "\"");
    value.replace("&#39;", "'");
    value.replace("&lt;", "<");
    value.replace("&gt;", ">");
    return value;
}

static String htmlAttribute(const String& tag, const char* attribute) {
    String lowerTag = tag;
    lowerTag.toLowerCase();
    String key(attribute);
    key.toLowerCase();
    int searchFrom = 0;

    while (true) {
        int keyStart = lowerTag.indexOf(key, searchFrom);
        if (keyStart < 0) {
            return "";
        }
        bool boundaryBefore = keyStart == 0 || isspace(lowerTag.charAt(keyStart - 1));
        int cursor = keyStart + key.length();
        while (cursor < lowerTag.length() && isspace(lowerTag.charAt(cursor))) {
            cursor++;
        }
        if (boundaryBefore && cursor < lowerTag.length() && lowerTag.charAt(cursor) == '=') {
            cursor++;
            while (cursor < lowerTag.length() && isspace(lowerTag.charAt(cursor))) {
                cursor++;
            }
            if (cursor >= lowerTag.length()) {
                return "";
            }

            char quote = lowerTag.charAt(cursor);
            if (quote == '"' || quote == '\'') {
                cursor++;
                int valueEnd = lowerTag.indexOf(quote, cursor);
                return valueEnd < 0 ? "" : decodeHtmlEntities(tag.substring(cursor, valueEnd));
            }

            int valueEnd = cursor;
            while (valueEnd < lowerTag.length() &&
                   !isspace(lowerTag.charAt(valueEnd)) &&
                   lowerTag.charAt(valueEnd) != '>') {
                valueEnd++;
            }
            return decodeHtmlEntities(tag.substring(cursor, valueEnd));
        }
        searchFrom = keyStart + key.length();
    }
}

PortalAuthEngine::PortalAuthEngine(HTTPClientLayer* httpClient, PortalDetector* detector) 
    : http(httpClient), detector(detector), authenticated(false), lastAuthAttempt(0) {}

bool PortalAuthEngine::begin(const String& username, const String& password, const String& portalURL,
                             const String& usernameField, const String& passwordField) {
    this->username = username;
    this->password = password;
    this->portalURL = portalURL;
    configuredUsernameField = usernameField;
    configuredPasswordField = passwordField;
    
    logger.info("PortalAuth", "Initialized");
    return true;
}

bool PortalAuthEngine::authenticate() {
    unsigned long now = millis();
    if (now - lastAuthAttempt < MIN_AUTH_INTERVAL) {
        logger.warning("PortalAuth", "Rate limiting: too soon since last attempt");
        return false;
    }
    lastAuthAttempt = now;
    http->clearCookies();
    http->clearHeaders();
    
    logger.info("PortalAuth", "Starting authentication...");
    
    String discoveredURL = detector->discoverPortalURL(portalURL);
    
    logger.logf(INFO, "PortalAuth", "Fetching login page: %s", discoveredURL.c_str());
    String loginPage;
    int httpCode = http->httpGET(discoveredURL, loginPage);
    
    if (httpCode <= 0) {
        logger.error("PortalAuth", "Failed to fetch login page");
        return false;
    }
    
    LoginFormData formData = parseHTMLForm(loginPage);
    if (configuredUsernameField.length() > 0) {
        formData.usernameField = configuredUsernameField;
    }
    if (configuredPasswordField.length() > 0) {
        formData.passwordField = configuredPasswordField;
    }
    
    if (formData.passwordField.length() == 0) {
        logger.error("PortalAuth", "Login form has no password input");
        return false;
    }
    if (formData.action.length() == 0) {
        formData.action = discoveredURL;
    }
    
    logger.logf(INFO, "PortalAuth", "Form action: %s", formData.action.c_str());
    logger.logf(INFO, "PortalAuth", "Username field: %s", formData.usernameField.c_str());
    logger.logf(INFO, "PortalAuth", "Password field: %s", formData.passwordField.c_str());
    
    String postData = buildPOSTData(formData);
    String actionURL = formData.action;
    if (actionURL.startsWith("//")) {
        int schemeEnd = discoveredURL.indexOf("://");
        if (schemeEnd < 0) {
            logger.error("PortalAuth", "Cannot resolve protocol-relative form action");
            return false;
        }
        actionURL = discoveredURL.substring(0, schemeEnd + 1) + actionURL;
    } else if (!actionURL.startsWith("http://") && !actionURL.startsWith("https://")) {
        if (actionURL.startsWith("/")) {
            int thirdSlash = discoveredURL.indexOf('/', 8);
            String baseURL = thirdSlash > 0 ? discoveredURL.substring(0, thirdSlash) : discoveredURL;
            actionURL = baseURL + actionURL;
        } else {
            int lastSlash = discoveredURL.lastIndexOf('/');
            String baseURL = discoveredURL.substring(0, lastSlash + 1);
            actionURL = baseURL + actionURL;
        }
    }
    
    logger.logf(INFO, "PortalAuth", "Submitting to: %s", actionURL.c_str());
    
    http->addHeader("Referer", discoveredURL);
    
    String response;
    String method = formData.method;
    method.toUpperCase();
    if (method != "POST") {
        logger.error("PortalAuth", "Refusing non-POST login form to keep credentials out of URLs");
        return false;
    }
    if (!actionURL.startsWith("https://")) {
        logger.error("PortalAuth", "Refusing to submit credentials over an unencrypted connection");
        return false;
    }
    httpCode = http->httpPOST(actionURL, postData, response);
    
    if (httpCode <= 0) {
        logger.error("PortalAuth", "Authentication request failed");
        return false;
    }
    
    sessionCookie = http->getResponseHeader("Set-Cookie");
    
    logger.info("PortalAuth", "Verifying external connectivity after form submission");
    if (detector->testConnectivity()) {
        authenticated = true;
        logger.info("PortalAuth", "Authentication verified by connectivity check");
        return true;
    }

    logger.error("PortalAuth", "Authentication could not be verified by connectivity check");
    return false;
}

bool PortalAuthEngine::isAuthenticated() {
    return authenticated;
}

void PortalAuthEngine::clearSession() {
    authenticated = false;
    sessionCookie = "";
    http->clearCookies();
    logger.info("PortalAuth", "Session cleared");
}

String PortalAuthEngine::getSessionCookie() {
    return sessionCookie;
}

LoginFormData PortalAuthEngine::parseHTMLForm(const String& html) {
    String lowerHTML = html;
    lowerHTML.toLowerCase();
    int formStart = 0;

    while ((formStart = lowerHTML.indexOf("<form", formStart)) >= 0) {
        int formTagEnd = html.indexOf('>', formStart);
        if (formTagEnd < 0) {
            break;
        }
        int formEnd = lowerHTML.indexOf("</form>", formTagEnd);
        if (formEnd < 0) {
            formEnd = html.length();
        }

        String formTag = html.substring(formStart, formTagEnd + 1);
        String formHTML = html.substring(formTagEnd + 1, formEnd);
        LoginFormData candidate;
        candidate.action = htmlAttribute(formTag, "action");
        candidate.method = htmlAttribute(formTag, "method");
        if (candidate.method.length() == 0) {
            candidate.method = "GET";
        }

        String lowerForm = formHTML;
        lowerForm.toLowerCase();
        int inputStart = 0;
        while ((inputStart = lowerForm.indexOf("<input", inputStart)) >= 0) {
            int inputEnd = formHTML.indexOf('>', inputStart);
            if (inputEnd < 0) {
                break;
            }
            String inputTag = formHTML.substring(inputStart, inputEnd + 1);
            String name = htmlAttribute(inputTag, "name");
            String type = htmlAttribute(inputTag, "type");
            String value = htmlAttribute(inputTag, "value");
            String autocomplete = htmlAttribute(inputTag, "autocomplete");
            String lowerName = name;
            String lowerType = type;
            String lowerAutocomplete = autocomplete;
            lowerName.toLowerCase();
            lowerType.toLowerCase();
            lowerAutocomplete.toLowerCase();

            if (name == configuredUsernameField) {
                candidate.usernameField = name;
            }
            if (name == configuredPasswordField) {
                candidate.passwordField = name;
            }

            if (name.length() > 0 && lowerType == "password") {
                candidate.passwordField = name;
            } else if (name.length() > 0 && lowerType == "hidden") {
                candidate.hiddenFields[name] = value;
            } else if (name.length() > 0 && candidate.usernameField.length() == 0 &&
                       (lowerType == "text" || lowerType == "email" ||
                        lowerAutocomplete == "username" ||
                        lowerName.indexOf("user") >= 0 ||
                        lowerName.indexOf("login") >= 0 ||
                        lowerName.indexOf("email") >= 0 ||
                        lowerName.indexOf("account") >= 0)) {
                candidate.usernameField = name;
            }
            inputStart = inputEnd + 1;
        }

        if (candidate.passwordField.length() > 0) {
            if (candidate.usernameField.length() == 0) {
                candidate.usernameField = "username";
            }
            return candidate;
        }
        formStart = formEnd + 7;
    }

    return LoginFormData();
}

String PortalAuthEngine::buildPOSTData(const LoginFormData& formData) {
    String postData = "";
    
    postData += urlEncode(formData.usernameField) + "=" + urlEncode(username);
    postData += "&" + urlEncode(formData.passwordField) + "=" + urlEncode(password);
    
    for (auto& field : formData.hiddenFields) {
        if (field.first != formData.usernameField && field.first != formData.passwordField) {
            postData += "&" + urlEncode(field.first) + "=" + urlEncode(field.second);
        }
    }
    
    return postData;
}

String PortalAuthEngine::urlEncode(const String& str) {
    String encoded = "";
    static const char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < str.length(); i++) {
        unsigned char c = static_cast<unsigned char>(str.charAt(i));
        if (c == ' ') {
            encoded += '+';
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '*') {
            encoded += static_cast<char>(c);
        } else {
            encoded += '%';
            encoded += hex[c >> 4];
            encoded += hex[c & 0x0f];
        }
    }
    
    return encoded;
}
