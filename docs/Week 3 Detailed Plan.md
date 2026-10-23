# 📋 VDT Platform — Hafta 3 Planı (Wi-Fi & Mobil Web Dashboard)

> **Mevcut Durum:** Hafta 2 tamamlandı. Araç, tek arayüzle (`VehicleController::drive` / `emergencyStop`) sürülüp yönlendirilebiliyor; direksiyon tablo tabanlı (±29°) ve güç/ısınma testlerinden geçti.  
> **Ekip:** 1× Bilgisayar Mühendisliği (CS) + 1× Elektrik-Elektronik Mühendisliği (EE)  
> **Yeni Mimari Kararı:** Dış mekan (bahçe/saha) sürüş testlerinde laptop taşıma zorunluluğunu ortadan kaldırmak, menzili artırmak ve iPhone Safari kısıtlamalarını aşmak için sistem **bağımsız Wi-Fi Access Point + Gömülü WebSocket Web Sunucusu** olarak güncellenmiştir.  
> **Hedef:** Aracı **bağımsız Wi-Fi** (`VDT-Car`) üzerinden, doğrudan **telefon tarayıcısından** (Safari/Chrome dokunmatik joystick) sürmek; bağlantı koptuğunda aracın **kendiliğinden güvenli biçimde durmasını** (fail-safe) sağlamak.  

---

## 🧭 1. Hedef Mimari

```
┌────────────────────────────────────────────────────────┐
│  Telefon / Mobil Tarayıcı (Safari / Chrome)            │
│  HTML5 + CSS3 + Dokunmatik Sanal Joystick              │
│  (Sıfır ek uygulama, sıfır bilgisayar ihtiyacı)        │
└───────────────────────────┬────────────────────────────┘
                            │ WebSocket (ws://192.168.4.1/ws)
                            │ HTTP GET (http://192.168.4.1/)
┌───────────────────────────▼────────────────────────────┐
│  ESP32 — ECU & Wi-Fi Access Point ("VDT-Car")          │
│  ┌────────────────────────┐   Queue   ┌──────────────┐ │
│  │ Wi-Fi + WebSocket Görevi│ ────────▶ │ Motor Görevi │ │
│  │ (Core 0)               │           │ (Core 1)     │ │
│  └────────────────────────┘           │ VehicleCtrl  │ │
│  ┌────────────────────────┐           └──────────────┘ │
│  │ Watchdog / Fail-Safe   │ ── timeout ──▶ Durdur      │
│  │ (500 ms eşik)          │                            │
│  └────────────────────────┘                            │
└────────────────────────────────────────────────────────┘
```

**Temel ilkeler:** 
1. **Tek Sorumluluk (SOLID):** Wi-Fi/WebSocket geri çağırması motorlara **doğrudan dokunmaz**. Gelen komut `commandQueue`'ya bırakılır; motor ve servoyu yalnızca Core 1'deki `vehicleTask` sürer.
2. **Tam Hareket Özgürlüğü (Mobilite):** ESP32 pili takıldığı anda kendi Wi-Fi ağını açar; sokakta, parkta hiçbir internete veya bilgisayara ihtiyaç duyulmaz.
3. **Telemetriye Hazır Çift Yönlü İletişim:** WebSocket kanalı iki yönlüdür. Tarayıcı komut gönderirken, ESP32 de Hafta 4'te sensör ve batarya verilerini anlık olarak geri iletir.

---

## 🧱 2. Konu Haritası ve Bağımlılıklar

```
[A] Protokol & JSON Formatı ──┬──▶ [C] Wi-Fi AP + Web/WebSocket Sunucu ──┐
                              │                                            ├──▶ [D] Fail-Safe & Watchdog ──┐
[B] FreeRTOS Görev Mimarisi ──┘                                            │                                │
                                                                           ▼                                ▼
                                                                 [E] Mobil Dashboard (Joystick) ──▶ [F] Saha & Menzil Testi
```

| Konu | Sorumlu | Bağımlı Olduğu |
|---|---|---|
| A. Protokol ve JSON Mesaj Formatı | CS | — |
| B. FreeRTOS Görev ve Kuyruk Mimarisi | CS | — |
| C. ESP32 Wi-Fi AP + Web/WebSocket Sunucusu | CS | A, B |
| D. Fail-Safe ve Watchdog Emniyet Kilidi | CS (+EE doğrulama) | B, C |
| E. Mobil Uyumlu Dokunmatik Dashboard (HTML/CSS/JS) | CS | A, C |
| F. Saha, Menzil ve Güç Testleri | Ortak (EE ağırlıklı) | D, E |

---

## ⚡ 3. Kritik Mühendislik Kuralları

1. **Güvenlik her şeyden önce (Fail-Safe):** WebSocket bağlantısı koptuğunda veya parmak ekrandan çekildiğinde araç **500 ms içinde kendiliğinden durmalı** ve direksiyonu ortalamalıdır.
2. **Test sırası:** İlk sürüş testleri mutlaka **tekerlekler havada** denenir, mekanik doğrulandıktan sonra yere inilir.
3. **Brown-out ve Akım Takibi:** Wi-Fi radyosu devreye girdiğinde anlık akım çekişi ~200-250 mA civarına çıkar. 5V regülatör (LM2596) ve kondansatörlerin gerilimi sabit tuttuğu multimetre ile teyit edilmelidir.
4. **Hafif ve Hızlı Web Sunumu:** Web dosyaları (HTML/CSS/JS) ESP32 flash belleğinden (PROGMEM / SPIFFS) tek parça halinde, sıfır harici internet bağımlılığıyla (CDN kütüphanesi olmadan) sunulmalıdır.
5. **Kabloyla Debug Açık:** USB Serial logları çalışmaya devam eder; bağlantı ve komut teşhisi buradan izlenebilir.

---

## 🧱 4. İş Paketleri

### A. Protokol ve JSON Formatı (CS)
**Amaç:** Telefon tarayıcısı ile ESP32 arasındaki hafif veri formatının belirlenmesi.
- **Komut Formatı:** `{"throttle": 35, "steering": 12}` (Gaz: -100..+100 %, Direksiyon: -29..+29°)
- **Heartbeat:** `{"type": "ping"}` veya periyodik komut akışı (20–30 Hz)
- **Acil Durdurma:** `{"emergency": true}`

### B. FreeRTOS Görev Mimarisi (CS)
**Amaç:** Ağ iletişimi ile motor kontrolünü birbirinden izole etmek.
- **Core 0:** Wi-Fi bağlantısı, HTTP ve WebSocket sunucu olayları.
- **Core 1:** `vehicleTask` — `commandQueue`'dan gelen veriyi alıp `VehicleController`'a ileten tek motor sürücü görev.
- **Kuyruk Davranışı:** `xQueueOverwrite` ile her zaman en taze komut işlenir, gecikme/kuyruk şişmesi önlenir.

### C. ESP32 Wi-Fi AP + Web/WebSocket Sunucusu (CS)
**Amaç:** ESP32'yi `VDT-Car` SSID ile Access Point yapmak ve port 80 / WebSocket sunucusunu ayağa kaldırmak.
- IP adresi: `192.168.4.1`
- `GET /`: `dashboard/index.html` sayfasını döndürür.
- `GET /ws`: WebSocket kanalı. Gelen mesajları JSON olarak ayrıştırıp `commandQueue`'ya gönderir.

### D. Fail-Safe ve Watchdog Emniyet Kilidi (CS & EE)
**Amaç:** Telefon menzil dışına çıktığında veya tarayıcı kapandığında aracı durdurmak.
- WebSocket `onClose` olayında anında acil durdurma.
- 500 ms boyunca yeni komut gelmezse `emergencyStop()`.

### E. Mobil Uyumlu Dokunmatik Dashboard (HTML/CSS/JS) (CS)
**Amaç:** iPhone / Android Safari ve Chrome'da çalışan modern, şık bir sürüş paneli.
- Sol joystick: İleri / Geri Gaz (bırakınca %0 merkez).
- Sağ joystick: Sol / Sağ Direksiyon (bırakınca 0° merkez).
- Büyük kırmızı Acil Durdurma butonu.
- Canlı bağlantı durumu göstergesi (yeşil/kırmızı).

### F. Saha, Menzil ve Güç Testleri (Ortak)
**Amaç:** Sistemin dış ortamda dayanıklılığını doğrulamak.
- Açık alanda Wi-Fi çekim mesafesi ölçümü (hedef: > 30 metre).
- Li-Po pil ve 5V hattında gerilim düşümü (brown-out) gözlemi.
- Kesintisiz 10 dakika sürüş testi.

---

## 📁 5. Hedeflenen Proje Yapısı

```
VDT-Platform/
├── software/
│   ├── include/
│   │   ├── pins.h
│   │   ├── protocol.h          # Ortak veri tipleri ve limitler
│   │   ├── wifi_service.h      # Wi-Fi AP ve Web/WebSocket sunucusu
│   │   ├── vehicle_controller.h
│   │   └── ...
│   └── src/
│       ├── main.cpp            # FreeRTOS görevleri ve başlatma
│       ├── wifi_service.cpp    # Wi-Fi ve WebSocket yönetimi
│       └── ...
└── docs/
    ├── ROADMAP.md
    └── Week 3 Detailed Plan.md
```

---

## 🎯 6. Hafta 3 Başarı Kriterleri (Definition of Done)

- [ ] ESP32 `VDT-Car` Wi-Fi yayını yapıyor, iPhone ile şifresiz/şifreli bağlanılabiliyor.
- [ ] iPhone Safari'de `192.168.4.1` açıldığında dokunmatik joystick dashboard yükleniyor.
- [ ] Sanal joystick ile araç ileri/geri ve sağa/sola kesintisiz sürülüyor.
- [ ] Joystick bırakıldığında araç duruyor ve direksiyon ortalanıyor.
- [ ] Safari kapatıldığında veya Wi-Fi menzili dışına çıkıldığında araç 500 ms içinde duruyor (Fail-safe).
- [ ] Saha testinde açık alanda > 30 metre menzil doğrulandı.
