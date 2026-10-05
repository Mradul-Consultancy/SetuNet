# PATENT FIGURES - DETAILED DESCRIPTIONS FOR AI GENERATION
# ESP32 Automated Captive Portal Authentication System

## FIGURE 1: System Architecture Block Diagram

### Description
A layered architecture diagram showing four horizontal layers with components and data flow arrows.

### Detailed Layout
**Layer 1 - Application Layer (Top)**
- Box: "Main State Machine"
- Box: "Authentication Flow Controller"
- Arrows pointing down to Layer 2

**Layer 2 - Component Layer**
- Left side boxes:
  * "WiFi Manager"
  * "Portal Detector"
  * "Portal Auth Engine"
  * "Session Manager"
  * "Connectivity Monitor"
- Right side boxes (Router Mode):
  * "AP Manager"
  * "NAT Router"
  * "DNS Forwarder"
  * "MAC Manager"
- Arrows connecting components horizontally and pointing down to Layer 3

**Layer 3 - Transport Layer**
- Box: "HTTP Client Layer"
- Box: "WiFi Stack (lwIP)"
- Box: "Credential Store (NVS)"
- Arrows pointing down to Layer 4

**Layer 4 - Hardware Layer (Bottom)**
- Box: "ESP32-WROOM-32"
  * Sub-labels: "Dual-core @ 240MHz"
  * "520 KB RAM"
  * "4 MB Flash"
- Box: "WiFi Radio (2.4 GHz)"
- Box: "Non-Volatile Storage"

### Color Scheme
- Application Layer: Light blue
- Component Layer: Green (client mode), Orange (router mode)
- Transport Layer: Yellow
- Hardware Layer: Gray

### Prompt for AI Tools
"Create a technical block diagram with 4 horizontal layers. Top layer has 'Main State Machine' and 'Authentication Flow Controller'. Second layer has 8 boxes: WiFi Manager, Portal Detector, Portal Auth Engine, Session Manager, Connectivity Monitor on left; AP Manager, NAT Router, DNS Forwarder, MAC Manager on right. Third layer has HTTP Client Layer, WiFi Stack, Credential Store. Bottom layer shows ESP32-WROOM-32 hardware with specs. Use arrows showing data flow downward. Professional patent diagram style, black and white with clear labels."

---

## FIGURE 2: State Machine Diagram

### Description
A circular state transition diagram with 9 states and labeled transition arrows.

### Detailed Layout
**States (circles with labels):**
1. BOOT (top center)
2. LOAD_CONFIG (clockwise from BOOT)
3. WIFI_CONNECT
4. PORTAL_DETECT
5. PORTAL_AUTH
6. AUTHENTICATED
7. MONITORING (bottom center)
8. ERROR (left side)
9. RESTART (returns to BOOT)

**Transitions (arrows with labels):**
- BOOT → LOAD_CONFIG: "System Start"
- LOAD_CONFIG → WIFI_CONNECT: "Credentials Loaded"
- WIFI_CONNECT → PORTAL_DETECT: "WiFi Connected"
- WIFI_CONNECT → ERROR: "Connection Failed"
- PORTAL_DETECT → PORTAL_AUTH: "Portal Detected"
- PORTAL_DETECT → AUTHENTICATED: "No Portal"
- PORTAL_AUTH → AUTHENTICATED: "Auth Success"
- PORTAL_AUTH → ERROR: "Auth Failed"
- AUTHENTICATED → MONITORING: "Start Monitor"
- MONITORING → PORTAL_DETECT: "Connectivity Lost"
- MONITORING → MONITORING: "Check OK (60s loop)"
- MONITORING → RESTART: "10 Failures"
- ERROR → RESTART: "Wait 60s"
- RESTART → BOOT: "ESP.restart()"

### Prompt for AI Tools
"Create a state machine diagram with 9 circular nodes arranged in a flow. Nodes labeled: BOOT, LOAD_CONFIG, WIFI_CONNECT, PORTAL_DETECT, PORTAL_AUTH, AUTHENTICATED, MONITORING, ERROR, RESTART. Draw arrows between states with transition labels. Main flow goes clockwise from BOOT through states to MONITORING. ERROR and RESTART states on the side. MONITORING has self-loop labeled '60s check'. Professional technical diagram, black and white."

---

## FIGURE 3: Portal Detection Flowchart

### Description
A vertical flowchart showing the multi-method portal detection algorithm.

### Detailed Layout
**Start** (rounded rectangle)
↓
**Initialize Detection URLs** (rectangle)
- clients3.google.com/generate_204
- detectportal.firefox.com
- captive.apple.com/hotspot-detect.html
- connectivitycheck.gstatic.com/generate_204
↓
**For Each URL** (loop start)
↓
**Send HTTP GET (no redirect)** (rectangle)
↓
**Response Code?** (diamond)
- If 301/302 → **Extract Redirect URL** → **Return Portal URL** (end)
- If 204 → Continue loop
↓
**All URLs Tested?** (diamond)
- No → Loop back
- Yes → Continue
↓
**Probe Gateway IP** (rectangle)
↓
**Response Contains <form>?** (diamond)
- Yes → **Return Gateway URL** (end)
- No → Continue
↓
**Return Fallback URL** (rectangle)
↓
**End** (rounded rectangle)

### Prompt for AI Tools
"Create a vertical flowchart for portal detection algorithm. Start with 'Initialize Detection URLs' box listing 4 URLs. Flow through 'For Each URL' loop, 'Send HTTP GET', decision diamond 'Response Code 301/302?', branches to 'Extract Redirect URL' or continue. After loop, 'Probe Gateway IP', decision 'Contains form tag?', branches to return gateway URL or fallback URL. Use standard flowchart symbols: rectangles for processes, diamonds for decisions, rounded rectangles for start/end. Black and white, clear labels."

---

## FIGURE 4: HTML Form Parsing Algorithm

### Description
A flowchart showing the adaptive HTML form parsing process.

### Detailed Layout
**Start** (rounded rectangle)
↓
**Download HTML Page** (rectangle)
↓
**Find <form> Tag** (rectangle)
↓
**Extract Form Boundaries** (rectangle)
- form_start = indexOf("<form")
- form_end = indexOf("</form>")
↓
**Extract Form Attributes** (rectangle)
- action URL
- method (POST/GET)
↓
**Find All <input> Tags** (rectangle)
↓
**For Each Input** (loop)
↓
**Extract Attributes** (rectangle)
- name
- type
- value
↓
**Check Type** (diamond with 3 branches)
- type="password" → **Store as passwordField**
- type="text" or "email" → **Store as usernameField**
- type="hidden" → **Store in hiddenFields map**
↓
**All Inputs Processed?** (diamond)
- No → Loop back
- Yes → Continue
↓
**Build LoginFormData** (rectangle)
- action, method
- usernameField, passwordField
- hiddenFields
↓
**Return FormData** (rounded rectangle)
↓
**End**

### Prompt for AI Tools
"Create a flowchart for HTML form parsing. Start with 'Download HTML Page', flow to 'Find form tag', 'Extract form boundaries', 'Extract attributes'. Then 'For each input tag' loop with 'Extract name, type, value'. Decision diamond with 3 branches: 'type=password' goes to 'Store passwordField', 'type=text/email' goes to 'Store usernameField', 'type=hidden' goes to 'Store hiddenFields'. Loop until all inputs processed. End with 'Build LoginFormData' and 'Return'. Standard flowchart style, black and white."

---

## FIGURE 5: Router Mode Network Topology

### Description
A network diagram showing the dual-interface architecture with client devices.

### Detailed Layout
**Left Side - Internet Cloud**
- Cloud shape labeled "Internet"
↓
**GLA WiFi Network**
- Router icon labeled "GLA WiFi Router"
- IP: 172.16.92.1
↓
**ESP32 Device (Center - Large Box)**
- Top half: "STA Interface"
  * IP: 172.16.92.86
  * Connected to GLA WiFi
- Bottom half: "AP Interface"
  * IP: 192.168.4.1
  * SSID: ESP32-Router
- Middle: "NAT Router + DNS Forwarder"
↓
**Right Side - Client Devices**
- Laptop icon: 192.168.4.2
- Smartphone icon: 192.168.4.3
- Tablet icon: 192.168.4.4
- IoT Device icon: 192.168.4.5

**Arrows showing data flow:**
- Bidirectional arrows between Internet ↔ GLA Router ↔ ESP32 STA
- Bidirectional arrows between ESP32 AP ↔ Client Devices
- Label on ESP32: "NAT Translation: 192.168.4.x ↔ 172.16.92.x"

### Prompt for AI Tools
"Create a network topology diagram. Left side: Internet cloud connected to GLA WiFi Router (172.16.92.1). Center: Large ESP32 box with two sections - top 'STA Interface' (172.16.92.86) connected to GLA WiFi, bottom 'AP Interface' (192.168.4.1) labeled 'ESP32-Router'. Middle of ESP32 shows 'NAT Router + DNS Forwarder'. Right side: 4 client device icons (laptop, phone, tablet, IoT device) with IPs 192.168.4.2-5 connected to ESP32 AP. Bidirectional arrows showing data flow. Professional network diagram style, black and white."

---

## FIGURE 6: NAT Translation Process

### Description
A detailed diagram showing packet translation through the NAT table.

### Detailed Layout
**Top Section - Incoming Packet from Client**
- Box: "Client Device (192.168.4.2:54321)"
- Arrow down labeled "HTTP Request to google.com"
- Packet details box:
  * Source: 192.168.4.2:54321
  * Dest: 142.250.185.46:80

**Middle Section - NAT Translation Table**
- Table with columns:
  | Client IP | Client Port | External IP | External Port | Last Activity |
  |-----------|-------------|-------------|---------------|---------------|
  | 192.168.4.2 | 54321 | 172.16.92.86 | 54321 | timestamp |
  | 192.168.4.3 | 49152 | 172.16.92.86 | 49152 | timestamp |
  | ... | ... | ... | ... | ... |

**ESP32 NAT Router Box**
- "Translate Source IP:Port"
- "192.168.4.2:54321 → 172.16.92.86:54321"

**Bottom Section - Outgoing Packet to Internet**
- Arrow down labeled "Translated Packet"
- Packet details box:
  * Source: 172.16.92.86:54321
  * Dest: 142.250.185.46:80
- Box: "Internet (Google Server)"

**Return Path (Right Side)**
- Arrow up from Internet
- "Response Packet"
- Source: 142.250.185.46:80
- Dest: 172.16.92.86:54321
- NAT Router: "Reverse Translation"
- 172.16.92.86:54321 → 192.168.4.2:54321
- Arrow to Client Device

### Prompt for AI Tools
"Create a technical diagram showing NAT packet translation. Top: Client device box (192.168.4.2:54321) sending packet with source/dest details. Middle: NAT translation table with 5 columns showing IP/port mappings. Center: ESP32 NAT Router box showing translation '192.168.4.2:54321 → 172.16.92.86:54321'. Bottom: Translated packet going to Internet. Right side shows return path with reverse translation. Use boxes, arrows, and packet detail callouts. Professional technical diagram, black and white."

---

## FIGURE 7: DNS Caching Architecture

### Description
A flowchart showing DNS query handling with LRU cache.

### Detailed Layout
**Top - Client Device**
- Box: "Client Device"
- Arrow down: "DNS Query: www.google.com"

**ESP32 DNS Forwarder (Large Box)**

**Step 1 - Cache Check**
- Diamond: "Domain in Cache?"
- Yes branch → Diamond: "Cache Entry Valid (TTL)?"
  * Yes → Box: "Update LRU Order" → Box: "Return Cached IP" → End
  * No → Box: "Remove Expired Entry" → Continue

**Step 2 - Forward Query**
- Box: "Forward to Upstream DNS"
- Sub-boxes: "8.8.8.8 (Google)" and "1.1.1.1 (Cloudflare)"
- Arrow: "DNS Query"

**Step 3 - Receive Response**
- Box: "Receive DNS Response"
- Box: "Resolved IP: 142.250.185.46"

**Step 4 - Cache Management**
- Diamond: "Cache Full?"
- Yes → Box: "Evict LRU Entry"
- No → Continue
- Box: "Add to Cache"
  * Domain: www.google.com
  * IP: 142.250.185.46
  * TTL: 300s
  * Timestamp: now

**Step 5 - Return**
- Box: "Return DNS Response to Client"
- Arrow up to Client Device

**Side Panel - Cache Statistics**
- Box showing:
  * Cache Size: 50 entries
  * Hit Rate: 78.3%
  * Hit Latency: 0.8ms
  * Miss Latency: 87.4ms

### Prompt for AI Tools
"Create a flowchart for DNS caching system. Top: Client sends DNS query. Main flow: Decision 'Domain in cache?', if yes check 'TTL valid?', if yes return cached IP with 'Update LRU'. If no or expired, 'Forward to upstream DNS' (8.8.8.8, 1.1.1.1), receive response, check 'Cache full?', if yes 'Evict LRU entry', then 'Add to cache', return response. Side panel shows cache statistics. Use flowchart symbols, boxes for processes, diamonds for decisions. Black and white, clear labels."

---

## FIGURE 8: Memory Utilization Chart

### Description
A stacked bar chart comparing memory usage between Client Mode and Router Mode.

### Detailed Layout
**Chart Type:** Horizontal stacked bar chart

**Y-Axis:** Two bars
1. "Client Mode" (top bar)
2. "Router Mode" (bottom bar)

**X-Axis:** Memory in KB (0 to 120 KB)

**Client Mode Bar (76.2 KB total):**
- Segment 1: Logger (10.2 KB) - Light gray
- Segment 2: HTTP Client (18.5 KB) - Medium gray
- Segment 3: Portal Auth (14.3 KB) - Dark gray
- Segment 4: WiFi Manager (4.8 KB) - Light gray
- Segment 5: Other (28.4 KB) - Medium gray

**Router Mode Bar (102.5 KB total):**
- Segment 1: All Client Components (76.2 KB) - Striped pattern
- Segment 2: AP Manager (5.1 KB) - Light gray
- Segment 3: NAT Router (9.7 KB) - Medium gray
- Segment 4: DNS Forwarder (7.8 KB) - Dark gray
- Segment 5: MAC Manager (2.1 KB) - Light gray
- Segment 6: Overhead (1.6 KB) - Medium gray

**Labels:**
- Each segment labeled with component name and size
- Total at end of each bar
- Title: "Memory Utilization: Client vs Router Mode"
- Subtitle: "ESP32 Total RAM: 520 KB"

### Prompt for AI Tools
"Create a horizontal stacked bar chart showing memory usage. Two bars: 'Client Mode' (76.2 KB) and 'Router Mode' (102.5 KB). Client Mode bar has 5 segments: Logger 10.2KB, HTTP Client 18.5KB, Portal Auth 14.3KB, WiFi Manager 4.8KB, Other 28.4KB. Router Mode bar has 6 segments: Client Components 76.2KB (striped), AP Manager 5.1KB, NAT Router 9.7KB, DNS Forwarder 7.8KB, MAC Manager 2.1KB, Overhead 1.6KB. Each segment labeled. X-axis shows KB scale 0-120. Title 'Memory Utilization'. Professional chart style, black and white with patterns."

---

## FIGURE 9: Performance Comparison Graph

### Description
A multi-line graph showing throughput vs number of clients.

### Detailed Layout
**Chart Type:** Line graph with data points

**X-Axis:** Number of Clients (1, 2, 3, 4)

**Y-Axis (Left):** Throughput (Mbps) - Scale 0 to 10

**Y-Axis (Right):** Latency (ms) - Scale 0 to 60

**Line 1 - Throughput (solid line with circles):**
- Point 1: (1 client, 9.2 Mbps)
- Point 2: (2 clients, 8.1 Mbps)
- Point 3: (3 clients, 6.8 Mbps)
- Point 4: (4 clients, 5.4 Mbps)
- Downward trend

**Line 2 - Latency (dashed line with squares):**
- Point 1: (1 client, 28 ms)
- Point 2: (2 clients, 35 ms)
- Point 3: (3 clients, 42 ms)
- Point 4: (4 clients, 51 ms)
- Upward trend

**Line 3 - Packet Loss (dotted line with triangles):**
- Point 1: (1 client, 0.2%)
- Point 2: (2 clients, 0.4%)
- Point 3: (3 clients, 0.7%)
- Point 4: (4 clients, 1.2%)
- Slight upward trend

**Labels:**
- Title: "Router Mode Performance vs Client Count"
- Legend showing three lines
- Grid lines for readability
- Data point values labeled

### Prompt for AI Tools
"Create a line graph with X-axis 'Number of Clients' (1-4) and dual Y-axes. Left Y-axis 'Throughput (Mbps)' 0-10, right Y-axis 'Latency (ms)' 0-60. Three lines: Solid line with circles showing throughput declining from 9.2 to 5.4 Mbps. Dashed line with squares showing latency increasing from 28 to 51 ms. Dotted line with triangles showing packet loss 0.2% to 1.2%. Each data point labeled with value. Grid lines, legend, title 'Router Mode Performance vs Client Count'. Professional graph style, black and white."

---

## FIGURE 10: Hardware Component Diagram

### Description
A detailed hardware schematic showing ESP32 connections and components.

### Detailed Layout
**Center - ESP32-WROOM-32 Module (Large Rectangle)**
- Pin labels on sides
- Internal blocks:
  * "Dual-Core CPU (240 MHz)"
  * "520 KB SRAM"
  * "4 MB Flash"
  * "WiFi Radio (2.4 GHz)"

**Top Connections:**
- USB-to-Serial Converter
  * TX, RX, GND, 3.3V pins
  * Connected to ESP32 UART pins

**Left Side:**
- Power Supply Circuit
  * 5V Input
  * AMS1117-3.3V Regulator
  * Capacitors (10µF, 100nF)
  * 3.3V Output to ESP32

**Right Side:**
- WiFi Antenna
  * PCB trace antenna or external antenna
  * Connected to ESP32 RF pin

**Bottom:**
- Programming Interface
  * BOOT button (GPIO0 to GND)
  * RESET button (EN to GND)
  * Pull-up resistors

**External Components:**
- LED indicator (GPIO2)
  * 330Ω resistor
  * LED symbol
- Crystal oscillator (if external)
  * 40 MHz
  * Load capacitors

**Labels:**
- Component values
- Pin numbers
- Voltage levels
- Signal names

### Prompt for AI Tools
"Create a hardware schematic diagram. Center: Large ESP32-WROOM-32 module rectangle with internal blocks labeled 'Dual-Core CPU 240MHz', '520KB SRAM', '4MB Flash', 'WiFi Radio 2.4GHz'. Top: USB-to-Serial converter connected to UART pins. Left: Power supply circuit with 5V input, AMS1117-3.3V regulator, capacitors, 3.3V output. Right: WiFi antenna symbol connected to RF pin. Bottom: BOOT and RESET buttons with pull-up resistors. External: LED on GPIO2 with 330Ω resistor. All components labeled with values and pin numbers. Professional electronic schematic style, black and white."

---

## SUMMARY TABLE FOR ALL FIGURES

| Figure | Type | Tool Recommendation | Complexity |
|--------|------|---------------------|------------|
| 1 | Block Diagram | Napkin AI, Lucidchart | Medium |
| 2 | State Machine | Napkin AI, Draw.io | Medium |
| 3 | Flowchart | Napkin AI, Draw.io | High |
| 4 | Flowchart | Napkin AI, Draw.io | High |
| 5 | Network Topology | Napkin AI, Lucidchart | Medium |
| 6 | Process Diagram | Napkin AI, Draw.io | High |
| 7 | Flowchart | Napkin AI, Draw.io | High |
| 8 | Bar Chart | Excel, Google Sheets, Napkin AI | Low |
| 9 | Line Graph | Excel, Google Sheets, Napkin AI | Low |
| 10 | Schematic | Fritzing, KiCad, Draw.io | High |

## RECOMMENDED TOOLS

### For Napkin AI (napkin.ai)
- Best for: Figures 1, 2, 3, 4, 5, 6, 7
- Input: Use the detailed text descriptions above
- Style: Select "Technical Diagram" or "Flowchart" style
- Export: PNG or SVG at high resolution (300 DPI for patents)

### For DALL-E / ChatGPT
- Best for: Conceptual diagrams (Figures 1, 2, 5)
- Input: Use the "Prompt for AI Tools" sections
- Style: Add "professional patent diagram style, black and white, clear labels"
- Note: May need manual cleanup in drawing tool

### For Traditional Tools
- **Draw.io / Diagrams.net** (Free): All flowcharts and diagrams
- **Lucidchart**: Professional network and block diagrams
- **Microsoft Visio**: All diagram types
- **Fritzing**: Hardware schematic (Figure 10)
- **Excel/Google Sheets**: Charts (Figures 8, 9)

## PATENT OFFICE REQUIREMENTS

### Image Specifications
- **Format**: PNG, TIFF, or PDF
- **Resolution**: 300 DPI minimum
- **Size**: Black and white or grayscale
- **Dimensions**: Fit within 170mm × 254mm (6.69" × 10")
- **Line Weight**: Minimum 0.3mm thick
- **Text**: Minimum 0.32cm (0.125") height
- **Margins**: 2.5cm top, 2.5cm left, 1.5cm right, 1cm bottom

### Labeling Requirements
- Use reference numerals (10, 20, 30, etc.)
- Each element must be labeled
- Labels must match description in patent text
- Use lead lines (arrows) to point to components

---

**END OF FIGURE DESCRIPTIONS**

**Total Figures**: 10
**Estimated Creation Time**: 4-6 hours for all figures
**Recommended Approach**: Use Napkin AI for quick generation, then refine in Draw.io
