#pragma once
#include <Arduino.h>
#include "servo_driver.h"

// Direksiyon katmani (SRP: Tekerlek acisini servo acisina cevirir)
class Steering {
public:
    // Kurucu fonksiyon: ServoDriver nesnesini referans olarak alir (kopyalamaz)
    explicit Steering(ServoDriver& servoDriver);

    // Servoyu baslatir, mekanik limitleri uygular ve direksiyonu ortalar
    void begin();

    // Hedef tekerlek acisini (derece, -28 ile +28 arasi) uygular
    void setAngle(float wheelDeg);

    // Direksiyonu duz konuma (0 derece) getirir
    void center();

    // Son komut verilen tekerlek acisini dondurur
    float getAngle() const;

private:
    // Tekerlek acisini tabloya gore servo acisina cevirir
    float calculateServoDeg(float wheelDeg) const;

    ServoDriver& _servo;   // Servo surucuye referans (disaridaki nesne kullanilir)
    float _currentAngle;   // Son komut verilen tekerlek acisi
};

