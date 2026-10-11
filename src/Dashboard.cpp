#include "Dashboard.h"
#include "Logger.h"
#include <esp_system.h>

namespace {
const char DASHBOARD_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <meta name="theme-color" content="#101827">
  <title>SetuNet | Device dashboard</title>
  <style>
    :root{color-scheme:dark;--bg:#0b1220;--panel:#111c2d;--line:#223149;--muted:#9aabc0;--text:#edf4ff;--cyan:#53d6d0;--blue:#71a9ff;--green:#58d6a2;--amber:#ffc66d;--red:#ff8383}
    *{box-sizing:border-box}
    body{margin:0;min-height:100vh;background:radial-gradient(ellipse at 14% 0%,#16334b 0,transparent 38%),radial-gradient(ellipse at 90% 12%,#1b2850 0,transparent 32%),var(--bg);color:var(--text);font:15px/1.5 Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif}
    .shell{width:min(1120px,100% - 36px);margin:0 auto;padding:34px 0 48px}
    header{display:flex;align-items:center;justify-content:space-between;gap:18px;margin-bottom:30px}
    .brand{display:flex;align-items:center;gap:13px}.mark{width:46px;height:46px;border-radius:15px;display:grid;place-items:center;background:linear-gradient(145deg,#62e1d2,#619bff);color:#081523;font-size:23px;font-weight:900;box-shadow:0 8px 30px #3cd5c32e}
    h1{font-size:19px;line-height:1.1;margin:0;letter-spacing:.01em}.subbrand{color:var(--muted);font-size:12px;margin-top:5px}
    .device-tag{border:1px solid var(--line);border-radius:999px;padding:8px 12px;color:#bed0e4;font-size:12px;background:#111c2dbb;white-space:nowrap}
    .mode-switch{display:flex;align-items:center;gap:4px;padding:4px;border:1px solid var(--line);border-radius:12px;background:#0b1422}
    .mode-switch button{height:34px;padding:0 12px;border:1px solid transparent;background:transparent;color:var(--muted);box-shadow:none;font-size:12px}
    .mode-switch button[aria-pressed="true"]{background:#1c3046;border-color:#38516e;color:var(--text)}
    .mode-switch button[data-mode="demo"][aria-pressed="true"]{color:var(--amber);border-color:#765c32;background:#3a2d1d}
    .demo-banner{display:none;margin:-4px 0 17px;border:1px solid #765c32;border-radius:12px;padding:12px 15px;background:#302617;color:#f6d9a7;font-size:13px}
    .demo-banner.visible{display:block}
    .demo-tag{display:none;color:var(--amber);font-weight:750}
    .demo-tag.visible{display:inline}
    .intro{display:flex;justify-content:space-between;align-items:end;gap:18px;margin-bottom:20px}.eyebrow{text-transform:uppercase;letter-spacing:.16em;color:var(--cyan);font-size:11px;font-weight:750}
    h2{font-size:clamp(25px,4vw,34px);letter-spacing:-.04em;margin:7px 0 5px}.intro p{color:var(--muted);margin:0;max-width:590px}
    .refresh{font-size:12px;color:var(--muted);white-space:nowrap}
    .grid{display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:15px}
    .card{background:linear-gradient(155deg,#152237ee,#101a2aee);border:1px solid #283951;border-radius:18px;padding:20px;box-shadow:0 14px 34px #00000018}
    .status-card{grid-column:span 4;min-height:153px}.wide{grid-column:span 8}.full{grid-column:span 12}
    .card-title{font-size:12px;text-transform:uppercase;letter-spacing:.1em;color:var(--muted);font-weight:700;display:flex;align-items:center;justify-content:space-between;gap:8px}
    .value{font-size:22px;font-weight:720;letter-spacing:-.03em;margin-top:18px;overflow-wrap:anywhere}.detail{font-size:13px;color:var(--muted);margin-top:6px}
    .pill{display:inline-flex;align-items:center;gap:7px;border:1px solid var(--line);border-radius:999px;padding:5px 9px;font-size:11px;color:var(--muted);font-weight:650;text-transform:none;letter-spacing:0}
    .dot{width:7px;height:7px;border-radius:50%;background:#8091a8}.ok .dot{background:var(--green);box-shadow:0 0 10px #58d6a2aa}.warn .dot{background:var(--amber)}.bad .dot{background:var(--red)}
    .inline{color:var(--cyan)}.form-grid{display:grid;grid-template-columns:1fr 1fr;gap:14px}
    label{display:block;color:#cad6e6;font-size:12px;font-weight:650;margin-bottom:7px}
    input{display:block;width:100%;height:44px;border-radius:10px;border:1px solid #30425b;background:#0b1422;color:var(--text);padding:0 12px;font:inherit;outline:none;transition:border .15s,box-shadow .15s}
    input:focus{border-color:#58d6d0;box-shadow:0 0 0 3px #53d6d01b}input::placeholder{color:#718198}
    .hint{font-size:11px;color:#8192a8;margin-top:6px}.actions{display:flex;align-items:center;gap:12px;flex-wrap:wrap;margin-top:18px}
    button{height:42px;padding:0 16px;border:0;border-radius:10px;background:linear-gradient(110deg,#5de0d1,#7cb5ff);color:#071521;font-weight:750;font-size:13px;cursor:pointer;box-shadow:0 6px 20px #53d6d022}
    button:hover{filter:brightness(1.08)}button:disabled{opacity:.55;cursor:wait}.secondary{border:1px solid #344761;background:#1a2940;color:#e3edfa;box-shadow:none}
    .notice{font-size:12px;color:var(--muted)}.setup-info{display:flex;align-items:center;justify-content:space-between;gap:20px}.setup-ip{font-size:20px;font-weight:700;color:var(--cyan);white-space:nowrap}
    footer{padding:20px 4px 0;color:#7789a0;font-size:11px;display:flex;justify-content:space-between;gap:14px}
    #toast{position:fixed;bottom:22px;left:50%;transform:translate(-50%,20px);opacity:0;pointer-events:none;transition:.2s;background:#18263a;border:1px solid #354963;color:var(--text);padding:11px 16px;border-radius:11px;box-shadow:0 12px 34px #0007;max-width:min(90vw,520px);font-size:13px}
    #toast.show{opacity:1;transform:translate(-50%,0)}.error-text{color:var(--red)}
    @media(max-width:760px){.shell{width:min(100% - 24px,600px);padding-top:22px}.status-card{grid-column:span 6}.wide,.full{grid-column:span 12}.intro{align-items:start;flex-direction:column}.refresh{white-space:normal}}
    @media(max-width:480px){header{align-items:flex-start}.device-tag{font-size:10px}.status-card{grid-column:span 12;min-height:120px}.value{margin-top:12px}.form-grid{grid-template-columns:1fr;gap:12px}.card{padding:17px}.setup-info{align-items:flex-start;flex-direction:column}footer{flex-direction:column}}
  </style>
</head>
<body>
  <main class="shell">
    <header>
      <div class="brand"><div class="mark" aria-hidden="true">S</div><div><h1>SetuNet</h1><div class="subbrand">ESP32 network bridge · device console</div></div></div>
      <div class="mode-switch" role="group" aria-label="Dashboard data mode"><button id="real-mode" type="button" data-mode="real" aria-pressed="true">Real device</button><button id="demo-mode" type="button" data-mode="demo" aria-pressed="false">Demo</button></div>
    </header>
    <div id="demo-banner" class="demo-banner" role="status"><strong>Presentation demo</strong> · Simulated sample data only. No device settings are changed and device actions are disabled.</div>
    <section class="intro">
      <div><div class="eyebrow">Overview</div><h2>Your connection, at a glance.</h2><p>Monitor Wi-Fi and portal status, update your network settings, and retry the connection.</p></div>
      <div id="refresh-label" class="refresh">Live device status · refreshes every 3 seconds</div>
    </section>
    <section class="grid" aria-label="Device dashboard">
      <article class="card status-card"><div class="card-title">Wi-Fi link <span id="wifi-pill" class="pill"><i class="dot"></i><span>Checking</span></span></div><div id="ssid" class="value">—</div><div id="wifi-detail" class="detail">Waiting for device status</div></article>
      <article class="card status-card"><div class="card-title">Internet access <span id="net-pill" class="pill"><i class="dot"></i><span>Checking</span></span></div><div id="net-value" class="value">—</div><div id="net-detail" class="detail">Portal authentication status</div></article>
      <article class="card status-card"><div class="card-title">Device uptime</div><div id="uptime" class="value">—</div><div id="state" class="detail">Current firmware state</div></article>
      <article class="card wide"><div class="card-title">Network details <span id="network-mode-tag" class="inline">SETUP ACCESS POINT</span><span id="demo-tag" class="demo-tag">SIMULATED DATA</span></div><div class="setup-info"><div><div id="ip" class="value">—</div><div id="signal" class="detail">Station IP address · signal strength</div></div><div><div id="ap-ssid" class="detail">Setup network</div><div id="ap-ip" class="setup-ip">192.168.4.1</div></div></div></article>
      <article class="card status-card"><div class="card-title">Authentication runs</div><div id="auth-runs" class="value">0</div><div id="auth-detail" class="detail">Successful logins · failed attempts</div></article>
      <article class="card full">
        <div class="card-title">Connection settings <span class="inline">SAVED ON THIS DEVICE</span></div>
        <p class="detail" style="margin:8px 0 16px">Passwords are never displayed. Leave a password blank to keep the currently saved value.</p>
        <form id="settings-form">
          <div class="form-grid">
            <div><label for="wifi-ssid">Wi-Fi network name</label><input id="wifi-ssid" name="wifi_ssid" maxlength="32" autocomplete="off" required placeholder="Your Wi-Fi SSID"></div>
            <div><label for="wifi-password">Wi-Fi password</label><input id="wifi-password" name="wifi_password" type="password" maxlength="64" autocomplete="new-password" placeholder="Leave blank to keep current"><div class="hint"><label style="display:inline;font-weight:500"><input id="wifi-open" name="wifi_open" value="true" type="checkbox" style="display:inline;width:auto;height:auto;margin:0 5px 0 0"> This is an open Wi-Fi network</label></div></div>
            <div><label for="portal-url">Portal login URL (HTTPS)</label><input id="portal-url" name="portal_url" type="url" maxlength="240" autocomplete="url" required placeholder="https://portal.example/login"></div>
            <div><label for="portal-user">Portal username</label><input id="portal-user" name="portal_username" maxlength="96" autocomplete="username" required placeholder="Your authorized account"></div>
            <div><label for="portal-password">Portal password</label><input id="portal-password" name="portal_password" type="password" maxlength="128" autocomplete="new-password" placeholder="Leave blank to keep current"></div>
            <div><label for="user-field">Username form field <span class="hint">(optional)</span></label><input id="user-field" name="username_field" maxlength="64" placeholder="Auto-detect if blank"></div>
            <div><label for="pass-field">Password form field <span class="hint">(optional)</span></label><input id="pass-field" name="password_field" maxlength="64" placeholder="Auto-detect if blank"></div>
          </div>
          <div class="actions"><button id="save-button" type="submit">Save settings &amp; restart</button><span class="notice">The device restarts to apply changed settings.</span></div>
        </form>
      </article>
      <article class="card full"><div class="card-title">Device action</div><div class="actions" style="margin-top:12px"><button id="retry-button" class="secondary" type="button">Retry connection</button><span class="notice">Retries Wi-Fi and the configured portal login.</span></div></article>
    </section>
    <footer><span>SetuNet · local access only</span><span>Use only networks and accounts you are authorized to access.</span></footer>
  </main>
  <div id="toast" role="status" aria-live="polite"></div>
  <script>
    const token="%TOKEN%";
    const byId=id=>document.getElementById(id);
    let unavailableNotified=false;
    let demoMode=false;
    const demoStartedAt=Date.now();
    const demoSample={state:"monitoring",wifi_connected:true,internet_connected:true,authenticated:true,wifi_ssid:"Campus-Guest",wifi_open:false,wifi_ip:"192.168.1.42",signal_dbm:-54,setup_ssid:"SetuNet-Setup-A3F2",setup_ip:"192.168.4.1",portal_url:"https://portal.demo.example/login",portal_username:"demo.account",username_field:"username",password_field:"password",uptime_seconds:368420,auth_attempts:28,auth_successes:26,auth_failures:2};
    function toast(message,error=false){const el=byId("toast");el.textContent=message;el.classList.toggle("error-text",error);el.classList.add("show");setTimeout(()=>el.classList.remove("show"),3600)}
    function setPill(id,text,kind){const el=byId(id);el.className="pill "+kind;el.lastElementChild.textContent=text}
    function formatUptime(seconds){const d=Math.floor(seconds/86400),h=Math.floor(seconds%86400/3600),m=Math.floor(seconds%3600/60);return d?`${d}d ${h}h`:`${h}h ${m}m`}
    function renderStatus(s){unavailableNotified=false;
      setPill("wifi-pill",s.wifi_connected?"Connected":"Disconnected",s.wifi_connected?"ok":"bad");
      byId("ssid").textContent=s.wifi_ssid||"Not connected";byId("wifi-detail").textContent=s.wifi_connected?"Connected to configured Wi-Fi":"Check Wi-Fi credentials or signal";
      setPill("net-pill",s.internet_connected?"Online":s.state==="setup"?"Setup needed":"Not online",s.internet_connected?"ok":s.state==="error"?"bad":"warn");
      byId("net-value").textContent=s.internet_connected?"Internet ready":s.authenticated?"Checking access":"Portal login pending";byId("net-detail").textContent=s.authenticated?"Portal session authenticated":"Authentication "+s.auth_successes+" successful · "+s.auth_failures+" failed";
      byId("uptime").textContent=formatUptime(s.uptime_seconds);byId("state").textContent="Firmware state · "+s.state.replaceAll("_"," ");
      byId("ip").textContent=s.wifi_ip||"—";byId("signal").textContent=s.wifi_connected?`${s.signal_dbm} dBm · station IP address`:"Station IP address unavailable";
      byId("ap-ssid").textContent="Setup network · "+s.setup_ssid;byId("ap-ip").textContent=s.setup_ip;
      byId("auth-runs").textContent=s.auth_attempts;byId("auth-detail").textContent=s.auth_successes+" successful · "+s.auth_failures+" failed";
      if(!byId("settings-form").dataset.loaded){byId("wifi-ssid").value=s.wifi_ssid||"";byId("portal-url").value=s.portal_url||"";byId("portal-user").value=s.portal_username||"";byId("user-field").value=s.username_field||"";byId("pass-field").value=s.password_field||"";byId("wifi-open").checked=s.wifi_open;byId("wifi-password").disabled=s.wifi_open;byId("settings-form").dataset.loaded="true"}
    }
    function renderDemo(){const sample={...demoSample,uptime_seconds:demoSample.uptime_seconds+Math.floor((Date.now()-demoStartedAt)/1000)};renderStatus(sample)}
    async function refresh(){if(demoMode){renderDemo();return}try{const r=await fetch("/api/status",{cache:"no-store"});if(!r.ok)throw new Error("Status unavailable");renderStatus(await r.json())}catch(e){setPill("wifi-pill","Device unavailable","bad");if(!unavailableNotified){toast("Could not reach the device. Check your setup Wi-Fi connection.",true);unavailableNotified=true}}}
    function setMode(isDemo){demoMode=isDemo;byId("real-mode").setAttribute("aria-pressed",String(!isDemo));byId("demo-mode").setAttribute("aria-pressed",String(isDemo));byId("demo-banner").classList.toggle("visible",isDemo);byId("demo-tag").classList.toggle("visible",isDemo);byId("network-mode-tag").hidden=isDemo;byId("refresh-label").textContent=isDemo?"Presentation data · simulated sample":"Live device status · refreshes every 3 seconds";
      byId("settings-form").querySelectorAll("input,button").forEach(control=>control.disabled=isDemo);
      byId("retry-button").disabled=isDemo;
      if(isDemo){byId("settings-form").dataset.loaded="";byId("wifi-ssid").value=demoSample.wifi_ssid;byId("wifi-password").value="";byId("wifi-open").checked=false;byId("portal-url").value=demoSample.portal_url;byId("portal-user").value=demoSample.portal_username;byId("portal-password").value="";byId("user-field").value=demoSample.username_field;byId("pass-field").value=demoSample.password_field;byId("settings-form").dataset.loaded="demo";renderDemo()}
      else{byId("settings-form").dataset.loaded="";refresh()}}
    byId("real-mode").addEventListener("click",()=>setMode(false));
    byId("demo-mode").addEventListener("click",()=>setMode(true));
    byId("settings-form").addEventListener("submit",async event=>{event.preventDefault();if(demoMode){toast("Demo mode is presentation-only. Switch to Real device to save settings.");return}const button=byId("save-button");button.disabled=true;try{const r=await fetch("/api/config",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded","X-SetuNet-Token":token},body:new URLSearchParams(new FormData(event.currentTarget))});const result=await r.json();if(!r.ok)throw new Error(result.error||"Could not save settings");toast("Settings saved. The device is restarting…")}catch(e){toast(e.message,true)}finally{button.disabled=false}});
    byId("wifi-open").addEventListener("change",event=>{byId("wifi-password").disabled=event.currentTarget.checked;if(event.currentTarget.checked)byId("wifi-password").value=""});
    byId("retry-button").addEventListener("click",async()=>{if(demoMode){toast("Retry is disabled in presentation demo mode.");return}try{const r=await fetch("/api/retry",{method:"POST",headers:{"X-SetuNet-Token":token}});const result=await r.json();if(!r.ok)throw new Error(result.error||"Retry failed");toast("Connection retry started.");}catch(e){toast(e.message,true)}});
    refresh();setInterval(refresh,3000);
  </script>
</body>
</html>
)HTML";

String jsonString(const String& value) {
    String result;
    result.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); i++) {
        const char c = value.charAt(i);
        if (c == '"' || c == '\\') {
            result += '\\';
            result += c;
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (static_cast<uint8_t>(c) < 0x20) {
            result += ' ';
        } else {
            result += c;
        }
    }
    return result;
}
}

Dashboard::Dashboard()
    : server(80), config(nullptr), credentialStore(nullptr), apManager(nullptr),
      status(nullptr), retryRequested(false), restartRequested(false) {}

bool Dashboard::begin(Config& configRef, CredentialStore& credentialStoreRef,
                      APManager& apManagerRef, SystemStatus& statusRef) {
    config = &configRef;
    credentialStore = &credentialStoreRef;
    apManager = &apManagerRef;
    status = &statusRef;

    char token[33];
    snprintf(token, sizeof(token), "%08lx%08lx%08lx%08lx",
             static_cast<unsigned long>(esp_random()),
             static_cast<unsigned long>(esp_random()),
             static_cast<unsigned long>(esp_random()),
             static_cast<unsigned long>(esp_random()));
    csrfToken = token;

    const char* headerKeys[] = {"X-SetuNet-Token"};
    server.collectHeaders(headerKeys, 1);
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/config", HTTP_POST, [this]() { handleSave(); });
    server.on("/api/retry", HTTP_POST, [this]() { handleRetry(); });
    server.onNotFound([this]() {
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });
    server.begin();
    logger.info("Dashboard", "Local dashboard server started on port 80");
    return true;
}

void Dashboard::handleClient() {
    server.handleClient();
}

bool Dashboard::consumeRetryRequest() {
    const bool requested = retryRequested;
    retryRequested = false;
    return requested;
}

bool Dashboard::consumeRestartRequest() {
    const bool requested = restartRequested;
    restartRequested = false;
    return requested;
}

void Dashboard::handleRoot() {
    String page(FPSTR(DASHBOARD_HTML));
    page.replace("%TOKEN%", csrfToken);
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/html; charset=utf-8", page);
}

void Dashboard::handleStatus() {
    String response;
    response.reserve(420);
    response = "{\"state\":\"" + stateName(status->currentState) +
        "\",\"wifi_connected\":" + String(status->wifiConnected ? "true" : "false") +
        ",\"internet_connected\":" + String(status->authenticated ? "true" : "false") +
        ",\"authenticated\":" + String(status->authenticated ? "true" : "false") +
        ",\"wifi_ssid\":\"" + escapeJson(status->wifiConnected ? WiFi.SSID() : config->wifiSSID) +
        "\",\"wifi_open\":" + String(config->wifiPassword.length() == 0 ? "true" : "false") +
        "\",\"wifi_ip\":\"" + escapeJson(status->ipAddress) +
        "\",\"signal_dbm\":" + String(status->signalStrength) +
        ",\"setup_ssid\":\"" + escapeJson(WiFi.softAPSSID()) +
        "\",\"setup_ip\":\"" + escapeJson(apManager->getAPIP().toString()) +
        "\",\"portal_url\":\"" + escapeJson(config->portalURL) +
        "\",\"portal_username\":\"" + escapeJson(config->portalUsername) +
        "\",\"username_field\":\"" + escapeJson(config->portalUsernameField) +
        "\",\"password_field\":\"" + escapeJson(config->portalPasswordField) +
        "\",\"uptime_seconds\":" + String(millis() / 1000) +
        ",\"auth_attempts\":" + String(status->authAttempts) +
        ",\"auth_successes\":" + String(status->authSuccesses) +
        ",\"auth_failures\":" + String(status->authFailures) + "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json; charset=utf-8", response);
}

void Dashboard::handleSave() {
    if (!hasValidToken()) {
        server.send(403, "application/json", "{\"error\":\"Dashboard request expired. Reload the page and try again.\"}");
        return;
    }

    Config updated = *config;
    updated.wifiSSID = server.arg("wifi_ssid");
    updated.portalURL = server.arg("portal_url");
    updated.portalUsername = server.arg("portal_username");
    updated.portalUsernameField = server.arg("username_field");
    updated.portalPasswordField = server.arg("password_field");

    const String wifiPassword = server.arg("wifi_password");
    if (server.arg("wifi_open") == "true") {
        updated.wifiPassword = "";
    } else if (wifiPassword.length() > 0) {
        updated.wifiPassword = wifiPassword;
    }

    const String portalPassword = server.arg("portal_password");
    if (portalPassword.length() > 0) {
        updated.portalPassword = portalPassword;
    }

    if (!credentialStore->validateCredentials(updated)) {
        server.send(400, "application/json", "{\"error\":\"Invalid settings. Check the Wi-Fi name/password, HTTPS portal URL, and portal credentials.\"}");
        return;
    }

    if (!credentialStore->saveCredentials(updated)) {
        server.send(500, "application/json", "{\"error\":\"Could not save settings to device storage.\"}");
        return;
    }

    *config = updated;
    server.send(200, "application/json", "{\"message\":\"Settings saved; restarting.\"}");
    restartRequested = true;
    logger.info("Dashboard", "Configuration saved; device restart requested");
}

void Dashboard::handleRetry() {
    if (!hasValidToken()) {
        server.send(403, "application/json", "{\"error\":\"Dashboard request expired. Reload the page and try again.\"}");
        return;
    }
    if (!credentialStore->validateCredentials(*config)) {
        server.send(409, "application/json", "{\"error\":\"Set valid network and portal settings before retrying.\"}");
        return;
    }
    retryRequested = true;
    server.send(202, "application/json", "{\"message\":\"Connection retry queued.\"}");
}

bool Dashboard::hasValidToken() {
    return server.header("X-SetuNet-Token") == csrfToken;
}

String Dashboard::escapeJson(const String& value) const {
    return jsonString(value);
}

String Dashboard::stateName(SystemState state) const {
    switch (state) {
        case STATE_BOOT: return "boot";
        case STATE_LOAD_CONFIG: return "load_config";
        case STATE_WIFI_CONNECT: return "wifi_connect";
        case STATE_WIFI_CONNECTED: return "wifi_connected";
        case STATE_PORTAL_DETECT: return "portal_detect";
        case STATE_PORTAL_AUTH: return "portal_auth";
        case STATE_AUTHENTICATED: return "authenticated";
        case STATE_MONITORING: return "monitoring";
        case STATE_SETUP: return "setup";
        case STATE_ERROR: return "error";
        case STATE_RESTART: return "restart";
        default: return "unknown";
    }
}
