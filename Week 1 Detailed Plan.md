# 📋 VDT Platform — Hafta 1 Detaylı Uygulama Planı (Week 1 Detailed Plan)

> **Mevcut Durum:** Şase ve tekerlek montajı tamamlandı, DC motorlar mekanik yataklarına oturtuldu. Şase içinde henüz hiçbir elektronik bileşen yer almıyor.  
> **Ekip:** 1× Bilgisayar Mühendisliği (CS) + 1× Elektrik-Elektronik Mühendisliği (EE)  
> **1. Hafta Nihai Hedefi:** Güvenli güç hattını kurmak (Power Integrity) ve ESP32 + TB6612FNG ile motorları tezgâhta (bench test) ilk kez kontrollü şekilde döndürmek.

---

## 🧭 1. Genel Strateji ve Yol Haritası (Bundan Sonra Nasıl İlerlemeliyiz?)

Robotik ve gömülü sistem geliştirmede en sık yapılan hata, tüm elektronik kartları doğrudan şaseye vidalayıp kabloları bağlayarak tek seferde çalıştırmayı denemektir. Bu durum kısa devrelere, voltaj dalgalanmalarına ve yanan mikrodenetleyicilere (ESP32) yol açar.

İzleyeceğimiz **4 Aşamalı Altın Mühendislik Sırası**:

```
[Aşama 1: Güç & Güvenlik Doğrulama] (EE)
   └── Pil + BMS + LM2596 Regülatör (Multimetre ile 5.0V doğrulaması yapılmadan ESP32 bağlanmaz!)
        │
        ▼
[Aşama 2: Masaüstü (Bench) Devre Kurulumu] (EE + CS)
   └── Breadboard üzerinde ESP32 + TB6612FNG + Motor bağlantısı
        │
        ▼
[Aşama 3: Temel Yazılım & Motor Sürüş Testi] (CS)
   └── PlatformIO, sade PWM kontrolü (KISS & SOLID), ileri-geri-dur testi
        │
        ▼
[Aşama 4: Şase İçi Entegrasyon & Kablo Düzeni] (EE + CS)
   └── Çalıştığı kanıtlanan devrenin şaseye taşınması ve ağırlık merkezine göre sabitlenmesi
```

---

## ⚡ 2. Kritik Mühendislik Kuralları (Donanım Koruma)

1. **Multimetre Şartı:** LM2596 regülatör potansiyometresi ayarlanıp çıkışta net **5.0V** görülmeden çıkış ucu ESP32'ye **asla** bağlanmamalıdır.
2. **Common Ground (Ortak Toprak):** Pil eksi ucu, LM2596 GND, ESP32 GND ve TB6612FNG GND hatları tek bir ortak referans noktasında (Star Ground) birleştirilmelidir. Ortak toprak olmazsa PWM sinyalleri referans bulamaz ve motorlar rastgele titrer.
3. **Motor vs. Lojik Voltaj Ayrımı (TB6612FNG):**
   - `VM` pini: Doğrudan 2S Li-Ion pilden (7.4V – 8.4V) motor gücü alır.
   - `VCC` pini: ESP32 lojik gerilimi ile (3.3V veya regüle 5V) beslenir.
4. **STBY (Standby) Pini:** TB6612FNG sürücüsünün `STBY` pini lojik `HIGH` (3.3V/5V) yapılmadığı sürece H-Köprüsü uykuda kalır, motorlar dönmez.

---

## 👥 3. Rol Dağılımı ve Görev Dağılımı

### ⚡ Elektrik-Elektronik Mühendisi (EE) Görevleri

#### Görev 1.1: Güç Dağıtım Hattı ve Güvenlik
- **Bileşenler:** 2S 18650 Li-Ion pil yuvası, 2S BMS koruma kartı, açma/kapama anahtarı, 2A-3A sigorta/polyswitch.
- **Yapılacaklar:**
  1. BMS kartını pillerle lehimle/bağla (BMS: aşırı deşarj ve kısa devre koruması sağlar).
  2. Pil çıkışına fiziksel bir bas-çek/toggle güç anahtarı ve seri sigorta ekle.
  3. Yüksüz durumda pil paketinin voltajını ölç (Dolu 2S: ~8.4V, Nominal: ~7.4V).

#### Görev 1.2: LM2596 Voltaj Regülasyonu
- **Yapılacaklar:**
  1. Pil paketini LM2596 `IN+` ve `IN-` pinlerine bağla.
  2. Multimetreyi `OUT+` ve `OUT-` uçlarına tak.
  3. LM2596 üzerindeki mavi çok turlu trimpotu tornavidayla çevirerek çıkışı hassas **5.00V** değerine sabitle.
  4. Çıkış dalgalanmasını (ripple) sönümlemek için çıkış hattına 100µF elektrolitik + 100nF seramik kondansatör ekle.

#### Görev 1.3: TB6612FNG Motor Sürücü Breadboard Kurulumu
- **Pin Haritası:**
  - `VM`: Pil Pozitif Hattı (+7.4V)
  - `VCC`: Regüle 3.3V (ESP32 3V3 pini önerilir)
  - `GND`: Ortak Toprak Hattı
  - `STBY`: ESP32 GPIO pini (veya test için doğrudan 3.3V)
  - `PWMA` / `PWMB`: ESP32 PWM pinleri
  - `AIN1`, `AIN2`: Sol motor yön pinleri
  - `BIN1`, `BIN2`: Sağ motor yön pinleri
  - `AO1`, `AO2`: Sol DC Motor
  - `BO1`, `BO2`: Sağ DC Motor

---

### 💻 Bilgisayar Mühendisi (CS) Görevleri

#### Görev 1.4: Geliştirme Ortamı (PlatformIO Kurulumu)
- VS Code üzerinde PlatformIO eklentisini kur.
- Yeni proje oluştur: Board olarak elinizdeki ESP32 modelini seçin (örn. `esp32dev`).
- `platformio.ini` dosyasını yapılandır:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

#### Görev 1.5: Temiz ve Sade Motor Sürücü Modülü (KISS & SOLID)
- Motor sürüş mantığını ana `main.cpp` içine doldurmak yerine, tek sorumluluk ilkesine (Single Responsibility) uygun bağımsız bir modül tasarla.
- ESP32'nin donanımsal `ledc` PWM kütüphanesini kullanarak motor yön ve hız fonksiyonlarını yaz.

---

## 🔌 4. Pin Bağlantı Tablosu (Pinout Matrix)

| TB6612FNG Pini | ESP32 GPIO | Açıklama |
|---|---|---|
| **PWMA** | `GPIO 25` | Sol Motor Hız Kontrolü (PWM) |
| **AIN1** | `GPIO 26` | Sol Motor Yön Pini 1 |
| **AIN2** | `GPIO 27` | Sol Motor Yön Pini 2 |
| **PWMB** | `GPIO 14` | Sağ Motor Hız Kontrolü (PWM) |
| **BIN1** | `GPIO 12` | Sağ Motor Yön Pini 1 |
| **BIN2** | `GPIO 13` | Sağ Motor Yön Pini 2 |
| **STBY** | `GPIO 33` | Sürücü Standby/Aktif Pini |
| **VCC** | ESP32 `3V3` | Mantık Voltajı (3.3V) |
| **VM** | Pil (+) 7.4V | Motor Güç Hattı |
| **GND** | ESP32 `GND` + Pil (-) | Ortak Şase (Common Ground) |

*(Not: Boşta kalan veya strapping pini olmayan GPIO'lar tercih edilmiştir. İhtiyaca göre kartınızın şematiğine göre revize edilebilir.)*

---

## 💻 5. Örnek Test Kodu (KISS & SOLID Prensipli)

Aşağıdaki kod yapısı, karmaşık kütüphanelere girmeden en sade şekilde motorların işlevselliğini test eder.

### Dosya: `include/motor_driver.h`
```cpp
#pragma once
#include <Arduino.h>

// Motor kimlikleri
enum MotorId {
    MOTOR_LEFT,
    MOTOR_RIGHT
};

// Motor dönüş yönleri
enum MotorDirection {
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_STOP
};

// Motor sürücü pin tanımları
struct MotorPins {
    uint8_t pwm;
    uint8_t in1;
    uint8_t in2;
};

// Motor kontrol arayüzü (Tek Sorumluluk: Motor donanım kontrolü)
class MotorDriver {
public:
    MotorDriver(MotorPins leftPins, MotorPins rightPins, uint8_t stbyPin);
    void begin();
    void setMotor(MotorId id, MotorDirection dir, uint8_t speed);
    void stopAll();

private:
    MotorPins _left;
    MotorPins _right;
    uint8_t _stby;
};
```

### Dosya: `src/motor_driver.cpp`
```cpp
#include "motor_driver.h"

// PWM yapılandırması: 20 kHz frekans, 8-bit çözünürlük (0 - 255)
static const uint32_t PWM_FREQ = 20000;
static const uint8_t PWM_RES   = 8;

// MotorDriver kurucu fonksiyonu: Pin atamalarını kaydeder
MotorDriver::MotorDriver(MotorPins leftPins, MotorPins rightPins, uint8_t stbyPin)
    : _left(leftPins), _right(rightPins), _stby(stbyPin) {}

// Donanım pinlerini ve PWM kanallarını başlatır
void MotorDriver::begin() {
    pinMode(_stby, OUTPUT);
    digitalWrite(_stby, HIGH); // Sürücüyü uyandır

    pinMode(_left.in1, OUTPUT);
    pinMode(_left.in2, OUTPUT);
    pinMode(_right.in1, OUTPUT);
    pinMode(_right.in2, OUTPUT);

    // ESP32 Arduino Core 3.x ledcAttach API'si
    ledcAttach(_left.pwm, PWM_FREQ, PWM_RES);
    ledcAttach(_right.pwm, PWM_FREQ, PWM_RES);

    stopAll();
}

// Belirtilen motorun yön ve hızını ayarlar
void MotorDriver::setMotor(MotorId id, MotorDirection dir, uint8_t speed) {
    const MotorPins& p = (id == MOTOR_LEFT) ? _left : _right;

    switch (dir) {
        case DIR_FORWARD:
            digitalWrite(p.in1, HIGH);
            digitalWrite(p.in2, LOW);
            break;
        case DIR_BACKWARD:
            digitalWrite(p.in1, LOW);
            digitalWrite(p.in2, HIGH);
            break;
        case DIR_STOP:
        default:
            digitalWrite(p.in1, LOW);
            digitalWrite(p.in2, LOW);
            speed = 0;
            break;
    }

    ledcWrite(p.pwm, speed);
}

// Güvenli acil durdurma: İki motoru da anında frenler
void MotorDriver::stopAll() {
    setMotor(MOTOR_LEFT, DIR_STOP, 0);
    setMotor(MOTOR_RIGHT, DIR_STOP, 0);
}
```

### Dosya: `src/main.cpp`
```cpp
#include <Arduino.h>
#include "motor_driver.h"

// Pin konfigürasyonları
MotorPins leftPins  = { .pwm = 25, .in1 = 26, .in2 = 27 };
MotorPins rightPins = { .pwm = 14, .in1 = 12, .in2 = 13 };
const uint8_t STBY_PIN = 33;

MotorDriver motors(leftPins, rightPins, STBY_PIN);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- VDT Platform: Hafta 1 Motor Testi Basliyor ---");

    motors.begin();
}

void loop() {
    Serial.println("[TEST] Ileri Yon (Hiz: 150/255)");
    motors.setMotor(MOTOR_LEFT, DIR_FORWARD, 150);
    motors.setMotor(MOTOR_RIGHT, DIR_FORWARD, 150);
    delay(2000);

    Serial.println("[TEST] Durdurma (1 saniye)");
    motors.stopAll();
    delay(1000);

    Serial.println("[TEST] Geri Yon (Hiz: 150/255)");
    motors.setMotor(MOTOR_LEFT, DIR_BACKWARD, 150);
    motors.setMotor(MOTOR_RIGHT, DIR_BACKWARD, 150);
    delay(2000);

    Serial.println("[TEST] Durdurma (2 saniye bekleme)");
    motors.stopAll();
    delay(2000);
}
```

---

## 🎯 6. Hafta 1 Başarı Kriterleri (Definition of Done)

Aşağıdaki maddelerin tümü yeşil olmadan Hafta 2'ye (direksiyon servo & Ackermann entegrasyonu) geçilmeyecektir:

- [ ] **Güç Güvenliği:** LM2596 çıkışı multimetre ile 5.00V olarak doğrulandı.
- [ ] **Ortak Şase:** Tüm GND hatları ortak noktaya bağlandı, şasede parazit/kaçak ölçülmedi.
- [ ] **Seri Port:** ESP32 115200 baud ile bilgisayara bağlandı ve loglar başarıyla izlendi.
- [ ] **Motor Yön Doğrulaması:** Sol ve sağ motorun "İleri" komutunda tekerleklerin ikisinin de aracın gidiş yönüne doğru döndüğü teyit edildi (ters dönüyorsa motor kablo uçları yer değiştirilir).
- [ ] **Hız (PWM) Kademesi:** PWM değeri 100, 180, 255 yapılarak hız farkı gözlemlendi.
- [ ] **Şaseye Yerleşim:** Pil yuvası, regülatör ve sürücü şasede dengeli bir şekilde konumlandırıldı, kablolar zip-tie (cırt kelepçe) ile düzenlendi.

---

## 🚀 7. Sonraki Adım: Hafta 2'ye Hazırlık

Hafta 1 başarıyla tamamlandığında aracımız kendi gücüyle ileri-geri gidebilir hale gelecek. Bunun hemen ardından:
1. Ön akstaki SG90 Servo motorun güç ve sinyal hattı eklenecek.
2. Direksiyon geometrisi (-30° ile +30°) test edilecek.
3. Araç ilk viraj alma denemesini yapmaya hazır hale gelecek.
