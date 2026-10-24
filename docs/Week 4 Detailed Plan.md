# 📋 VDT Platform — Hafta 4 Planı (MPU6050 IMU ve Canlı Telemetri)

> **Mevcut Durum:** Hafta 3 tamamlandı. Araç, ESP32'nin kendi Wi-Fi ağı (`VDT-Car`) üzerinden telefon tarayıcısından WebSocket ile sürülüyor; FreeRTOS çift çekirdek mimarisi (Core 0: ağ, Core 1: `vehicleTask`) ve 500 ms fail-safe çalışıyor.  
> **Ekip:** 1× Bilgisayar Mühendisliği (CS) + 1× Elektrik-Elektronik Mühendisliği (EE)  
> **Mimari Notu:** ROADMAP'teki "BLE ile telemetri" ifadesi Hafta 3'teki Wi-Fi kararıyla **geçersizdir**; telemetri mevcut WebSocket kanalından (ters yönde) gönderilir.  
> **Hedef:** MPU6050'yi I2C ile bağlamak, ham veriyi **kalibre edilmiş SI birimlerine** çevirmek ve bu veriyi sürüş komutlarını **bozmadan** telefondaki dashboard'a canlı akıtmak.

> Bu plan zaman değil **konu/iş** odaklıdır. Yazılım kısımları **gereksinim** olarak tanımlanmıştır, kodu ekip yazar.

---

## 🧭 1. Konu Haritası ve Bağımlılıklar

```
[A] Pin & Donanım Bağlantısı ──▶ [B] I2C Tarama & Sensör Sürücüsü ──▶ [C] Kalibrasyon & SI Dönüşümü ──┐
                                                                                                      ├──▶ [E] Telemetri Protokolü ──▶ [F] Dashboard Gösterimi ──▶ [G] Test & Doğrulama
                                          [D] FreeRTOS Sensör Görevi (Core/Öncelik/Thread güvenliği) ──┘
```

| Konu | Sorumlu | Bağımlı Olduğu |
|---|---|---|
| A. Donanım bağlantısı ve montaj | EE | — |
| B. I2C tarama ve sensör sürücüsü (`imu_driver`) | CS | A |
| C. Kalibrasyon ve SI birim dönüşümü | CS (+EE doğrulama) | B |
| D. FreeRTOS sensör görevi ve veri paylaşımı | CS | B |
| E. Telemetri protokolü (JSON, ESP32 → Telefon) | CS | C, D |
| F. Dashboard'a canlı veri gösterimi | CS | E |
| G. Test ve doğrulama | Ortak | C, F |

---

## ⚡ 2. Kritik Mühendislik Kuralları

1. **Sensör 3.3 V ile beslenir:** MPU6050 (GY-521 modülü) ESP32'nin `3V3` pininden beslenmelidir. I2C hatları 3.3 V lojiktir; 5 V'a bağlanırsa ESP32 GPIO'ları zarar görebilir.
2. **Pull-up dirençleri:** GY-521 modüllerinin çoğunda 4.7 kΩ pull-up zaten vardır. **Önce modülü kontrol et**; yoksa SDA ve SCL hatlarına 4.7 kΩ (3.3 V'a) ekle. Çift pull-up eş değer direnci düşürür, hat bozulmaz ama gereksizdir.
3. **Kısa ve düzgün I2C kabloları:** Motor ve servo kablolarından uzak, mümkünse kısa (< 15 cm) tut. PWM gürültüsü I2C okumalarını bozabilir.
4. **Ortak GND:** Sensör GND'si ESP32 GND'sine (star ground) bağlı olmalı.
5. **Sürüş güvenliği telemetriden önce gelir:** Sensör okuma/gönderme hatası (I2C kilitlenmesi, WebSocket tıkanması) **asla** `vehicleTask`'ı ve watchdog'u geciktirmemeli. Sensör ayrı bir görevde, **daha düşük öncelikle** çalışır.
6. **WebSocket kütüphanesi thread-safe değildir:** `broadcastTXT` yalnızca `loop()`'u çalıştıran çekirdekten (Core 0) çağrılmalı; sensör görevi veriyi paylaşılan bir yapıya yazar, gönderimi `WifiService` yapar (bkz. İş Paketi D).
7. **Test sırası:** Önce tezgâhta (araç hareketsiz) ölç ve kalibre et; sonra tekerlekler havadayken motor titreşimini gözle; en son yerde sürüş.
8. **Kalibrasyon yerinde ve düz zeminde yapılır:** Gyro offset araç tamamen hareketsizken, ivmeölçer düz zeminde ölçülür.

---

## 🔩 3. MPU6050 Özet Bilgileri

| Özellik | Değer |
|---|---|
| Sensör | 3 eksen ivmeölçer + 3 eksen jiroskop (6-DOF), 16-bit ADC |
| Besleme | 3.3 V (GY-521 modülünde regülatör vardır, 3.3–5 V kabul eder; biz 3.3 V kullanıyoruz) |
| Haberleşme | I2C, 100 kHz (standart) / 400 kHz (hızlı) |
| I2C adresi | `0x68` (AD0 = GND/boş) veya `0x69` (AD0 = 3.3 V) |
| ESP32 pinleri | `GPIO 21` = SDA, `GPIO 22` = SCL (varsayılan I2C, `pinout.md`'de rezerve) |
| İvme aralığı | ±2g (16384 LSB/g) · ±4g (8192) · ±8g (4096) · ±16g (2048) |
| Gyro aralığı | ±250°/s (131 LSB/°/s) · ±500 (65.5) · ±1000 (32.8) · ±2000 (16.4) |
| Sıcaklık | Dahili sensör (dönüşüm: `T[°C] = raw / 340 + 36.53`) |
| Kimlik kaydı | `WHO_AM_I` (0x75) → `0x68` döner (klon modüllerde `0x70`/`0x72` görülebilir) |
| Uyku modu | Açılışta **uykudadır**; `PWR_MGMT_1` (0x6B) = 0 yazılarak uyandırılır |

> ⚠️ Piyasadaki GY-521 modüllerinin bir kısmı klon çiplidir (`WHO_AM_I` farklı dönebilir). Bu durumda sürücü kimlik kontrolünü **uyarı** olarak yazmalı, sensörü reddetmemelidir.

### Kapsam Kararı: Aralık Seçimi

| Eksen | Seçim | Gerekçe |
|---|---|---|
| İvme | **±4g** | Bu ölçekteki araç ±1g'yi nadiren aşar; ±2g'de çarpma/zıplamada doyma riski var |
| Gyro | **±500°/s** | Küçük aracın yaw hızı yüksek olabilir; ±250 doyabilir |
| DLPF | **~42 Hz** | Motor titreşim gürültüsünü kırpar, gecikme ~5 ms |

*(Ekip değiştirebilir; seçilen aralık dönüşüm ölçeğini doğrudan etkiler. Ölçek sabitleri tek yerde tanımlanmalı.)*

---

## 🧱 4. İş Paketleri

### A. Donanım Bağlantısı ve Montaj (EE)
**Amaç:** MPU6050'yi güvenli ve titreşimden korunmuş şekilde ESP32'ye bağlamak.

| MPU6050 (GY-521) | ESP32 | Not |
|---|---|---|
| VCC | `3V3` | 3.3 V |
| GND | `GND` | Ortak toprak |
| SDA | `GPIO 21` | I2C veri |
| SCL | `GPIO 22` | I2C saat |
| AD0 | `GND` | Adres = 0x68 |
| INT | (bağlanmaz) | Bu hafta kullanılmayacak (Hafta 5 opsiyonu: veri hazır kesmesi) |

**Yapılacaklar:**
- Modülde pull-up direnç var mı kontrol et (multimetre: SDA–3V3 arası ≈ 4.7 kΩ).
- Sensörü aracın **ağırlık merkezine yakın**, düz ve sabit bir yüzeye monte et (çift taraflı köpük bant / TPU pad). Eksenler (X ileri, Y sola/sağa, Z yukarı) **belgelenmeli**.
- Kabloları motor ve servo kablolarından ayır; zip-tie ile sabitle.
- Motor çalışırken 3V3 hattında dalgalanma olup olmadığını ölç.
- **Kabul:** Sensör sağlam monte edilmiş, eksen yönleri çizilmiş, 3V3 hattı motor çalışırken ≥ 3.2 V.

### B. I2C Tarama ve Sensör Sürücüsü (CS)
**Amaç:** Donanımı doğrulayıp sensörü bağımsız bir modülle okumak.
**Gereksinimler:**
- **Önce I2C scanner:** Geçici bir test kodu ile bus taranır; `0x68` görülmeden sürücü yazılmaz. Görülmüyorsa donanım (kablo/pull-up/adres) çözülür, yazılım değil.
- **SRP:** `imu_driver` yalnızca sensör haberleşmesi ve ham/SI veri üretiminden sorumludur; WebSocket, motor veya filtre mantığı içermez.
- Sensörü uyandırır, aralık ve DLPF'i ayarlar, `WHO_AM_I` kontrolü yapar.
- Tek bir burst okuma ile (`0x3B`'den 14 bayt) ivme + sıcaklık + gyro birlikte okunur (tek I2C işlemi, tutarlı zaman damgası).
- `begin()` başarısızsa (sensör yok) sistem **çökmemeli**: araç sürülebilmeli, telemetri "sensör yok" olarak işaretlenmeli.
- Okuma hatası (I2C NACK/timeout) sayılmalı ve hata sayacı tutulmalı.
- Pinler `pins.h`'a (`PIN_IMU_SDA`, `PIN_IMU_SCL`) eklenir; `docs/pinout.md`'de "Rezerve → Kullanılan" taşınır.
- **Kabul:** Sürücü sensörü başlatıyor, Serial'de ham veri akıyor, sensör sökülünce araç hâlâ sürülebiliyor.

### C. Kalibrasyon ve SI Birim Dönüşümü (CS + EE)
**Amaç:** Ham sayıları güvenilir fiziksel büyüklüklere çevirmek.

**Dönüşüm formülleri (±4g, ±500°/s için):**
```c
// İvme: ±4g -> 8192 LSB/g
float ax_g   = (float)raw_ax / 8192.0f;
float ax_ms2 = ax_g * 9.80665f;

// Gyro: ±500°/s -> 65.5 LSB/(°/s)
float gz_dps = (float)raw_gz / 65.5f;
```

**Kalibrasyon gereksinimleri:**
- **Gyro bias:** Açılışta araç hareketsizken ~1–2 saniye boyunca (≥ 200 örnek) ortalama alınır, her okumadan çıkarılır. Kalibrasyon sırasında araca dokunulmamalı; bu süre boyunca Serial'e uyarı yazılır.
- **Hareket kontrolü:** Kalibrasyon sırasında gyro varyansı eşiği aşarsa (araç oynuyor) kalibrasyon tekrarlanır.
- **İvmeölçer offset:** Araç düz zeminde dururken `ax≈0, ay≈0, az≈+1g` olmalı. Sapma ölçülür; küçükse offset olarak çıkarılır, büyükse montaj eğimi sorgulanır.
- **Eksen eşleme:** Sensör eksenleri araç eksenleriyle uyuşmuyorsa (ters/çapraz monte), **tek bir yerde** (örn. `imu_driver` içinde dönüşüm matrisi/işaret tablosu) araç eksenine (X ileri, Y sol, Z yukarı) çevrilir.
- **Doğrulama (EE):** Aracı elle 90° yatır; ilgili eksenin ≈ ±1g'ye gittiği, ivme vektör büyüklüğünün ≈ 1g kaldığı görülmeli.
- Ölçülen offset değerleri `docs/vehicle_params.md`'ye kaydedilir.
- **Kabul:** Hareketsizken `|a| ≈ 1.0 ± 0.05 g`, gyro ≈ 0 ± 1 °/s; aracı eğince doğru eksen ve işaret.

### D. FreeRTOS Sensör Görevi ve Thread Güvenliği (CS)
**Amaç:** Sensör okumayı, ağ ve motor görevlerinden izole edip güvenli veri paylaşmak.

```
Core 0 (loop):                         Core 1:
┌────────────────────────┐             ┌───────────────────────┐
│ wifiService.loop()     │             │ vehicleTask (Öncelik 2)│  ◀── commandQueue
│ + telemetri yayını     │             └───────────────────────┘
│   (broadcastTXT, 20 Hz)│             ┌───────────────────────┐
└──────────▲─────────────┘             │ imuTask (Öncelik 1)   │
           │  paylaşılan ImuData       │ 100 Hz okuma          │
           └───────────────────────────┴───────────────────────┘
```

**Gereksinimler:**
- `imuTask`: Sabit periyotlu (hedef **100 Hz / 10 ms**) çalışır. Gecikme için `vTaskDelayUntil` kullanılır (`vTaskDelay` periyodu kaydırır).
- **Öncelik:** `vehicleTask`'tan **düşük** (`imuTask` = 1, `vehicleTask` = 2). Sensör hiçbir koşulda motor görevini aç bırakmamalı.
- **Veri paylaşımı:** En güncel örnek tek elemanlı bir kuyruk (`xQueueOverwrite`/`xQueuePeek`) veya mutex korumalı yapı ile paylaşılır. Aynı mantık, Hafta 3'teki `commandQueue` ile tutarlıdır ("en taze veri kazanır").
- **Telemetri gönderimi** `WifiService` içinde, Core 0'da, **20 Hz** (50 ms) ile yapılır; sensör 100 Hz okusa da ağ 20 Hz gönderir (bant genişliği ve paket tıkanması önlenir).
- İstemci bağlı değilse veya sensör yoksa gönderim yapılmaz.
- Yığın (stack) boyutu ölçülmeli (`uxTaskGetStackHighWaterMark`), gereksiz büyük seçilmemeli.
- **Kabul:** 100 Hz okuma sabit; sürüş sırasında komut gecikmesi artmıyor; fail-safe (500 ms) aynen çalışıyor.

### E. Telemetri Protokolü (CS)
**Amaç:** ESP32 → telefon yönünde hafif, genişletilebilir bir mesaj formatı tanımlamak.

- **Yön:** Hafta 3'te yalnızca telefon → ESP32 akışı vardı. Artık WebSocket çift yönlü kullanılır.
- **Önerilen mesaj:**
  ```json
  {"type":"telemetry","t":123456,"ax":0.02,"ay":-0.10,"az":0.99,
   "gx":0.3,"gy":-0.1,"gz":1.2,"temp":31.4,"imu":true}
  ```
  - İvme birimi **g**, gyro birimi **°/s**, `t` = `millis()`.
  - Alan adları kısa tutulur (paket boyutu küçük, `StaticJsonDocument` boyutu ölçülmeli).
  - Sensör yoksa `"imu": false` ve değer alanları atlanır.
- **Gelen mesajlardan ayrım:** Telefon → ESP32 komutları (`throttle/steering/emergency`) ile ESP32 → telefon telemetrisi `type` alanıyla ayrışır; mevcut komut ayrıştırma bozulmamalı.
- `docs/protocol.md` ve `protocol.h` (örn. `TelemetryPacket` yapısı) güncellenir.
- **Kabul:** Tarayıcı geliştirici konsolunda telemetri mesajları 20 Hz akıyor; sürüş komutları etkilenmiyor.

### F. Dashboard'a Canlı Veri Gösterimi (CS)
**Amaç:** Hafta 5'teki gelişmiş görselleştirmeye (yapay ufuk, G-meter) zemin olacak **basit** bir gösterim.
- Mevcut dashboard'a küçük bir telemetri paneli: `ax, ay, az (g)` ve `gx, gy, gz (°/s)` sayısal değerleri.
- Telemetri gelmezse ("IMU yok / veri bekleniyor") açıkça belirtilir.
- Bağlantı göstergesi ve joystick'ler **bozulmamalı** (dokunmatik alan daralmamalı).
- Ek kütüphane (CDN) **yok**; dashboard hâlâ PROGMEM içinde tek parça.
- Boyut kontrolü: Flash kullanımı ≈ %62'den belirgin artmamalı.
- *(Chart.js, Canvas yapay ufuk, Traction Circle → Hafta 5.)*
- **Kabul:** Telefon ekranında canlı değerler akıyor; aracı eğince değerler doğru eksende değişiyor.

### G. Test ve Doğrulama (Ortak)
- **Tezgâh testi:** Hareketsiz gürültü seviyesi, eğme testi, eksen doğrulaması.
- **Motor titreşimi:** Tekerlekler havada, motor %50 / %100 gazda iken ivme sinyalindeki gürültü (Serial Plotter ile) gözlenir; DLPF/montaj iyileştirmesi gerekirse not alınır.
- **Sürüş testi:** Yerde ileri/geri ve viraj; ivme `ax` gaz/frende, `ay` virajda beklenen işaretle değişmeli.
- **Güvenlik regresyonu:** Telemetri açıkken Safari kapat/menzil dışı → 500 ms içinde araç durmalı (Hafta 3 davranışı bozulmadı).
- **Dayanıklılık:** ~10 dk kesintisiz telemetri + sürüş; I2C hata sayacı ≈ 0, ESP32 reset yok.
- **Akım kontrolü:** IMU + Wi-Fi + servo + motor birlikte iken 5 V / 3V3 hattında düşüş (brown-out) yok.
- Kod `git` ile commit'lenir, hafta sonunda etiketlenir (ör. `v0.4-week4`).

---

## 🧩 5. Ek Konular (Opsiyonel)

| Konu | Neden Faydalı? | Zorluk |
|---|---|---|
| I2C kilitlenme kurtarma (SCL toggling / `Wire.end()` yeniden başlatma) | Titreşim/gürültü sonrası bus donmasına karşı dayanıklılık | Orta |
| Telemetri CSV kaydı (Serial üzerinden) | Hafta 5 filtre karşılaştırması için veri seti | Kolay |
| MPU6050 `INT` pini ile "veri hazır" kesmesi | Sabit zamanlama, boşa okuma yok | Orta |
| Batarya voltajı telemetriye ekleme (GPIO 34, direnç bölücü) | Pil boşalma uyarısı | Orta |
| Telemetri sıklığı ayarlanabilirliği (10/20/50 Hz) | Wi-Fi yükü / menzil dengesi | Kolay |
| Çarpma/devrilme algılama (|a| eşiği) → otomatik durdurma | Fail-safe'in doğal uzantısı | Orta |

> **Öneri:** *Telemetri CSV kaydı* (Hafta 5 filtre çalışması için veri biriktirir) ve *I2C kilitlenme kurtarma* (saha güvenilirliği).

---

## 📁 6. Hedeflenen Proje Yapısı

```
software/
├── include/  → pins.h (+IMU pinleri), imu_driver.h (yeni), protocol.h (+TelemetryPacket), wifi_service.h (+telemetri yayını)
└── src/      → main.cpp (+imuTask), imu_driver.cpp (yeni), wifi_service.cpp (+telemetri, dashboard paneli)
docs/
├── pinout.md          → IMU pinleri "Kullanılan"a taşınacak
├── protocol.md        → telemetri mesajı eklenecek
├── vehicle_params.md  → IMU offset değerleri ve eksen yönleri
└── Week 4 Detailed Plan.md
```
*(Dosya adları öneridir; ekip değiştirebilir.)*

---

## 🎯 7. Hafta 4 Başarı Kriterleri (Definition of Done)

Aşağıdakilerin tümü tamamlanmadan Hafta 5'e (Sensör Füzyonu ve Filtreler) geçilmez:

- [ ] **Donanım:** MPU6050 3.3 V ile besleniyor, pull-up doğrulandı, sağlam monte edildi, eksen yönleri belgelendi.
- [ ] **I2C taraması:** `0x68` adresi görüldü, `WHO_AM_I` okundu.
- [ ] **Sürücü:** `imu_driver` bağımsız modül; pinler yalnızca `pins.h`'tan geliyor; sensör yokken araç çökmüyor.
- [ ] **SI dönüşümü:** İvme g/m·s⁻², gyro °/s olarak doğru ölçeklenmiş; seçilen aralıklar belgelenmiş.
- [ ] **Kalibrasyon:** Hareketsizken `|a| ≈ 1g ± 0.05`, gyro ≈ 0 ± 1 °/s; offset'ler `vehicle_params.md`'de.
- [ ] **Eksen doğrulaması:** Aracı eğince/döndürünce doğru eksen ve işaret tepki veriyor.
- [ ] **FreeRTOS:** `imuTask` 100 Hz, `vehicleTask`'tan düşük öncelikli; WebSocket gönderimi yalnızca Core 0'da.
- [ ] **Telemetri:** ESP32 → telefon 20 Hz JSON akıyor; `protocol.md` güncel.
- [ ] **Dashboard:** Telefonda canlı ivme/gyro değerleri görünüyor; joystick ve fail-safe bozulmadı.
- [ ] **Güvenlik regresyonu:** Telemetri açıkken bağlantı kopunca araç 500 ms içinde duruyor.
- [ ] **Dayanıklılık:** ~10 dk testte I2C hata sayacı ≈ 0, reset/brown-out yok.

---

## 🚀 8. Sonraki Adım: Hafta 5'e Hazırlık

Kalibre, güvenilir ve canlı akan IMU verisi elde edildiğinde:
1. **Complementary Filter** ile gürültülü ivme ve kayan gyro birleştirilip Pitch/Roll hesaplanacak.
2. Ham vs filtrelenmiş veri, CSV kayıtları ile karşılaştırılacak (Serial Plotter / Python).
3. Dashboard'a **Yapay Ufuk** ve **Traction Circle (G-Meter)** eklenecek.
4. Anti-vibration mount (TPU pad) tasarımı iyileştirilecek.
