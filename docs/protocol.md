# 📡 VDT Platform — Wi-Fi WebSocket İletişim Protokolü

## 1. Genel Bakış
VDT Platform, araç kontrolü ve telemetri aktarımı için **bağımsız Wi-Fi Access Point (AP)** ve **tam çift yönlü (full-duplex) WebSocket** mimarisini kullanır.

* **Wi-Fi SSID:** `VDT-Car` (Açık / Parolasız)
* **ESP32 IP Adresi:** `192.168.4.1`
* **HTTP Web Sunucu Portu:** `80` (`http://192.168.4.1`)
* **WebSocket Portu:** `81` (`ws://192.168.4.1:81/`)

---

## 2. Ağ ve Servis Mimarisi

```
[ Telefon / Tarayıcı (Safari/Chrome) ]
   │
   ├─► HTTP GET http://192.168.4.1:80  ──► Mobil Dashboard (HTML5/CSS3/JS)
   │
   └─► WS CONNECT ws://192.168.4.1:81  ──► Canlı Çift Yönlü İletişim (25 Hz)
```

1. **HTTP Port 80:** ESP32'nin flash belleğinde gömülü bulunan (`PROGMEM`) dokunmatik sanal joystick sayfasını istemciye sunar. Sıfır harici internet ve sıfır CDN bağımlılığı vardır.
2. **WebSocket Port 81:** Düşük gecikmeli (< 20 ms), canlı komut akışı ve telemetri kanalıdır.

---

## 3. Komut Mesaj Formatı (İstemci ➔ ESP32)

İstemci (mobil web sayfası), WebSocket üzerinden JSON formatında mesajlar gönderir:

### 3.1 Normal Sürüş Komutu (Drive)
Saniyede 25 kez (her 40 ms'de bir) periyodik olarak veya joystick hareket ettikçe gönderilir:
```json
{
  "throttle": 45,
  "steering": 12.5
}
```
* `throttle`: Tamsayı (`int8_t`), `-100` (tam geri) ile `+100` (tam ileri) arası.
* `steering`: Kayan noktalı sayı (`float`), `-29.0` (tam sol) ile `+29.0` (tam sağ) derece arası.

### 3.2 Acil Durdurma Komutu (Emergency Stop)
Kırmızı acil durdurma butonuna basıldığında gönderilir:
```json
{
  "emergency": true
}
```
* ESP32 motorları anında keser (`PWM 0`) ve direksiyon servosu merkeze (`0°`) döner.

---

## 4. Güvenlik ve Emniyet Kilidi (Fail-Safe / Watchdog)

Sistem 3 kademeli donanımsal ve yazılımsal güvenlik mekanizması içerir:

1. **WebSocket Kopma Olayı (`WStype_DISCONNECTED`):**
   * Telefonun Wi-Fi bağlantısı koptuğunda veya Safari sekmesi kapatıldığında ESP32 anında durur.
2. **Heartbeat Zaman Aşımı (`PROTOCOL_HEARTBEAT_TIMEOUT_MS = 500 ms`):**
   * Bağlantı açık görünse dahi, telefon 500 ms boyunca yeni paket iletmezse araç otomatik olarak motorları keser.
3. **Fiziksel Değer Sınırlaması (Clamping):**
   * Hatalı veya sınır dışı bir veri gelse bile `DriveCommand::validate()` fonksiyonu değerleri güvenli aralığa (`[-100, 100]`, `[-29, 29]`) kırpar.

---

## 5. Telemetri Yayını (ESP32 ➔ İstemci — Hafta 4 Altyapısı)

WebSocket kanalı çift yönlü olduğu için, Hafta 4'te MPU6050 sensöründen okunacak veriler aynı kanal üzerinden telefon ekranına geri gönderilecektir:
```json
{
  "accel_x": 0.12,
  "accel_y": -0.05,
  "gyro_z": 14.2,
  "battery_v": 7.8
}
```
