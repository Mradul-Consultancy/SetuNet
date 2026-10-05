#ifndef PORTAL_DETECTOR_H
#define PORTAL_DETECTOR_H

#include <Arduino.h>
#include "HTTPClientLayer.h"

class PortalDetector {
public:
    PortalDetector(HTTPClientLayer* httpClient);
    
    String discoverPortalURL(const String& fallbackURL);
    bool testConnectivity();
    
private:
    HTTPClientLayer* http;
    
    const char* DETECTION_URLS[3] = {
        "http://detectportal.firefox.com/success.txt",
        "http://clients3.google.com/generate_204",
        "http://captive.apple.com/hotspot-detect.html"
    };
    
    bool testDetectionURL(const String& url);
};

#endif
