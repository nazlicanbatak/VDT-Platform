#pragma once
#include <Arduino.h>

// Motor secimi icin tanimlar (Sol / Sag)
enum MotorId {
    MOTOR_LEFT,
    MOTOR_RIGHT
};

// Motor donus yonleri
enum MotorDirection {
    DIR_FORWARD,   // Ileri donus
    DIR_BACKWARD,  // Geri donus
    DIR_STOP       // Serbest durus (coast)
};

// Bir motorun baglanti pinleri
struct MotorPins {
    uint8_t pwm;   // PWM hiz kontrol pini
    uint8_t in1;   // Yon kontrol pini 1
    uint8_t in2;   // Yon kontrol pini 2
};

// TB6612FNG motor surucu sinifi (SRP: Yalnizca motor donanim surusunden sorumludur)
class MotorDriver {
public:
    // Kurucu fonksiyon: pin atamalarini alir
    MotorDriver(MotorPins leftPins, MotorPins rightPins, uint8_t stbyPin);

    // Donanim pinlerini cikis olarak ayarlar ve PWM kanallarini baslatir
    void begin();

    // Belirli bir motorun yon ve hizini (0-255 arasi) ayarlar
    void setMotor(MotorId id, MotorDirection dir, uint8_t speed);

    // Tum motorlari guvenli sekilde aninda durdurur
    void stopAll();

    // Kablo ters baglanmissa yazilimsal olarak motor yonunu tersine cevirir
    void invertMotor(MotorId id, bool invert);

private:
    MotorPins _left;
    MotorPins _right;
    uint8_t _stby;

    bool _leftInverted;
    bool _rightInverted;

    // Donanima fiilen sinyal yazan alt seviye fonksiyon
    void writeMotorHardware(const MotorPins& pins, uint8_t pwmChannel, MotorDirection dir, uint8_t speed, bool isInverted);
};
