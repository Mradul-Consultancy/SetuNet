#include "HTTPClientLayer.h"
#include "Logger.h"

HTTPClientLayer::HTTPClientLayer() : timeoutMs(15000), sslValidation(false) {}

void HTTPClientLayer::begin() {
    logger.info("HTTPClient", "Initialized");
}

void HTTPClientLayer::setTimeout(int timeoutMs) {
    this->timeoutMs = timeoutMs;
}

void HTTPClientLayer::setSSLValidation(bool validate) {
    this->sslValidation = validate;
}

int HTTPClientLayer::httpGET(const String& url, String& response, bool followRedirects) {
    return performRequest(false, url, "", response, followRedirects);
}

int HTTPClientLayer::httpPOST(const String& url, const String& postData, String& response) {
    return performRequest(true, url, postData, response, true);
}

void HTTPClientLayer::setCookie(const String& cookie) {
    if (cookies.length() > 0) {
        cookies += "; ";
    }
    cookies += cookie;
}

void HTTPClientLayer::addHeader(const String& name, const String& value) {
    customHeaders[name] = value;
}

String HTTPClientLayer::getResponseHeader(const String& name) {
    if (responseHeaders.find(name) != responseHeaders.end()) {
        return responseHeaders[name];
    }
    return "";
}

void HTTPClientLayer::clearCookies() {
    cookies = "";
}

void HTTPClientLayer::clearHeaders() {
    customHeaders.clear();
}

void HTTPClientLayer::addCommonHeaders() {
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    if (cookies.length() > 0) {
        http.addHeader("Cookie", cookies);
    }
    
    for (auto& header : customHeaders) {
        http.addHeader(header.first, header.second);
    }
}

void HTTPClientLayer::extractSetCookieHeaders() {
    int headerCount = http.headers();
    for (int i = 0; i < headerCount; i++) {
        String headerName = http.headerName(i);
        String headerValue = http.header(i);
        
        responseHeaders[headerName] = headerValue;
        
        if (headerName.equalsIgnoreCase("Set-Cookie")) {
            int semicolon = headerValue.indexOf(';');
            String cookieValue = semicolon > 0 ? headerValue.substring(0, semicolon) : headerValue;
            setCookie(cookieValue);
            logger.debug("HTTPClient", "Stored session cookie");
        }
    }
}

int HTTPClientLayer::performRequest(bool isPost, const String& url, const String& postData, 
                                    String& response, bool followRedirects) {
    response = "";
    responseHeaders.clear();
    
    logger.logf(INFO, "HTTPClient", "%s %s", isPost ? "POST" : "GET", url.c_str());
    
    bool isHTTPS = url.startsWith("https://");
    
    if (isHTTPS) {
        if (!sslValidation) {
            secureClient.setInsecure();
        }
        http.begin(secureClient, url);
    } else {
        http.begin(client, url);
    }
    
    http.setTimeout(timeoutMs);
    http.setFollowRedirects(followRedirects ? HTTPC_STRICT_FOLLOW_REDIRECTS : HTTPC_DISABLE_FOLLOW_REDIRECTS);
    static const char* responseHeaderNames[] = {
        "Location",
        "Set-Cookie",
        "Content-Type"
    };
    http.collectHeaders(responseHeaderNames, sizeof(responseHeaderNames) / sizeof(responseHeaderNames[0]));
    
    addCommonHeaders();
    
    if (isPost) {
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    }
    
    int httpCode = -1;
    int retries = 0;
    
    while (retries < MAX_RETRIES) {
        if (isPost) {
            httpCode = http.POST(postData);
        } else {
            httpCode = http.GET();
        }
        
        if (httpCode > 0) {
            break;
        }
        
        retries++;
        logger.logf(WARNING, "HTTPClient", "Request failed, retry %d/%d", retries, MAX_RETRIES);
        delay(1000);
    }
    
    if (httpCode > 0) {
        logger.logf(INFO, "HTTPClient", "Response code: %d", httpCode);
        
        extractSetCookieHeaders();
        
        int contentLength = http.getSize();
        if (contentLength > MAX_RESPONSE_SIZE) {
            logger.logf(WARNING, "HTTPClient", "Response too large (%d bytes), truncating", contentLength);
        }
        
        WiFiClient* stream = http.getStreamPtr();
        int bytesRead = 0;
        
        while (http.connected() && (contentLength > 0 || contentLength == -1)) {
            size_t available = stream->available();
            
            if (available) {
                char buffer[128];
                int c = stream->readBytes(buffer, min((size_t)sizeof(buffer), available));
                
                if (bytesRead + c <= MAX_RESPONSE_SIZE) {
                    response.concat(buffer, c);
                    bytesRead += c;
                } else {
                    break;
                }
                
                if (contentLength > 0) {
                    contentLength -= c;
                }
            }
            
            if (contentLength == 0) {
                break;
            }
            
            delay(1);
        }
        
        logger.logf(DEBUG, "HTTPClient", "Read %d bytes", bytesRead);
    } else {
        logger.logf(ERROR, "HTTPClient", "Request failed: %s", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return httpCode;
}
