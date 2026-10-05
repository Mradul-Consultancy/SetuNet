#include "PortalDetector.h"
#include "Logger.h"
#include <WiFi.h>

static bool isRedirectCode(int code) {
    return code == 301 || code == 302 || code == 303 || code == 307 || code == 308;
}

PortalDetector::PortalDetector(HTTPClientLayer* httpClient) : http(httpClient) {}

String PortalDetector::discoverPortalURL(const String& fallbackURL) {
    logger.info("PortalDet", "Discovering portal URL...");
    
    for (int i = 0; i < 3; i++) {
        String url = DETECTION_URLS[i];
        logger.logf(INFO, "PortalDet", "Testing %s", url.c_str());
        
        String response;
        int httpCode = http->httpGET(url, response, false);
        
        if (isRedirectCode(httpCode)) {
            String location = http->getResponseHeader("Location");
            if (location.length() > 0) {
                if (location.startsWith("//")) {
                    location = "http:" + location;
                } else if (location.startsWith("/")) {
                    int schemeEnd = url.indexOf("://");
                    int hostEnd = schemeEnd >= 0 ? url.indexOf('/', schemeEnd + 3) : -1;
                    String origin = hostEnd >= 0 ? url.substring(0, hostEnd) : url;
                    location = origin + location;
                } else if (!location.startsWith("http://") &&
                           !location.startsWith("https://")) {
                    int lastSlash = url.lastIndexOf('/');
                    location = url.substring(0, lastSlash + 1) + location;
                }
                logger.logf(INFO, "PortalDet", "Portal detected via redirect: %s", location.c_str());
                return location;
            }
        }

        String lowerResponse = response;
        lowerResponse.toLowerCase();
        if (httpCode == 200 && lowerResponse.indexOf("<form") >= 0) {
            logger.logf(INFO, "PortalDet", "Portal form detected at %s", url.c_str());
            return url;
        }
    }
    
    IPAddress gateway = WiFi.gatewayIP();
    if (gateway != IPAddress(0, 0, 0, 0)) {
        String gatewayURL = "http://" + gateway.toString();
        logger.logf(INFO, "PortalDet", "Testing gateway: %s", gatewayURL.c_str());
        
        String response;
        int httpCode = http->httpGET(gatewayURL, response, false);
        
        if (httpCode == 200 && response.indexOf("<form") >= 0) {
            logger.info("PortalDet", "Portal detected at gateway");
            return gatewayURL;
        }
    }
    
    logger.logf(INFO, "PortalDet", "Using fallback URL: %s", fallbackURL.c_str());
    return fallbackURL;
}

bool PortalDetector::testConnectivity() {
    String response;
    int httpCode = http->httpGET("http://clients3.google.com/generate_204", response, false);
    
    if (httpCode == 204) {
        logger.info("PortalDet", "Internet accessible (no portal)");
        return true;
    }
    if (isRedirectCode(httpCode)) {
        logger.info("PortalDet", "Captive portal detected");
        return false;
    }

    const char* fallbackURLs[] = {
        "http://detectportal.firefox.com/success.txt",
        "http://captive.apple.com/hotspot-detect.html"
    };
    for (size_t i = 0; i < sizeof(fallbackURLs) / sizeof(fallbackURLs[0]); i++) {
        response = "";
        httpCode = http->httpGET(fallbackURLs[i], response, false);
        response.trim();
        response.toLowerCase();

        if (i == 0 && httpCode == 200 && response == "success") {
            logger.info("PortalDet", "Internet verified by Firefox connectivity endpoint");
            return true;
        }
        if (i == 1 && httpCode == 200 && response.indexOf("success") >= 0 &&
            response.indexOf("<form") < 0) {
            logger.info("PortalDet", "Internet verified by Apple connectivity endpoint");
            return true;
        }
        if (isRedirectCode(httpCode)) {
            logger.info("PortalDet", "Captive portal redirect detected");
            return false;
        }
    }

    logger.logf(WARNING, "PortalDet", "Connectivity could not be verified (last HTTP status: %d)", httpCode);
    return false;
}

bool PortalDetector::testDetectionURL(const String& url) {
    String response;
    int httpCode = http->httpGET(url, response, false);
    return (httpCode == 302 || httpCode == 301);
}
