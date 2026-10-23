# 🚗 Otonom Araç Dinamikleri ve Telemetri Projesi — Yol Haritası

> **Ekip:** 1× Bilgisayar Mühendisliği + 1× Elektrik-Elektronik Mühendisliği (3. Sınıf)
> **Platform:** ESP32 + TB6612FNG + MPU6050 + Python WebSocket + Tarayıcı Dashboard
> **Süre:** 8 Haftalık Temel Sprint + İleri Aşamalar

---

## 📌 Proje Felsefesi

Bu proje, bir RC araba yapmaktan çok daha fazlasıdır. Her hafta eklenen bir katman, endüstriyel araçlarda gerçekten kullanılan bir mühendislik konseptinin sıfırdan implement edilmesidir. Hedef; diploma almak değil, **gerçek mühendis düşünmek**.

> *"The best way to learn engineering is to build something that can fail."*

---

## 🗺️ Genel Mimari Haritası

```
┌───────────────────────────────────────────────────────────────────┐
│         Telefon / PC Tarayıcı — Mobil Telemetri & Sürüş           │
│       HTML5 + CSS3 + Canvas Joystick (WebSocket Client)           │
│  ┌──────────────┐  ┌─────────────────┐  ┌──────────────────────┐ │
│  │ Joystick     │  │  G-Meter /      │  │  IMU Pitch/Roll      │ │
│  │ (Drive-by-   │  │  Traction Circle│  │  Gauge (Yapay Ufuk)  │ │
│  │  Wire)       │  │                 │  │                      │ │
│  └──────────────┘  └─────────────────┘  └──────────────────────┘ │
└────────────────────────────┬──────────────────────────────────────┘
                             │ Wi-Fi WebSocket (ws://192.168.4.1/ws)
                             │ HTTP Web Sunucu (http://192.168.4.1)
┌────────────────────────────▼──────────────────────────────────────┐
│           ESP32 — ECU & Wi-Fi Access Point ("VDT-Car")            │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────────────┐    │
│  │  FreeRTOS   │  │ Sensor Task  │  │  Motor Control Task   │    │
│  │  Scheduler  │  │ MPU6050/I2C  │  │  TB6612FNG/PWM        │    │
│  └─────────────┘  └──────────────┘  └───────────────────────┘    │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────────────┐    │
│  │ Wi-Fi + WS  │  │  Watchdog    │  │  E-Diff Algorithm     │    │
│  │ Web Server  │  │  Heartbeat   │  │  Torque Vectoring     │    │
│  └─────────────┘  └──────────────┘  └───────────────────────┘    │
└───────────────────────────────────────────────────────────────────┘
        │                     │                     │
   [TB6612FNG]           [MPU6050]              [MG995 Servo]
   Motor L/R             Accel+Gyro            Ön Direksiyon
        │
  [DC Motor x2]
```
*(Opsiyonel: Bilgisayar da aynı Wi-Fi ağına bağlanarak telemetriyi Python ile CSV'ye kaydedebilir).*

---

## 📅 Haftalık Sprint Planı

---

### 🔧 HAFTA 1 — Temel Altyapı ve Güç Devresinin Kurulumu

**Hedef:** Aracı güvenli biçimde besleyen güç hattını kurmak ve ESP32'yi ilk kez çalıştırmak.

#### Yapılacaklar

**EE Görevi:**
- [ ] 2S 18650 pil paketi + BMS bağlantısı (aşırı deşarj/şarj koruması doğrulanacak)
- [ ] LM2596 ile 7.4V → 5V regülasyon devresi kurulumu (multimetre ile ölçüm)
- [ ] TB6612FNG motor sürücü breadboard bağlantısı (datasheetini oku!)
- [ ] Güç hattına sigorta / polisiklik sigorta eklenmesi (Functional Safety ilkesi)
- [ ] Tüm toprak (GND) hatlarının ortak noktada birleştirilmesi (common ground)

**CS Görevi:**
- [ ] PlatformIO kurulumu ve ilk ESP32 "Blink" testi
- [ ] TB6612FNG için temel PWM motor sürüş kodu yazımı (ileriye/geriye/dur)
- [ ] Serial Monitor ile PWM değerinin (0–255) debug edilmesi

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| Lityum İyon pil kimyası ve BMS çalışma prensibi | EE |
| H-Köprüsü (H-Bridge) topolojisi ve yön kontrolü | EE |
| LDO vs Switching Regülatör (LM2596 neden verimli?) | EE |
| PWM (Pulse Width Modulation) ile hız kontrolü | CS/EE |
| PlatformIO proje yapısı ve platformio.ini | CS |
| Datasheet okuma: TB6612FNG pin açıklamaları | EE |

#### Dikkat Edilecek Noktalar
- BMS bypass etme — kısa devre riski yüksek.
- TB6612FNG STBY pini HIGH yapılmadan motorlar çalışmaz.
- LM2596 çıkışına mutlaka kondansatör koy (output ripple).

---

### 🏗️ HAFTA 2 — Mekanik Montaj ve Servo Entegrasyonu

**Hedef:** Şasiyi monte etmek, Ackermann direksiyon geometrisini fiziksel olarak uygulamak ve servo kontrol kodunu yazmak.

#### Yapılacaklar

**EE + Mekanik:**
- [ ] 3D baskı parçaları: Ön aks, rot kolu, motor tutucular (Fusion 360 / Onshape)
- [ ] MG995 servo motorun direksiyon sistemine mekanik bağlantısı
- [ ] Ackermann geometrisine göre rot kolu açısının hesaplanması ve ayarlanması
- [ ] TPU lastikler ve PLA jantların montajı

**CS Görevi:**
- [ ] ESP32 ile MG995 servo PWM kontrolü (Arduino Servo.h veya ledc API)
- [ ] Servo açısının (-30° ↔ +30°) PWM sinyal genişliğiyle eşleştirilmesi
- [ ] Motor + Servo kombine kontrol testi (ileri giderken sola dön)

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| Ackermann Direksiyon Geometrisi (iç/dış tekerlek açı farkı) | Mekanik |
| Servo PWM protokolü (50Hz, 1ms–2ms pulse) | EE/CS |
| 3D baskı malzeme seçimi: PLA vs TPU mekanik özellikleri | Mekanik |
| Kinematik zincirleme: servo açısı → tekerlek açısı | Mekanik |
| ESP32 ledc (LED Control) PWM API kullanımı | CS |

#### Ackermann Formülü

```
cot(delta_dis) - cot(delta_ic) = T / L

Burada:
  delta_dis = Dış tekerlek dönüş açısı
  delta_ic  = İç tekerlek dönüş açısı
  T         = Dingil iz genişliği (mm)
  L         = Dingil arası mesafe (aks mesafesi, mm)
```

> Bu formülü ölçüp koda gömeceksiniz — saf mekanik, saf mühendislik.

---

### 📡 HAFTA 3 — Wi-Fi Drive-by-Wire Kontrolü ve Bağımsız Mobil Dashboard

**Hedef:** ESP32'yi bağımsız bir Wi-Fi Erişim Noktası (Access Point: `VDT-Car`) ve Web Sunucu haline getirmek; herhangi bir bilgisayara ihtiyaç duymadan doğrudan telefon Safari/Chrome tarayıcısından WebSocket ile düşük gecikmeli Drive-by-Wire sürüş sağlamak.

#### Yapılacaklar

**CS — ESP32 Tarafı:**
- [ ] ESP32 Wi-Fi Access Point (`VDT-Car`, 192.168.4.1) kurulumu
- [ ] HTTP Web Sunucusu: Mobil uyumlu dashboard (HTML/CSS/JS) dosyalarını doğrudan tarayıcıya sunma
- [ ] WebSocket Sunucusu: Canlı iki yönlü komut ve telemetri kanalı (`ws://192.168.4.1/ws`)
- [ ] FreeRTOS Mimarisi: Wi-Fi/WebSocket görevi (Core 0) ve Motor Kontrol görevi (Core 1) ayrımı (`commandQueue` ile)
- [ ] Fail-Safe / Watchdog: WebSocket bağlantısı koptuğunda veya 500ms heartbeat gelmediğinde acil durdurma

**CS — Mobil Tarayıcı Dashboard (HTML/CSS/JS):**
- [ ] Responsive, dokunmatik sanal joystick (Sol el: Gaz/Fren, Sağ el: Direksiyon)
- [ ] WebSocket üzerinden gerçek zamanlı komut iletimi (20–30 Hz)
- [ ] Acil Durdurma butonu ve bağlantı durumu göstergesi (yeşil/kırmızı)
- [ ] İlerisi için telemetriye hazır arayüz (Hafta 4 sensör göstergeleri)

**EE Görevi:**
- [ ] Wi-Fi açıkken 5V güç hattının akım/voltaj kararlılığı testi (brown-out kontrolü)
- [ ] Açık alanda ve engelli ortamda Wi-Fi çekim mesafesi testi

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| Wi-Fi Access Point (AP) ve DHCP sunucu mantığı | CS/EE |
| Gömülü HTTP Web Sunucu & WebSocket protokolü | CS |
| FreeRTOS Görevleri ve Queue ile çekirdekler arası veri iletimi | CS |
| Mobil dokunmatik olayları (Touch Events) ve Canvas API | CS |
| Dağıtık sistemlerde Heartbeat & Fail-Safe güvenlik mimarisi | CS/Otomotiv |
| Drive-by-Wire konsepti ve modern araçlardaki kullanımı | Otomasyon |

#### FreeRTOS Görev Mimarisi

```
Core 0:                               Core 1:
┌──────────────────────────┐          ┌─────────────────────┐
│  Wi-Fi + WebSocket Task  │          │  Motor Control Task  │
│  (Web Server & Network)  │──Queue──▶│  (VehicleController) │
│  Priority: 2, Stack: 4096│          │  Priority: 3        │
└──────────────────────────┘          └─────────────────────┘
                                      ┌─────────────────────┐
                                      │   Watchdog Check    │
                                      │  (500ms timeout)    │
                                      └─────────────────────┘
```

---

### 🛡️ HAFTA 3 EKİ — Fail-Safe ve Watchdog Sistemi

> **Not:** Bu konu 3. haftanın zorunlu bir parçasıdır, atlanmamalıdır.

**Hedef:** BLE bağlantısı koptuğunda aracı güvenli biçimde durdurma.

#### Watchdog Algoritması

```c
// Heartbeat timeout: 500ms içinde sinyal gelmezse dur
#define HEARTBEAT_TIMEOUT_MS 500

void watchdogTask(void *pvParams) {
    TickType_t lastHeartbeat = xTaskGetTickCount();
    
    for (;;) {
        if ((xTaskGetTickCount() - lastHeartbeat) > pdMS_TO_TICKS(HEARTBEAT_TIMEOUT_MS)) {
            // FAIL-SAFE: Tüm motorları durdur
            motorStop(MOTOR_LEFT);
            motorStop(MOTOR_RIGHT);
            servoCenter(); // Direksiyonu düzelt
            ESP_LOGW("WATCHDOG", "BLE timeout! Emergency stop.");
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

#### Bu Ekte Öğrenilen Konular

| Konu | Alan |
|------|------|
| ISO 26262 Functional Safety kavramı | Otomotiv |
| Hardware Watchdog Timer (ESP32 WDT) | EE/CS |
| Fail-safe vs Fail-secure farkı | Sistem Tasarımı |
| Heartbeat pattern (dağıtık sistemlerde) | CS |

---

### 📊 HAFTA 4 — MPU6050 Entegrasyonu ve Sensör Okuma

**Hedef:** I2C üzerinden MPU6050'yi okumak ve ham veriye anlam kazandırmak.

#### Yapılacaklar

**EE Görevi:**
- [ ] MPU6050 I2C bağlantısı (SDA/SCL pull-up dirençleri: 4.7kΩ)
- [ ] I2C scanner ile adres doğrulaması (0x68 veya 0x69)
- [ ] MPU6050 register haritasını okuma (ACCEL_XOUT_H vb.)

**CS Görevi:**
- [ ] Wire.h ile ham ivme ve açısal hız verisi okuma
- [ ] Ham veriyi SI birimlerine dönüştürme (m/s², °/s, G)
- [ ] Serial Plotter ile veri görselleştirme
- [ ] BLE üzerinden iOS'a telemetri verisi iletimi

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| I2C protokolü: SDA, SCL, adres, acknowledge | EE/CS |
| İvmeölçer (Accelerometer) çalışma prensibi (MEMS) | EE |
| Jiroskop (Gyroscope) çalışma prensibi ve drift | EE |
| Koordinat sistemi ve vektör yönleri (6 eksen) | Fizik |
| Ham ADC verisini fiziksel büyüklüğe dönüştürme | CS |
| G-Kuvveti ve motorsportlardaki önemi | Otomotiv |

#### Birim Dönüşümü

```c
// ±2g hassasiyeti için ölçek faktörü: 16384 LSB/g
float accel_x_g = (float)raw_accel_x / 16384.0f;

// ±250°/s hassasiyeti için ölçek faktörü: 131 LSB/(°/s)
float gyro_z_dps = (float)raw_gyro_z / 131.0f;
```

---

### 🔮 HAFTA 5 — Sensör Füzyonu ve Filtre Algoritmaları

**Hedef:** Ham sensör verisindeki gürültüyü temizleyerek güvenilir durum tahmini yapmak.

#### Yapılacaklar

**CS Görevi:**
- [ ] Complementary Filter implementasyonu (Pitch ve Roll için)
- [ ] Kalman Filter implementasyonu (tercihli: MPU6050 DMP kullanımı)
- [ ] Filtrelenmiş vs ham veri karşılaştırması (Serial Plotter)
- [ ] Dashboard'a canlı Pitch/Roll gauge bileşeni eklenmesi (Chart.js + requestAnimationFrame)
- [ ] Traction Circle görselleştirmesi (Scatter chart: Lateral G vs Longitudinal G)

**EE Görevi:**
- [ ] MPU6050 yerleştirme pozisyonu optimizasyonu (titreşimden uzak, COG yakını)
- [ ] Anti-vibration mount tasarımı (3D baskı TPU pad)

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| Sensor noise ve bias (sistematik hata) kavramları | EE |
| Complementary Filter matematiği | CS/Matematik |
| Kalman Filter durum-uzay modeli (temel) | CS/Matematik |
| Traction Circle (Kammscher Kreis) ve sürüş limiti | Otomotiv |
| Pitch, Roll, Yaw tanımları (Euler açıları) | Fizik |
| Dead Reckoning (sensör füzyonuyla konum tahmini) | Navigasyon |

#### Complementary Filter

```c
// dt: örnekleme periyodu (saniye)
// alpha: 0.98 (jiroskopa güven ağırlığı)
#define ALPHA 0.98f

float pitch = ALPHA * (pitch + gyro_x * dt) + (1.0f - ALPHA) * accel_pitch;
float roll  = ALPHA * (roll  + gyro_y * dt) + (1.0f - ALPHA) * accel_roll;
```

#### Tarayıcı Telemetri Dashboard Bileşenleri

- **G-Meter / Traction Circle:** Scatter chart (Lateral G - Longitudinal G), Chart.js ile
- **Yapay Ufuk (Artificial Horizon):** Canvas API ile çizilen Pitch/Roll göstergesi
- **Hız Göstergesi:** Tahmini hız (ivme integrasyonu), SVG needle gauge
- **Motor Güç Barları:** Sol/Sağ PWM yüzdesi — CSS animasyonlu bar chart
- **Bağlantı Durumu:** WebSocket + BLE RSSI göstergesi (yeşil/sarı/kırmızı)
- **Log Akışı:** Gelen son 20 telemetri paketi (scrolling JSON viewer)
- **CSV Kayıt Düğmesi:** Python backend tarafından dosyaya yazılır

#### Dashboard Teknoloji Kararı

| Katman | Teknoloji | Neden? |
|--------|-----------|--------|
| Görselleştirme | Chart.js + Canvas API | Hafif, bağımlılık yok, gerçek zamanlı update |
| İletişim | WebSocket (ws://) | Düşük gecikme, çift yönlü, HTTP'den üstün |
| Backend | Python asyncio + bleak | BLE → WS köprüsü, cross-platform |
| Kontrol | nipplejs joystick | Tarayıcıda dokunmatik/fare joystick |
| Kayıt | Python CSV logger | Pandas ile test sonrası analiz |

---

### ⚙️ HAFTA 6 — Elektronik Diferansiyel (Torque Vectoring)

**Hedef:** Yaw Rate bilgisini kullanarak sağ/sol motor güçlerini dinamik olarak dengelemek.

#### Yapılacaklar

**CS Görevi:**
- [ ] Ackermann modeline dayalı ideal tekerlek hızı hesaplama
- [ ] MPU6050 Yaw Rate'inden gerçek dönüş hızı okuma
- [ ] E-Diff algoritmasının C kodu olarak implementasyonu
- [ ] Test: Sabit direksiyon açısında düz vs E-Diff aktif karşılaştırması

**EE Görevi:**
- [ ] Yüksek PWM frekansında motor ısınma testi (termik analiz)

#### Elektronik Diferansiyel Algoritması

```c
// Ackermann'a göre ideal tekerlek hızları
// v: hedef hız, delta: ön tekerlek açısı, T: iz genişliği, L: aks mesafesi
float turningRadius = L / tanf(delta); // Anlık dönüş yarıçapı

float v_left  = v * (turningRadius - T / 2.0f) / turningRadius;
float v_right = v * (turningRadius + T / 2.0f) / turningRadius;

// PWM oranını normalize et (0-255)
uint8_t pwm_left  = (uint8_t)(v_left  / v_max * 255);
uint8_t pwm_right = (uint8_t)(v_right / v_max * 255);
```

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| Torque Vectoring prensibi (F1, EV'lerde kullanım) | Otomotiv |
| Elektronik Diferansiyel vs Mekanik Diferansiyel | Otomotiv |
| Kinematik model: Bicycle Model (tek izli model) | Kontrol |
| Slip Angle (Kayma Açısı) ve understeer/oversteer | Otomotiv |
| PID kontrolcü temelleri (hız sabitleme için) | Kontrol |
| Yaw Rate ve araç dinamiği ilişkisi | Dinamik |

---

### 📈 HAFTA 7 — ESC Algoritması ve Rejeneratif Frenleme

**Hedef:** Kayma açısı analizi ile ESC benzeri bir algoritma kurmak ve frenleme sistemini implement etmek.

#### Yapılacaklar

**CS Görevi:**
- [ ] Slip angle tahmini (sideslip angle β hesaplama)
- [ ] Understeer/Oversteer tespiti ve müdahale algoritması
- [ ] Dinamik/Rejeneratif frenleme: TB6612FNG BRAKE modu aktivasyonu
- [ ] Dashboard'a anlık stabilite uyarısı ve müdahale log'u ekleme (WebSocket notify)

**EE Görevi:**
- [ ] Rejeneratif frenleme sırasında motor geri-EMF ölçümü (osiloskop)

#### Kayma Açısı (Sideslip Angle) Tahmini

```c
// Basit kinematik model ile arka aksın slip açısı
// beta: araç merkezi kayma açısı
// v_x: boyuna hız, v_y: yanal hız, psi_dot: Yaw Rate
float beta = atanf(v_y / v_x); // v_y ivme integrasyonuyla

// Oversteer algılama
if (fabsf(beta) > SLIP_THRESHOLD_RAD) {
    // ESC müdahalesi: iç tekerleği frenleme
    applyESC(beta, yaw_rate);
}
```

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| ESC (Electronic Stability Control) çalışma prensibi | Otomotiv |
| Sideslip Angle ve araç yönü farkı | Dinamik |
| Rejeneratif Frenleme (H-Bridge brake modu) | EE/Otomotiv |
| Back-EMF ve motor jeneratör modunda çalışması | EE |
| Understeer vs Oversteer araç davranışı | Otomotiv |
| ADAS (Advanced Driver Assistance Systems) kavramı | Kariyer |

---

### 🖥️ HAFTA 8 — PCB Tasarımı, Test ve Dokümantasyon

**Hedef:** Tüm devre KiCad'e aktarılacak, PCB tasarımı yapılacak ve araç asfalt testine alınacak.

#### Yapılacaklar

**EE Görevi:**
- [ ] KiCad şematik çizimi (tüm bileşenler: ESP32, TB6612FNG, MPU6050, BMS, LM2596)
- [ ] ESP32 Shield PCB layout tasarımı (2 katmanlı)
- [ ] PCB tasarım kuralları: trace genişliği (motor akımı için min. 1mm), via boyutu
- [ ] Gerber dosyası export (JLCPCB veya PCBWay formatı)

**CS Görevi:**
- [ ] Tüm ESP32 kodunun modüler hale getirilmesi (her sistem ayrı .cpp/.h)
- [ ] Python backend'e otomatik CSV export ve oturum loglama özelliği
- [ ] Asfalt test senaryoları: düz ivmelenme, U-dönüş, slalom

**Ortak:**
- [ ] Proje README.md ve teknik dokümantasyon
- [ ] Test videoları ve telemetri verisi arşivlenmesi

#### Bu Hafta Öğrenilen Konular

| Konu | Alan |
|------|------|
| KiCad şematik ve PCB layout temelleri | EE |
| PCB trace genişliği ve akım kapasitesi hesaplama | EE |
| EMC (Elektromanyetik Uyumluluk) temel kuralları | EE |
| Yazılım modülarizasyonu ve Header Guard | CS |
| Test senaryosu tasarlama ve veri analizi | Mühendislik |
| Teknik raporlama ve dokümantasyon | Mühendislik |

---

## 🚀 İLERİ AŞAMALAR (Faz 5+)

---

### Faz 5 — CAN-Bus ile Dağıtık Mimari

**Hedef:** Tek ESP32'li merkezi mimariyi, endüstri standardı CAN-Bus ağıyla çoklu düğümlere bölmek.

#### Mimari Dönüşümü

```
[ÖNCE — Merkezi]              [SONRA — Dağıtık]

ESP32 (Tek)                   Master ECU (ESP32/STM32)
  ├── Motor L/R  ────────────     CAN ID: 0x001
  ├── Servo                            │ CAN-Bus (500kbps)
  ├── MPU6050                ┌─────────┼─────────┐
  └── BLE               [Motor]   [Sensor]  [Telemetri]
                        [Node ]   [Node  ]  [Node     ]
                        [0x010]   [0x020 ]  [0x030    ]
```

#### Bu Fazda Öğrenilecek Konular

| Konu | Alan |
|------|------|
| CAN-Bus fiziksel katman (CANH/CANL, terminasyon) | EE |
| CAN Frame yapısı (ID, DLC, Data, CRC) | CS/EE |
| Arbitrasyon ve çarpışma yönetimi | EE |
| MCP2515 SPI→CAN Bridge kullanımı | EE/CS |
| OBD-II protokolü ve gerçek araç ECU'ları | Otomotiv |

---

### Faz 6 — Ölü Hesaplama ve Konum Tahmini (Dead Reckoning)

**Hedef:** GPS olmadan, yalnızca IMU ve encoder verisiyle 2D konum tahmini.

#### Bu Fazda Öğrenilecek Konular

| Konu | Alan |
|------|------|
| Rotary Encoder (tekerlek odometrisi) | EE |
| Dead Reckoning algoritması | Navigasyon |
| Hata birikimi ve drift analizi | Matematik |
| Extended Kalman Filter (EKF) | CS/Matematik |

---

### Faz 7 — Otonom Şerit Takibi (Computer Vision)

**Hedef:** Kamera modülü ekleyerek basit şerit takip algoritması.

#### Bu Fazda Öğrenilecek Konular

| Konu | Alan |
|------|------|
| OV2640 kamera modülü ve ESP32-CAM | EE/CS |
| OpenCV Canny Edge Detection | CS |
| Hough Transform ile çizgi tespiti | CS |
| PID ile şerit merkezine yönelme | Kontrol |

---

### Faz 8 — LiDAR ile Engel Algılama

**Hedef:** RPLiDAR A1 veya TFmini ile 360° engel haritası.

#### Bu Fazda Öğrenilecek Konular

| Konu | Alan |
|------|------|
| LiDAR çalışma prensibi (ToF / Triangulation) | EE |
| Pointcloud verisi işleme | CS |
| Basit SLAM (Simultaneous Localization and Mapping) | Robotik |
| Engel kaçınma algoritması (VFH) | Robotik |

---

## 📋 Otomotiv Mühendisliğinin Olmazsa Olmazları — Kontrol Listesi

| # | Konsept | Projede Yer | Hafta/Faz |
|---|---------|-------------|-----------|
| 1 | Ackermann Direksiyon Geometrisi | Aktif | Hafta 2 |
| 2 | Torque Vectoring / E-Diff | Aktif | Hafta 6 |
| 3 | Sensör Füzyonu (Complementary / Kalman) | Aktif | Hafta 5 |
| 4 | Traction Circle | Aktif | Hafta 5 |
| 5 | Slip Angle Analizi | Aktif | Hafta 7 |
| 6 | ESC (Electronic Stability Control) | Aktif | Hafta 7 |
| 7 | Rejeneratif Frenleme | Aktif | Hafta 7 |
| 8 | Fail-Safe / Functional Safety (ISO 26262) | Aktif | Hafta 3 |
| 9 | RTOS Görev Zamanlama | Aktif | Hafta 3 |
| 10 | Drive-by-Wire Mimarisi | Aktif | Hafta 3 |
| 11 | CAN-Bus Ağ Protokolü | Sonraki Aşama | Faz 5 |
| 12 | OBD-II Diagnostics | Sonraki Aşama | Faz 5 |
| 13 | Dead Reckoning / Odometri | Sonraki Aşama | Faz 6 |
| 14 | LiDAR Engel Algılama | Sonraki Aşama | Faz 8 |
| 15 | ADAS (Şerit Takibi) | Sonraki Aşama | Faz 7 |
| 16 | PCB Tasarımı (EMC Uyumlu) | Aktif | Hafta 8 |
| 17 | Termal Yönetim (Motor ısınma) | Aktif | Hafta 6 |
| 18 | Bicycle Model (Araç Kinematiği) | Aktif | Hafta 6 |
| 19 | Extended Kalman Filter | Sonraki Aşama | Faz 6 |
| 20 | SLAM | Sonraki Aşama | Faz 8 |

---

## 🛠️ Haftalık Araç ve Teknoloji Şeması

```
Hafta 1-2: [PlatformIO] + [TB6612FNG] + [MG995] + [Fusion360]
Hafta 3:   + [FreeRTOS] + [BLE GATT] + [Python/bleak] + [WebSocket] + [Chart.js Dashboard]
Hafta 4:   + [MPU6050/I2C] + [WS Telemetri Akışı]
Hafta 5:   + [Kalman/CF Filter] + [Traction Circle Dashboard]
Hafta 6:   + [E-Diff Algorithm] + [PID Controller]
Hafta 7:   + [ESC Algorithm] + [Regen Braking] + [WS Uyarı Bildirimi]
Hafta 8:   + [KiCad] + [Gerber Export] + [Python CSV Logger + Pandas Analiz]
Faz 5+:    + [CAN-Bus] + [MCP2515] + [OpenCV] + [LiDAR]
```

---

## 📚 Kaynaklar ve Referanslar

### Kitaplar
- **"Vehicle Dynamics and Control"** — Rajesh Rajamani (Araç dinamiğinin İncil'i)
- **"The Art of Electronics"** — Horowitz & Hill (Devre tasarımı)
- **"FreeRTOS Reference Manual"** — Richard Barry (Ücretsiz PDF)

### Online Kaynaklar
- ESP32 Teknik Referans Kılavuzu (Espressif Docs)
- MPU6050 Register Map (InvenSense)
- KiCad Resmi Dokümantasyonu
- Phil's Lab (YouTube) — PCB Tasarımı
- MATLAB Tech Talks (YouTube) — Kalman Filter

### Simülasyon Araçları
- **MATLAB/Simulink** — Araç dinamiği simülasyonu (üniversite lisansı)
- **KiCad** — PCB tasarım ve simülasyon
- **Wokwi** — ESP32 online simülatörü (wokwi.com)

---

## 💡 Projenin Değeri — Neden Yapılmalı?

> **"Bu proje 3. sınıf bir CS + EE öğrencisi için yararlı ve faydalı mı?"**

**Cevap: Kesinlikle Evet.**

### Teknik Kazanımlar
- Gerçek donanım üzerinde embedded yazılım geliştirme deneyimi
- Endüstriyel protokolleri (CAN, I2C, BLE, RTOS) fiilen uygulama
- Modern EV ve ADAS sistemlerinin temel bloklarını sıfırdan kurma
- PCB tasarımından gerçek zamanlı web dashboard geliştirmeye uzanan geniş yelpaze
- Python async programlama ve WebSocket iletişimi ile gerçek IoT mimarisi deneyimi

### Kariyer Etkisi

| Bu Proje | Endüstride Karşılığı |
|----------|---------------------|
| TB6612FNG H-Bridge | Tesla Inverter IGBT/SiC Modülü |
| E-Diff Algoritması | Porsche Torque Vectoring Plus |
| Kalman Filter | Tesla Autopilot Sensor Fusion |
| FreeRTOS Tasks | AUTOSAR OS Scheduling |
| BLE Watchdog | ISO 26262 ASIL-B Fail-Safe |
| KiCad PCB | Automotive Grade PCB (AEC-Q) |

- **CS öğrencisi için:** Embedded Systems / Automotive Software Engineer pozisyonlarında rakipsiz portföy.
- **EE öğrencisi için:** PCB tasarımı + Motor sürücü + Sensör füzyonu = Güç elektroniği veya otomotiv EE rollerinde somut kanıt.
- **İkisi birlikte:** Sistem bütünleştirme yetkinliği — sizi sadece "kod yazan" veya "devre çizen" birinden ayıran şey.

### Akademik Fırsat
- Bitirme projesi veya lisansüstü başvuru portföyü olarak kullanılabilir
- IEEE / SAE makale konusu olabilir (Sensör Füzyonu veya E-Diff bölümü)
- TÜBİTAK 2209-A veya 2209-B destek programına başvurulabilir

---

## 🎯 Haftalık Başarı Kriterleri

| Hafta | Kabul Kriteri |
|-------|--------------|
| 1 | Motor ileri-geri-dur komutu alıyor, BMS kısa devre koruması test edildi |
| 2 | Servo -30° / +30° aralığında hassas kontrol, Ackermann geometrisi fiziksel doğrulandı |
| 3 | Python betiğinden BLE ile araç kontrol ediliyor, tarayıcı dashboard açık, bağlantı kopunca araç duruyor |
| 4 | MPU6050 raw data 100Hz'de okunuyor, tarayıcı dashboard'da canlı sayısal telemetri görülüyor |
| 5 | Filtrelenmiş Pitch/Roll titreşimde <1° drift, Traction Circle scatter chart tarayıcıda çiziliyor |
| 6 | Virajda iç tekerlek daha yavaş dönüyor (ölçümlü), dönüş yarıçapı düz gidişe göre azaldı |
| 7 | Keskin dönüşte ESC müdahalesi loglanıyor, regen fren komutu motorları freniyor |
| 8 | KiCad şematik DRC hatasız, Gerber export hazır, telemetri CSV'si kaydediliyor |

---

*Son güncelleme: 4 Ekim 2026 | Proje Durumu: Planlama Aşaması*
