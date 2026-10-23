#include "../include/wifi_service.h"

// ==============================================================================
// VDT Platform - Wi-Fi AP & WebSocket Gerceklesimi (Hafta 3)
// ==============================================================================

WifiService* WifiService::_instance = nullptr;

// --- Dahili Mobil Dokunmatik Web Dashboard (HTML5 + CSS + JS) ---
static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <title>VDT Platform — Mobil Kokpit</title>
  <style>
    :root {
      --bg: #0f1117;
      --card: #181b24;
      --accent: #00d26a;
      --danger: #f8312f;
      --text: #e6edf3;
      --text-dim: #8b949e;
      --border: #30363d;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      touch-action: none;
    }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, sans-serif;
      height: 100vh;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      padding: 12px;
    }
    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding-bottom: 10px;
      border-bottom: 1px solid var(--border);
    }
    h1 {
      font-size: 1.1rem;
      font-weight: 700;
      letter-spacing: 0.5px;
    }
    .badge {
      font-size: 0.75rem;
      padding: 4px 10px;
      border-radius: 12px;
      font-weight: 600;
      background: #331515;
      color: var(--danger);
      transition: all 0.3s;
    }
    .badge.connected {
      background: #0d2818;
      color: var(--accent);
    }
    .metrics {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin: 10px 0;
    }
    .metric-card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 8px 12px;
      text-align: center;
    }
    .metric-title {
      font-size: 0.7rem;
      color: var(--text-dim);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    .metric-val {
      font-size: 1.4rem;
      font-weight: 800;
      margin-top: 2px;
    }
    .controls {
      flex: 1;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      min-height: 0;
    }
    .stick-zone {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      position: relative;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .stick-label {
      position: absolute;
      top: 10px;
      font-size: 0.75rem;
      font-weight: 600;
      color: var(--text-dim);
      text-transform: uppercase;
    }
    .stick-sub {
      position: absolute;
      bottom: 10px;
      font-size: 0.65rem;
      color: var(--text-dim);
    }
    .puck {
      width: 80px;
      height: 80px;
      border-radius: 50%;
      background: radial-gradient(circle, #2d3342 0%, #1e222d 100%);
      border: 2px solid var(--border);
      box-shadow: 0 4px 15px rgba(0,0,0,0.5);
      position: absolute;
      transition: transform 0.05s ease-out;
      pointer-events: none;
    }
    .puck::after {
      content: '';
      position: absolute;
      width: 14px;
      height: 14px;
      background: var(--accent);
      border-radius: 50%;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      box-shadow: 0 0 10px var(--accent);
    }
    .btn-emergency {
      background: var(--danger);
      color: #fff;
      font-weight: 800;
      font-size: 1rem;
      padding: 14px;
      border: none;
      border-radius: 12px;
      margin-top: 10px;
      text-transform: uppercase;
      letter-spacing: 1px;
      box-shadow: 0 4px 15px rgba(248, 49, 47, 0.4);
      cursor: pointer;
    }
    .btn-emergency:active {
      transform: scale(0.98);
      filter: brightness(0.9);
    }
  </style>
</head>
<body>
  <header>
    <h1>🚗 VDT Platform</h1>
    <div id="statusBadge" class="badge">KOPTU</div>
  </header>

  <div class="metrics">
    <div class="metric-card">
      <div class="metric-title">Gaz (Throttle)</div>
      <div id="throttleDisplay" class="metric-val">0%</div>
    </div>
    <div class="metric-card">
      <div class="metric-title">Direksiyon (Steering)</div>
      <div id="steeringDisplay" class="metric-val">0°</div>
    </div>
  </div>

  <div class="controls">
    <!-- Sol: Gaz (Dikey Joystick) -->
    <div id="throttleZone" class="stick-zone">
      <div class="stick-label">İleri / Geri</div>
      <div id="throttlePuck" class="puck"></div>
      <div class="stick-sub">İleri (+100) / Geri (-100)</div>
    </div>

    <!-- Sag: Direksiyon (Yatay Joystick) -->
    <div id="steeringZone" class="stick-zone">
      <div class="stick-label">Direksiyon</div>
      <div id="steeringPuck" class="puck"></div>
      <div class="stick-sub">Sol (-29°) / Sağ (+29°)</div>
    </div>
  </div>

  <button id="btnEmergency" class="btn-emergency">🚨 ACİL DURDURMA</button>

  <script>
    let ws = null;
    let throttleVal = 0;
    let steeringVal = 0.0;
    const badge = document.getElementById('statusBadge');
    const throttleDisp = document.getElementById('throttleDisplay');
    const steeringDisp = document.getElementById('steeringDisplay');
    const throttlePuck = document.getElementById('throttlePuck');
    const steeringPuck = document.getElementById('steeringPuck');
    const throttleZone = document.getElementById('throttleZone');
    const steeringZone = document.getElementById('steeringZone');

    function connectWS() {
      const wsUrl = 'ws://' + window.location.hostname + ':81/';
      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        badge.textContent = 'BAĞLI';
        badge.className = 'badge connected';
      };

      ws.onclose = () => {
        badge.textContent = 'KOPTU';
        badge.className = 'badge';
        setTimeout(connectWS, 1000);
      };

      ws.onerror = (err) => {
        ws.close();
      };
    }

    function sendCommand() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        const payload = JSON.stringify({
          throttle: Math.round(throttleVal),
          steering: parseFloat(steeringVal.toFixed(1))
        });
        ws.send(payload);
      }
    }

    // Periyodik Heartbeat & Komut gonderimi (25 Hz - her 40ms)
    setInterval(sendCommand, 40);

    // --- Sol Joystick: Gaz Mantigi (Dikey) ---
    let throttleActive = false;
    let throttleTouchId = null;

    function handleThrottleTouch(e) {
      const rect = throttleZone.getBoundingClientRect();
      const centerY = rect.top + rect.height / 2;
      const touch = Array.from(e.touches).find(t => t.identifier === throttleTouchId) || e.touches[0];
      if (!touch) return;

      const dy = touch.clientY - centerY;
      const maxDy = rect.height / 2 - 40;
      const clampedDy = Math.max(-maxDy, Math.min(maxDy, dy));

      // Ekran koordinatinda yukari negatif oldugu icin ters ceviriyoruz
      throttleVal = -Math.round((clampedDy / maxDy) * 100);
      throttlePuck.style.transform = `translateY(${clampedDy}px)`;
      throttleDisp.textContent = (throttleVal > 0 ? '+' : '') + throttleVal + '%';
    }

    throttleZone.addEventListener('touchstart', (e) => {
      throttleActive = true;
      throttleTouchId = e.changedTouches[0].identifier;
      handleThrottleTouch(e);
    });

    throttleZone.addEventListener('touchmove', (e) => {
      if (throttleActive) handleThrottleTouch(e);
    });

    function resetThrottle() {
      throttleActive = false;
      throttleTouchId = null;
      throttleVal = 0;
      throttlePuck.style.transform = `translateY(0px)`;
      throttleDisp.textContent = '0%';
      sendCommand();
    }
    throttleZone.addEventListener('touchend', resetThrottle);
    throttleZone.addEventListener('touchcancel', resetThrottle);

    // --- Sag Joystick: Direksiyon Mantigi (Yatay) ---
    let steeringActive = false;
    let steeringTouchId = null;

    function handleSteeringTouch(e) {
      const rect = steeringZone.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const touch = Array.from(e.touches).find(t => t.identifier === steeringTouchId) || e.touches[0];
      if (!touch) return;

      const dx = touch.clientX - centerX;
      const maxDx = rect.width / 2 - 40;
      const clampedDx = Math.max(-maxDx, Math.min(maxDx, dx));

      steeringVal = parseFloat(((clampedDx / maxDx) * 29.0).toFixed(1));
      steeringPuck.style.transform = `translateX(${clampedDx}px)`;
      steeringDisp.textContent = (steeringVal > 0 ? '+' : '') + steeringVal + '°';
    }

    steeringZone.addEventListener('touchstart', (e) => {
      steeringActive = true;
      steeringTouchId = e.changedTouches[0].identifier;
      handleSteeringTouch(e);
    });

    steeringZone.addEventListener('touchmove', (e) => {
      if (steeringActive) handleSteeringTouch(e);
    });

    function resetSteering() {
      steeringActive = false;
      steeringTouchId = null;
      steeringVal = 0.0;
      steeringPuck.style.transform = `translateX(0px)`;
      steeringDisp.textContent = '0°';
      sendCommand();
    }
    steeringZone.addEventListener('touchend', resetSteering);
    steeringZone.addEventListener('touchcancel', resetSteering);

    // --- Acil Durdurma Butonu ---
    document.getElementById('btnEmergency').addEventListener('click', () => {
      resetThrottle();
      resetSteering();
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ emergency: true }));
      }
    });

    connectWS();
  </script>
</body>
</html>
)rawliteral";

// --- WifiService Metodlari ---

WifiService::WifiService()
    : _commandQueue(nullptr),
      _server(80),
      _webSocket(81),
      _connected(false),
      _lastHeartbeatMs(0),
      _activeCommand{0, 0.0f} {
    _instance = this;
}

void WifiService::webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (_instance) {
        _instance->handleWebSocketEvent(num, type, payload, length);
    }
}

void WifiService::begin(QueueHandle_t commandQueue, const char* ssid, const char* password) {
    _commandQueue = commandQueue;
    Serial.printf("\n[Wi-Fi] Access Point olusturuluyor: '%s'...\n", ssid);

    // 1. Wi-Fi Erisim Noktasini (AP) baslat
    WiFi.mode(WIFI_AP);
    if (password && strlen(password) >= 8) {
        WiFi.softAP(ssid, password);
    } else {
        WiFi.softAP(ssid); // Parolasiz acik ag (hizli baglanti)
    }

    IPAddress IP = WiFi.softAPIP();
    Serial.printf("[Wi-Fi] AP Hazir! IP Adresi: %s\n", IP.toString().c_str());

    // 2. HTTP Web Sunucu Yonlendirmeleri
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.onNotFound([this]() { handleNotFound(); });
    _server.begin();
    Serial.println("[HTTP] Port 80 Web Sunucusu baslatildi.");

    // 3. WebSocket Sunucusunu baslat
    _webSocket.begin();
    _webSocket.onEvent(webSocketEventWrapper);
    Serial.println("[WebSocket] Port 81 WebSocket Sunucusu baslatildi.");
    Serial.println("[Wi-Fi] Telefonunuzla 'VDT-Car' agina baglanip Safari'den '192.168.4.1' adresini acin!\n");
}

void WifiService::loop() {
    _server.handleClient();
    _webSocket.loop();
}

bool WifiService::isConnected() const {
    return _connected;
}

bool WifiService::isHeartbeatTimedOut() const {
    if (!_connected) return false;
    return (millis() - _lastHeartbeatMs) > PROTOCOL_HEARTBEAT_TIMEOUT_MS;
}

void WifiService::refreshHeartbeat() {
    _lastHeartbeatMs = millis();
}

void WifiService::handleRoot() {
    _server.send_P(200, "text/html", DASHBOARD_HTML);
}

void WifiService::handleNotFound() {
    _server.send(404, "text/plain", "404: Not Found");
}

void WifiService::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            _connected = true;
            _lastHeartbeatMs = millis();
            IPAddress ip = _webSocket.remoteIP(num);
            Serial.printf("[WebSocket] [#%u] Istemci baglandi: %s\n", num, ip.toString().c_str());
            break;
        }

        case WStype_DISCONNECTED: {
            _connected = false;
            Serial.printf("[WebSocket] [#%u] Istemci ayrildi! Acil durdurma tetikleniyor.\n", num);

            // Guvenlik durdurmasi
            _activeCommand.throttle = 0;
            _activeCommand.steering = 0.0f;
            if (_commandQueue != nullptr) {
                xQueueOverwrite(_commandQueue, &_activeCommand);
            }
            break;
        }

        case WStype_TEXT: {
            refreshHeartbeat();

            // Gelen JSON paketini ayristir
            StaticJsonDocument<128> doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            if (error) {
                return;
            }

            // Acil Durdurma kontrolu
            if (doc["emergency"] == true) {
                _activeCommand.throttle = 0;
                _activeCommand.steering = 0.0f;
                if (_commandQueue != nullptr) {
                    xQueueOverwrite(_commandQueue, &_activeCommand);
                }
                Serial.println("[WebSocket] Acil Durdurma tetiklendi!");
                return;
            }

            // Gaz ve direksiyon degerlerini oku
            if (doc.containsKey("throttle")) {
                _activeCommand.throttle = doc["throttle"].as<int8_t>();
            }
            if (doc.containsKey("steering")) {
                _activeCommand.steering = doc["steering"].as<float>();
            }

            _activeCommand.validate();

            // Kargo bandina (Queue) en yeni komutu birak
            if (_commandQueue != nullptr) {
                xQueueOverwrite(_commandQueue, &_activeCommand);
            }
            break;
        }

        default:
            break;
    }
}
