#include "servo_driver.h"

// Servo PWM ve donanim yapilandirmasi (MotorDriver yapisi ile ayni mantik)
static const uint32_t SERVO_FREQ    = 50;     // 50 Hz = 20 ms periyot
static const uint8_t  SERVO_RES     = 16;     // 16-bit cozunurluk (0 - 65535 arasi)
static const uint8_t  PWM_CH_SERVO  = 2;      // Motorlar 0 ve 1 kullandigi icin kanal 2 (Timer 1)
static const uint32_t PERIOD_US     = 20000;  // 20 ms = 20000 mikrosaniye
static const uint16_t MIN_PULSE_US  = 500;    // MG995 0 derece darbe suresi (us)
static const uint16_t MAX_PULSE_US  = 2500;   // MG995 180 derece darbe suresi (us)

// Kurucu fonksiyon: yalnizca sinyal pinini alir
ServoDriver::ServoDriver(uint8_t pin)
    : _pin(pin),
      _minDeg(0.0f),
      _maxDeg(180.0f),
      _currentAngle(90.0f) {}

// LEDC kanalini baslatir, pine baglar ve servoyu guvenli merkeze alir
void ServoDriver::begin() {
    ledcSetup(PWM_CH_SERVO, SERVO_FREQ, SERVO_RES);
    ledcAttachPin(_pin, PWM_CH_SERVO);
    center();
}

// Mekanik zorlanmalari engellemek icin aci sinirlarini belirler
void ServoDriver::setLimits(float minDeg, float maxDeg) {
    _minDeg = minDeg;
    _maxDeg = maxDeg;
}

// Belirtilen aci degerine doner (sinirlar disina cikamaz)
void ServoDriver::setAngle(float angleDeg) {
    // Guvenlik: Acıyı belirlenen min/max limitleri arasinda tut
    angleDeg = constrain(angleDeg, _minDeg, _maxDeg);
    _currentAngle = angleDeg;

    uint32_t duty = angleToDuty(angleDeg);
    ledcWrite(PWM_CH_SERVO, duty);
}

// Servoyu 90 derece merkez konumuna getirir
void ServoDriver::center() {
    setAngle(90.0f);
}

// Anlik servo acisini dondurur
float ServoDriver::getAngle() const {
    return _currentAngle;
}

// Aci (0-180) -> Darbe Suresi (us) -> LEDC Duty (0-65535) donusumu
uint32_t ServoDriver::angleToDuty(float angleDeg) const {
    // 1. Acıyı darbe genisligine (us) oranla
    float pulseUs = MIN_PULSE_US + (angleDeg / 180.0f) * (MAX_PULSE_US - MIN_PULSE_US);

    // 2. Darbe genisligini 16-bit donanim duty degerine donustur
    uint32_t duty = (uint32_t)((uint64_t)pulseUs * ((1UL << SERVO_RES) - 1) / PERIOD_US);
    return duty;
}
