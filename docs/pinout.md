# ESP32 DevKit V1 Pinout

> Kod tarafı: `software/include/pins.h`. Burayı değiştirirsen orayı da değiştir.
> Son KiCad doğrulaması: _henüz yapılmadı_

## Kullanılan pinler

| Modül | Sinyal | ESP32 GPIO | Tür | Motor / Fonksiyon | Hafta | Durum |
|---|---|---|---|---|---|---|
| TB6612FNG | PWMA | 25 | PWM | Sağ Motor Hız (PWM) | 1 | Kullanımda |
| TB6612FNG | AIN1 | 26 | GPIO | Sağ Motor Yön 1 | 1 | Kullanımda |
| TB6612FNG | AIN2 | 27 | GPIO | Sağ Motor Yön 2 | 1 | Kullanımda |
| TB6612FNG | PWMB | 14 | PWM | Sol Motor Hız (PWM) | 1 | Kullanımda |
| TB6612FNG | BIN1 | 32 | GPIO | Sol Motor Yön 1 (12 strapping, 16 yok → 32) | 1 | Kullanımda |
| TB6612FNG | BIN2 | 13 | GPIO | Sol Motor Yön 2 | 1 | Kullanımda |
| TB6612FNG | STBY | 33 | GPIO | Sürücü Standby/Aktif | 1 | Kullanımda |

## Rezerve pinler (henüz bağlı değil)

| Modül | Sinyal | ESP32 GPIO | Hafta/Faz |
|---|---|---|---|
| SG90 servo | Sinyal | 4 | 2 |
| MPU6050 | SDA / SCL | 21 / 22 | 4 |
| Pil ölçümü | ADC | 34 | İsteğe bağlı |
| Encoder | Sol / Sağ | 35 / 39 | Faz 6 |
| MCP2515 CAN | SCK / MISO / MOSI / CS / INT | 18 / 19 / 23 / 5 / 17 | Faz 5 |

## Kaçınılacak pinler

- Strapping: 0, 2, 5 (CS hariç), 12, 15
- Flash: 6-11
- Sadece giriş: 34, 35, 36, 39
- USB seri: 1, 3
