# 📋 VDT Platform — Hafta 2 Planı (Konu Bazlı)

> **Mevcut Durum:** Hafta 1 tamamlandı. Güç hattı, ESP32, TB6612FNG ve iki DC motor şasede çalışıyor. Kod `software/` altında modüler (`pins.h`, `motor_driver`).  
> **Ekip:** 1× Bilgisayar Mühendisliği (CS) + 1× Elektrik-Elektronik Mühendisliği (EE)  
> **Servo:** SG90 yerine **MG995** (metal dişli, yüksek tork).  
> **Hedef:** MG995 ile ±30° direksiyon kontrolünü güvenli bir güç hattıyla kurmak, kalibre etmek ve motorla birlikte tek bir araç kontrol katmanında birleştirmek.

> Bu plan zaman değil **konu/iş** odaklıdır. Konular arasındaki bağımlılıklar belirtilmiştir; sıra ve tempo ekibe aittir. Yazılım kısımları sadece **gereksinim** olarak tanımlanmıştır, kodu ekip yazar.

---

## 🧭 1. Konu Haritası ve Bağımlılıklar

```
[A] Servo Güç Hattı ───┐
                       ├──▶ [C] Servo Sürücü Yazılımı ──▶ [D] Mekanik Montaj ──▶ [E] Kalibrasyon & Ackermann
[B] Pin Tanımı ────────┘                                                                │
                                                                                        ▼
                                              [F] Direksiyon Katmanı ──▶ [G] Araç Kontrol Katmanı ──▶ [H] Test & Doğrulama
```

| Konu | Sorumlu | Bağımlı Olduğu |
|---|---|---|
| A. Servo güç hattı | EE | — |
| B. Pin tanımı ve dokümantasyon | CS | — |
| C. Servo sürücü yazılımı | CS | A, B |
| D. Mekanik montaj | EE / Mekanik | C (servo merkezlenmeli) |
| E. Kalibrasyon ve Ackermann ölçümü | EE + CS | D |
| F. Direksiyon katmanı | CS | E |
| G. Araç kontrol katmanı | CS | F |
| H. Test ve doğrulama | Ortak | G |

---

## ⚡ 2. Kritik Mühendislik Kuralları (Servo Koruma)

1. **Tek LM2596, iki ayrı dal (Star Split):** Elimizde tek LM2596 var. Çıkışı (OUT+/OUT-) **regülatörün pinlerinde** ikiye ayır: bir dal ESP32'ye, bir dal servoya gitsin. Servo kablosunu ESP32'nin 5V/VIN pininden **geçirme**; her dalın kendi kablosu LM2596 çıkışına gelsin. Böylece servo akım darbesi ESP32 kablosunda gerilim düşümü yapmaz.
2. **Besleme gerilimi:** LM2596 **5.0 V**'ta kalır (ESP32 VIN için uygun, MG995 4.8–7.2 V aralığında olduğu için 5 V'ta çalışır). Tork 6 V'a göre biraz düşer; direksiyon için yeterlidir.
2a. **Akım bütçesi:** LM2596 modülü ~3 A sınıfıdır. ESP32 (~0.25–0.5 A) + servo hareket (~0.5 A) rahat; ancak servo **kilitlenirse (1.2–2.5 A)** regülatör sınıra yaklaşır ve ESP32 resetlenebilir. Bu yüzden mekanik sınırlama ve yazılımsal limit zorunludur. Regülatöre soğutucu/hava akışı düşün.
3. **Ortak GND:** Servo hattı GND'si ESP32 GND'siyle birleşmeli; aksi halde servo titrer veya kendi kendine döner.
4. **Önce yüksüz test:** Servo kolu direksiyona bağlanmadan önce serbest halde test edilmeli.
5. **Yazılımsal sınır:** Direksiyon mekanik sınıra çarparsa servo kilitlenir, aşırı akım çeker, dişliler zarar görür. Sınırlar yazılımda tanımlanmalı.
6. **Kapasitör:** Servo hattına 470–1000 µF elektrolitik + 100 nF seramik, kalkış akımını sönümler.
7. **Merkezleme sırası:** Servo kolu, servo yazılımla merkeze alınmışken ve tekerlekler düz iken takılmalı.

---

## 🔩 3. MG995 Özet Bilgileri

| Özellik | Değer |
|---|---|
| Çalışma gerilimi | 4.8 – 7.2 V (optimum 6.0 V; bu projede ortak LM2596 ile 5.0 V) |
| Kilitlenme torku | ~9.4 kg·cm (4.8 V) / ~11 kg·cm (6 V) |
| Hareket akımı | ~170–500 mA |
| Kilitlenme akımı | ~1.2 – 2.5 A |
| PWM | 50 Hz (20 ms periyot) |
| Darbe genişliği | ~500 – 2500 µs (**kalibre edilmeli**), merkez ~1500 µs |
| Sinyal seviyesi | 3.3 V genelde yeterli; çalışmazsa seviye dönüştürücü |

> ⚠️ Piyasadaki MG995'lerin çoğu klon olabilir: geniş ölü bölge ve düşük konum tekrarlanabilirliği yaygındır. Bu yüzden kalibrasyon (Konu E) zorunludur.

---

## 🧱 4. İş Paketleri

### A. Servo Güç Hattı (EE)
**Amaç:** Tek LM2596 çıkışını ESP32 ve servo için iki bağımsız dala bölmek, servo akım darbelerinin ESP32'yi etkilememesini sağlamak.
- **Bileşenler:** Mevcut LM2596 (5.0 V), 1000 µF + 100 nF (servo dalı), 470 µF + 100 nF (ESP32 dalı), 3 pinli servo kablosu, (opsiyonel) 2 A polyswitch, yüksek kesitli (≥ 0.5 mm²) servo güç kabloları.
- **Topoloji:**
  ```
  LM2596 OUT+ ─┬── (kısa kablo) ── ESP32 VIN   [+470µF/100nF yakınında]
               └── (kısa, kalın)  ── Servo V+    [+1000µF/100nF servo yakınında]
  LM2596 OUT- ─┬── ESP32 GND
               └── Servo GND
  ```
  Ayrım noktası LM2596 çıkış terminalleri (veya klemens) olmalı; servo kablosu ESP32 pininden türetilmemeli.
- **Yapılacaklar:**
  - Servo bağlı **değilken** çıkışın 5.00 V olduğunu multimetreyle yeniden doğrula.
  - İki dalı klemens/lehim ile ayrıştır, kondansatörleri hedef bileşenlerin yakınına koy.
  - Servo sinyal → ESP32 GPIO 4; GND'ler ortak.
  - Servoyu bağladıktan sonra hareket sırasında 5 V çıkışını ve ESP32 VIN'ini multimetre/osiloskopla izle.
- **Risk:** Ortak regülatör nedeniyle servo yoğun yük veya kilitlenmede 5 V hattını düşürebilir (brown-out). Reset görülürse: servo akımını düşür (açı sınırlarını daralt, slew rate ekle), kondansatörü büyüt veya ileride servo için ikinci bir regülatör al.
- **Kabul:** Servo yüksüz ve direksiyona bağlı haldeyken ESP32 resetlenmiyor, 5 V hattı düşümü ölçüldü ve kabul edilebilir (≈ ≤ 0.2 V).

### B. Pin Tanımı ve Dokümantasyon (CS)
**Amaç:** Servo pinini tek kaynak kuralına uygun şekilde tanımlamak.
- `pins.h` içine servo sinyal pinini ekle (GPIO 4, strapping pini değil).
- `docs/pinout.md`'de pini "Rezerve"den "Kullanılan pinler"e taşı.
- **Kabul:** Servo pini yalnızca `pins.h`'ta tanımlı, `.cpp` dosyalarında sabit numara yok.

### C. Servo Sürücü Yazılımı (CS)
**Amaç:** Açı → PWM dönüşümünü yapan, bağımsız bir modül.
**Gereksinimler:**
- **SRP:** Modül yalnızca açı/PWM işinden sorumlu; direksiyon veya araç mantığı içermez.
- ESP32 donanımsal LEDC ile 50 Hz PWM üretir.
- Motor sürücü kanalları (0, 1) ile **çakışmaz**. İpucu: LEDC kanalları zamanlayıcıları çiftler halinde paylaşır; servo için kanal 2-3 ayrı zamanlayıcıdadır.
- Açı sınırlama (clamp), merkez trim ve darbe genişliği aralığı **parametrik** olmalı (kalibrasyonda değişecek).
- Başlangıçta servo **merkeze** alınmalı.
- İlk testlerde güvenli aralık (≈1000–2000 µs) kullanılmalı; tam aralığa sonra çıkılmalı.
- **Kabul:** Servo komutla merkeze ve belirlenen açılara gidiyor, sınırları aşmıyor, titremiyor.

### D. Mekanik Montaj (EE / Mekanik)
**Amaç:** Servoyu ve direksiyon mekanizmasını şaseye doğru şekilde kurmak.
- 3D baskı parçaları: ön aks, rot kolu, servo tutucu. Tutucu MG995 gövde ölçülerine (~40.7 × 19.7 × 42.9 mm) uymalı.
- Servo kolu: servo merkezdeyken ve tekerlekler düz iken tak. Dişli atlaması ≈ 14°/diş, ince ayar yazılımdaki trim ile yapılır.
- Rot kolu bağlantı noktası, servo açısı → tekerlek açısı oranını belirler.
- PLA jant ve TPU lastikleri tak.
- **Dikkat:** Zorlanma sesi duyarsan güç hattını kes, mekanik sınırı yazılımla daralt.
- **Kabul:** Tekerlekler servo ±30° bandında çarpma/zorlanma olmadan dönüyor.

### E. Kalibrasyon ve Ackermann Ölçümleri (EE + CS)
**Amaç:** Gerçek mekanik davranışı ölçüp yazılım parametrelerine dönüştürmek.
- **Ölçülecekler** (`docs/vehicle_params.md`'ye kaydedilecek):
  - `T`: ön tekerlek iz genişliği (mm)
  - `L`: dingil arası mesafe (mm)
  - Maksimum fiziksel direksiyon açısı (sol/sağ)
  - Servo açısı → sol/sağ tekerlek açısı tablosu (ör. her 10°'de)
- **Kalibre edilecek parametreler:** merkez trim, min/max darbe genişliği, servo/tekerlek açı oranı.
- **Ackermann kontrolü:** `cot(δ_dış) − cot(δ_iç) = T / L` ile ölçümleri karşılaştır. Hedef: iç tekerlek dış tekerlekten **daha büyük** açıda dönsün. Hobi ölçeğinde yaklaşık Ackermann yeterlidir.
- **Kabul:** Ölçüm tablosu hazır, trim ayarlı (direksiyon merkezde araç düz gidiyor), Ackermann sapması yorumlanmış.

### F. Direksiyon Katmanı (CS)
**Amaç:** Tekerlek açısı (-30°..+30°) komutunu servo açısına çeviren ince katman.
**Gereksinimler:**
- Servo sürücüyü dışarıdan alır (dependency injection), kendisi üretmez.
- Oran, trim ve limitler kalibrasyon sonucundan gelen sabitlerdir.
- Aralık dışı komutlar sınırlanır.
- **Kabul:** Tek bir çağrıyla direksiyon istenen açıya gidiyor, merkeze dönebiliyor.

### G. Araç Kontrol Katmanı (CS)
**Amaç:** Motor ve direksiyonu tek arayüzden yönetmek (Hafta 3 Drive-by-Wire hazırlığı).
**Gereksinimler:**
- Tek arayüz: throttle (-100..+100 %) ve steering (-30..+30°).
- Acil durdurma: motorlar durur, direksiyon ortalanır.
- Serial üzerinden basit, satır tabanlı bir komut protokolü (sür / dur / ortala). Protokolü ekip tasarlar; Hafta 3'te BLE komutlarıyla aynı yapı kullanılabilmeli.
- **Kabul:** Serial komutuyla motor ve direksiyon birlikte çalışıyor, acil durdurma anında etkili.

### H. Test ve Doğrulama (Ortak)
- **Güç ölçümü:** Servo boşta/hareket/kilitlenme akımı; hareket sırasında 5 V ve 3V3 hatlarında düşüş var mı?
- **Dayanıklılık:** ~10 dk sürekli komutla servo ve LM2596 ısınması kontrol edilir.
- **Kablo düzeni:** Zip-tie ile sabitlenir, hareketli parçalara temas etmemeli.
- Sürüş testleri önce tekerlekler havada, sonra yerde yapılmalı.
- Kod `git` ile commit'lenir, hafta sonunda etiketlenir (ör. `v0.2-week2`).

---

## 🧩 5. Ek Konular (Opsiyonel)

| Konu | Neden Faydalı? | Zorluk |
|---|---|---|
| Servo hız sınırlama (slew rate) | Ani direksiyon darbesi ve akım tepelerini azaltır | Kolay |
| Fail-safe ön versiyonu (komut gelmezse dur) | Hafta 3 watchdog'un prototipi | Orta |
| Batarya voltaj ölçümü (GPIO 34, direnç bölücü) | Pil boşalmadan uyarı | Orta |
| Serial CSV kayıt (zaman, throttle, steering) | Telemetri altyapısı | Kolay |
| Mekanik revizyon (rot kolu delik seçenekleri, tutucu sağlamlaştırma) | Oran ve dayanıklılık | Orta |
| Direksiyon geri bildirimi | Kapalı çevrim doğrulama | Zor |

> **Öneri:** Hafta 3'ün güvenlik gereksinimleriyle örtüştüğü için önce *slew rate* ve *fail-safe ön versiyonu*.

---

## 📁 6. Hedeflenen Proje Yapısı

```
software/
├── include/  → pins.h (+servo pini), motor_driver.h, servo_driver.h, steering.h, vehicle_controller.h
└── src/      → main.cpp, motor_driver.cpp, servo_driver.cpp, steering.cpp, vehicle_controller.cpp
docs/
├── pinout.md          → servo pini güncellenecek
└── vehicle_params.md  → T, L, açı tablosu (yeni)
```
*(Dosya adları öneridir; ekip değiştirebilir.)*

---

## 🎯 7. Hafta 2 Başarı Kriterleri (Definition of Done)

Aşağıdakilerin tümü tamamlanmadan Hafta 3'e (BLE + Python Backend + Fail-safe) geçilmez:

- [ ] **Servo güç güvenliği:** Servo hattı 6.00 V, ESP32'den bağımsız; hareket sırasında ESP32 resetlenmiyor.
- [ ] **Ortak şase:** Servo GND'si ESP32 GND'sine bağlı, servo titremiyor.
- [ ] **Servo sürücü:** Pin yalnızca `pins.h`'tan geliyor, açı sınırlaması çalışıyor.
- [ ] **Merkez doğruluğu:** Direksiyon merkezdeyken araç düz gidiyor.
- [ ] **Direksiyon aralığı:** ±30° tekerlek açısına zorlanmadan ulaşılıyor.
- [ ] **Ackermann doğrulaması:** `T`, `L` ve açı tablosu `vehicle_params.md`'de; iç tekerlek açısı dış tekerlekten büyük.
- [ ] **Birleşik kontrol:** Motor ve direksiyon tek komutla birlikte çalışıyor.
- [ ] **Acil durdurma:** Motorlar duruyor, direksiyon ortalanıyor.
- [ ] **Dayanıklılık:** ~10 dk testte servo ve regülatör aşırı ısınmıyor, dişlide zorlanma sesi yok.

---

## 🚀 8. Sonraki Adım: Hafta 3'e Hazırlık

Araç tek arayüzle sürülüp yönlendirilebilir hale geldiğinde:
1. ESP32'de BLE GATT Server (Throttle, Steering, Heartbeat).
2. Araç kontrol katmanının BLE komutlarına bağlanması (Drive-by-Wire).
3. FreeRTOS ile BLE ve Motor Control task ayrımı.
4. Fail-safe / Watchdog: bağlantı kopunca motor dur, direksiyon ortala.
5. Python `bleak` + WebSocket + tarayıcı joystick dashboard.
