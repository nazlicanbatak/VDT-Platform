#pragma once
#include <Arduino.h>
#include "motor_driver.h"
#include "steering.h"

// Arac Kontrol Unitesi (VCU/ECU: Motor ve direksiyonu tek merkezden yonetir)
class VehicleController {
public:
    // Kurucu fonksiyon: motor ve direksiyon alt sistemlerini referans olarak alir (SOLID)
    VehicleController(MotorDriver& motors, Steering& steering);

    // Tum alt sistemleri baslatir ve araci guvenli bekleme konumuna alir
    void begin();

    // Araci sur: Gaz yuzdesi (-100 ile +100 arasi) ve Direksiyon acisi (-29 ile +29 derece arasi)
    void drive(int throttlePct, float steeringDeg);

    // Acil durum durdurmasi: motorlari aninda keser ve direksiyonu duzeltir
    void emergencyStop();

    // Guncel gaz yuzdesini dondurur
    int getThrottle() const;

    // Guncel tekerlek acisini dondurur
    float getSteering() const;

private:
    MotorDriver& _motors;    // Motor surucu referansi
    Steering&    _steering;  // Direksiyon katmani referansi

    int   _currentThrottle;  // Anlik gaz yuzdesi (-100..+100)
    float _currentSteering;  // Anlik tekerlek acisi (-29..+29)

    // Gaz yuzdesini (-100..+100) TB6612 PWM degerine (0..255) donusturur
    uint8_t throttleToPwm(int throttlePct) const;
};
