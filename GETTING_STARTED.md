# Getting started

This guide is for the client-mode prototype. Build success does not confirm the firmware works against the live GLA captive portal. Router mode is not integrated.

## 1. Inspect the live portal

Use a browser on the target network to record the actual login page URL, form action and method, username/password input names, hidden form fields, and redirects. Do not put real credentials or session cookies in documentation or issue reports.

The current firmware submits standard HTML forms with hidden inputs. It does not run JavaScript or support portal-specific challenges.

## 2. Configure

Copy `include/config.h.template` to `include/config.h` and set these macros using your local network/account values:

```cpp
#define WIFI_SSID "your-network"
#define WIFI_PASSWORD "your-wifi-password"
#define PORTAL_URL "https://actual-login-page.example/"
#define PORTAL_USER "your-portal-username"
#define PORTAL_PASS "your-portal-password"
```

Keep `include/config.h` private; Git ignores it. Credentials in this file are compiled into the firmware. Preferences/NVS is not encrypted by this application.

Copy `include/portal_ca.h.template` to `include/portal_ca.h` and add the trusted HTTPS CA certificate for the portal as a C++ raw string literal. The default empty certificate is intentional: HTTPS requests fail closed until you configure a trusted certificate. Never use a certificate copied from an untrusted network as a trust anchor. Set `PORTAL_USERNAME_FIELD` and `PORTAL_PASSWORD_FIELD` in `config.h` if the portal uses non-standard form names.

## 3. Build and upload

Open `esp32-gla-wifi-autologin` in PlatformIO, then run:

```sh
pio run
pio run --target upload
pio device monitor
```

The default build target is `esp32dev`. Change `board` in `platformio.ini` if using another ESP32-compatible board.

## 4. Validate on hardware

Check logs for Wi-Fi connection, login-page fetch, parsed form fields, and external connectivity verification. Test both valid and invalid portal credentials, then test Wi-Fi interruption and recovery. Do not claim deployment readiness until the live login flow and extended stability have been tested.

For current capabilities and security caveats, see `README.md`.
