# ESP32 DevKit V1 Pinout

> Kod tarafı: `firmware/include/pins.h`. Burayı değiştirirsen orayı da değiştir.
> Son KiCad doğrulaması: _henüz yapılmadı_

## Kullanılan pinler

| Modül | Sinyal | ESP32 GPIO | Tür | Hafta | Durum |
|---|---|---|---|---|---|
| TB6612FNG | PWMA | 25 | PWM | 1 | Kullanımda |
| TB6612FNG | AIN1 | 26 | GPIO | 1 | Kullanımda |
| TB6612FNG | AIN2 | 27 | GPIO | 1 | Kullanımda |
| TB6612FNG | PWMB | 14 | PWM | 1 | Kullanımda |
| TB6612FNG | BIN1 | 32 | GPIO | 1 | Kullanımda (12 strapping, 16 yok → 32) |
| TB6612FNG | BIN2 | 13 | GPIO | 1 | Kullanımda |
| TB6612FNG | STBY | 33 | GPIO | 1 | Kullanımda |

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
