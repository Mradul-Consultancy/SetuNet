#ifndef HTTP_CLIENT_LAYER_H
#define HTTP_CLIENT_LAYER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <map>

class HTTPClientLayer {
public:
    HTTPClientLayer();
    
    void begin();
    void setTimeout(int timeoutMs);
    void setCACertificate(const String& certificate);
    void setClockReady(bool ready);
    
    int httpGET(const String& url, String& response, bool followRedirects = true);
    int httpPOST(const String& url, const String& postData, String& response);
    
    void setCookie(const String& cookie);
    void addHeader(const String& name, const String& value);
    String getResponseHeader(const String& name);
    
    void clearCookies();
    void clearHeaders();
    
private:
    HTTPClient http;
    WiFiClient client;
    WiFiClientSecure secureClient;
    
    int timeoutMs;
    String caCertificate;
    bool clockReady;
    String cookies;
    std::map<String, String> customHeaders;
    std::map<String, String> responseHeaders;
    
    const int MAX_RESPONSE_SIZE = 16384;
    const int MAX_RETRIES = 3;
    
    void addCommonHeaders();
    void extractSetCookieHeaders();
    int performRequest(bool isPost, const String& url, const String& postData, String& response, bool followRedirects);
};

#endif
