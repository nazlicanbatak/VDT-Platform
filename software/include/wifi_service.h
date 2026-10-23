#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "protocol.h"

// ==============================================================================
// VDT Platform - Wi-Fi AP & WebSocket Servis Sinifi (Hafta 3)
// Gorevi: ESP32'yi bagimsiz bir Wi-Fi erisim noktasi (Access Point) yapmak,
// web sitesini telefona sunmak ve WebSocket uzerinden surus komutlarini almak.
// ==============================================================================

class WifiService {
public:
    // Kurucu fonksiyon
    WifiService();

    // Wi-Fi AP, Web Server ve WebSocket sunucusunu baslatir
    void begin(QueueHandle_t commandQueue, const char* ssid = "VDT-Car", const char* password = nullptr);

    // Web ve WebSocket olaylarini isleyen dongu (Core 0 gorevinde cagrilir)
    void loop();

    // Baglanti durumunu dondurur
    bool isConnected() const;

    // Heartbeat zaman asimina ugradi mi kontrol eder (Fail-Safe)
    bool isHeartbeatTimedOut() const;

    // Son canlilik zamanini gunceller
    void refreshHeartbeat();

private:
    QueueHandle_t     _commandQueue;     // Komutlarin iletildigi kargo bandi
    WebServer         _server;           // Port 80 HTTP Web Sunucusu
    WebSocketsServer  _webSocket;        // Port 81 WebSocket Sunucusu
    bool              _connected;        // En az 1 istemci bagli mi?
    uint32_t          _lastHeartbeatMs;  // Son gecerli paket zamani (ms)
    DriveCommand      _activeCommand;    // Son gecerli gaz ve direksiyon durumu

    // WebSocket olay dinleyicisi
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    // HTTP sayfa yonlendiricileri
    void handleRoot();
    void handleNotFound();

    static void webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static WifiService* _instance;
};
