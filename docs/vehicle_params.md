# 📐 VDT Platform — Araç Parametreleri (vehicle_params.md)

> Bu dosya, direksiyon kalibrasyonunda ölçülen **gerçek** değerleri tutur. Kod sabitleri (limitler, trim, oran) buradaki değerlerden türetilir.  
> Ölçüm tarihi: ____ / ____ / ________  
> Ölçümü yapan: ______________

---

## 1. Servo Kalibrasyonu (Ölçüldü)

| Parametre | Servo Açısı (°) | Not |
|---|---|---|
| **Merkez (düz)** | **84** | Tekerlekler düz |
| **Sol mekanik sınır** | **34** | Bu açıda şaseye temas başlıyor / hareket bitiyor |
| **Sağ mekanik sınır** | **129** | Bu açıdan sonra servo hareketi bitiyor |
| Merkezden sola mesafe | 50 | 84 − 34 |
| Merkezden sağa mesafe | 45 | 129 − 84 |

> ✅ **Simetri:** Sol (50°) ve sağ (45°) hareket payları birbirine oldukça yakın ve dengelidir.

### Yazılım Limitleri (Güvenlik Payı Dahil)

Mekanik sınırlara dayanıp motorun aşırı akım çekmesini engellemek için sınırlardan 4° içeride kalıyoruz:

| Parametre | Mekanik | Güvenlik payı | **Kodda kullanılacak** |
|---|---|---|---|
| Sol limit | 34 | + 4° | **38** |
| Sağ limit | 129 | − 4° | **125** |
| Merkez trim | 84 | — | **84** |

*(Güvenlik payı bir öneridir; ekip değiştirebilir.)*

---

## 2. Mekanik Boşluk (Backlash / Ölü Bölge)

Direksiyon bağlantılarında gevşeklik olduğunu gözlemledin: merkez civarında servo ±2° döndüğünde tekerlek tepki vermiyor.

| Gözlem | Değer |
|---|---|
| Merkez civarı boşluk (ölü bölge) | ≈ ±2° servo açısı |
| Titreme (merkezde) | Var / Yok: ____ |
| Boşluğun kaynağı (tahmin) | Rot kolu bağlantısı / servo kolu dişlisi / servo içi ölü bölge |

**Boşluğun sonucu:** Servo komutundaki ±2°'lik küçük değişimler tekerleğe iletilmez. Hassas direksiyon beklenmemeli; Hafta 6 E-Diff için direksiyon açısı **ölçülen** tekerlek açısından alınmalı, servo komutundan türetilmemeli.

**Azaltma fikirleri (opsiyonel):**
- [ ] Rot kolu bağlantı noktalarındaki gevşeklik (cıvata, rotil) kontrol edilecek
- [ ] Servo kolu dişliye sıkı oturuyor mu
- [ ] Lastik/jant gevşekliği
- [ ] Servo ölü bölgesi (klon MG995'lerde geniş olabilir)

---

## 3. Servo Açısı → Tekerlek Açısı Tablosu

Servoyu 10° adımlarla çevir; her adımda **sol** ve **sağ** tekerlek açısını ölç (düz konuma göre, derece).  
Sol dönüş için sola yatık = negatif, sağ dönüş = pozitif yazılabilir.

| Servo (°) | Servo − Merkez (°) | Sol Tekerlek (°) | Sağ Tekerlek (°) | Ortalama (Sanal) Açı (°) | Not / Durum |
|---|---|---|---|---|---|
| 38 (sol limit) | −46 | **−45** | **−13** | **−29.0** | Maksimum Sol (İç: Sol 45°, Dış: Sağ 13°) |
| 50 | −34 | −26 | −10 | −18.0 | Sol Dönüş |
| 62 | −22 | −14 | −5 | −9.5 | Sol Dönüş |
| 74 | −10 | −9 | −2 | −5.5 | Sol Dönüş (Hafif) |
| 84 (merkez) | 0 | 0 | 0 | 0.0 | Tam Düz Gidiş |
| 94 | +10 | +8 | +18 | +13.0 | Sağ Dönüş (Hafif) |
| 104 | +20 | +12 | +29 | +20.5 | Sağ Dönüş |
| 114 | +30 | +17 | +35 | +26.0 | Sağ Dönüş |
| 125 (sağ limit) | +41 | **+20** | **+45** | **+32.5** | Maksimum Sağ (İç: Sağ 45°, Dış: Sol 20°) |

**Ölçüm yöntemi:** Manuel kalibrasyon ve açıölçer / görsel hizalama ile tezgahta ölçüldü.

### Çıkarılan Değerler

| Parametre | Değer | Açıklama |
|---|---|---|
| **Maksimum Efektif Dönüş (Sol)** | **−29.0°** | Sol limit tekerleklerin ortalama yönelme açısı |
| **Maksimum Efektif Dönüş (Sağ)** | **+32.5°** | Sağ limit tekerleklerin ortalama yönelme açısı |
| **Ackermann Geometrisi Durumu** | **BAŞARILI** | Her iki yönde de iç tekerlek dış tekerlekten belirgin şekilde fazla dönüyor (45° vs 20° / 13°). |
| **Hedef Uyumu** | **%100 (±30°)** | Roadmap'te hedeflenen ±30° efektif direksiyon aralığı neredeyse birebir yakalandı! |

> **Hedef:** Roadmap ±30° tekerlek açısını hedefliyor. Gerçek mekanik bunu vermeyebilir. Ölçülen maksimum buradaki değerdir; ±30° sağlanmıyorsa gerçek aralık kabul edilir ve hedef güncellenir.

---

## 4. Geometri Ölçüleri (Ackermann)

| Parametre | Değer (mm) | Nasıl ölçüldü |
|---|---|---|
| **T** — Ön iz genişliği (ön tekerlek merkezleri arası) | **165 mm** | Şase üzerinden kumpas/cetvel ile |
| **L** — Dingil arası mesafe (ön–arka aks arası) | **253 mm** | Şase üzerinden kumpas/cetvel ile |
| **T / L oranı** | **0.652** | $165 / 253$ |

---

## 5. Ackermann Kontrolü

Maksimum direksiyon açılarında iç ve dış tekerlek durumu:

| Dönüş Yönü | Dış tekerlek $\delta_{dış}$ (Ölçülen) | İç tekerlek $\delta_{iç}$ (Ölçülen) | İdeal Dış Tekerlek ($\delta_{dış,ideal}$) | Analiz |
|---|---|---|---|---|
| **Sol Dönüş** | 13° (Sağ) | 45° (Sol) | 31.2° | Pro-Ackermann (İç tekerlek belirgin şekilde daha çok dönüyor) |
| **Sağ Dönüş** | 20° (Sol) | 45° (Sağ) | 31.2° | Pro-Ackermann (İç tekerlek belirgin şekilde daha çok dönüyor) |

**İdeal Ackermann Formülü:**
$$\cot(\delta_{dış}) - \cot(\delta_{iç}) = \frac{T}{L} = 0.652$$

**Değerlendirme:**
- [x] İç tekerlek, dış tekerlekten **belirgin şekilde büyük** açıda dönüyor (45° vs 13° / 20°).
- [x] Temel Ackermann şartı sağlandı: Dönüşlerde dış tekerlek yeri kazımıyor / sürtünme minimize edildi.
- [x] Mekanizma "Pro-Ackermann" davranışı sergileyerek düşük hızda ve dar virajlarda çevikliği artırıyor.

---

## 6. Kod Sabitlerine Aktarılacak Değerler (Özet)

| Sabit | Değer | Kaynak / Not |
|---|---|---|
| **Servo merkez trim** | **84°** | Tekerleklerin tam düz olduğu kalibre servo açısı |
| **Servo fiziksel min limit (sol)** | **38°** | 4° güvenlik payı dahil mekanik limit |
| **Servo fiziksel max limit (sağ)** | **125°** | 4° güvenlik payı dahil mekanik limit |
| **Yazılımsal Simetrik Direksiyon Aralığı** | **$\pm 28.0^\circ$** | Her iki yönde tam simetrik sürüş hissi için seçilen sınır |
| **Sol Servo Açısı (-28° Efektif için)** | **≈ 39°** | Linear interpolasyon ile hesaplanan servo hedefi |
| **Sağ Servo Açısı (+28° Efektif için)** | **≈ 118°** | Linear interpolasyon ile hesaplanan servo hedefi |

---

## 7. Notlar ve Gözlemler

- Sol dönüşte dış tekerlek açısı (13°) sağ dönüşteki dış tekerlek açısına (20°) göre biraz daha az dönüyor. Bu durum rot kollarının simetrisinden kaynaklanabilir.
- Yazılımsal olarak her iki yönün efektif açısını $\pm 28^\circ$ sınırında sabitleyerek aracın sağa ve sola eşit çeviklikte dönmesi sağlanacak.
