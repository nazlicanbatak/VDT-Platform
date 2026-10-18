#pragma once
#include <Arduino.h>

// MG995 ve standart RC servolar icin surucu sinifi (SRP: Yalnizca aci -> PWM donusumu)
class ServoDriver {
public:
    // Kurucu fonksiyon: yalnizca sinyal pinini alir (Diger sabitler cpp dosyasindadir)
    explicit ServoDriver(uint8_t pin);

    // Donanim PWM kanalini (50 Hz, 16-bit) baslatir ve servoyu merkeze alir
    void begin();

    // Servoyu belirli bir aciya konumlandirir (0.0 ile 180.0 derece arasi)
    void setAngle(float angleDeg);

    // Servoyu guvenli merkez konumuna (90 derece) getirir
    void center();

    // Mekanik sikismalari onlemek icin yazilimsal guvenlik sinirlari koyar
    void setLimits(float minDeg, float maxDeg);

    // Mevcut aci degerini dondurur
    float getAngle() const;

private:
    uint8_t _pin;
    float   _minDeg;
    float   _maxDeg;
    float   _currentAngle;

    // Aci degerini (derece) LEDC 16-bit duty degerine donusturur
    uint32_t angleToDuty(float angleDeg) const;
};
