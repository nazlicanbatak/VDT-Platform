#include "vehicle_controller.h"

// Kurucu fonksiyon: alt sistem referanslarini kaydeder
VehicleController::VehicleController(MotorDriver& motors, Steering& steering)
    : _motors(motors),
      _steering(steering),
      _currentThrottle(0),
      _currentSteering(0.0f) {}

// Alt sistemleri baslatir ve guvenli durus konumuna alir
void VehicleController::begin() {
    _motors.begin();
    _steering.begin();
    emergencyStop();
}

// Araci verilen gaz ve direksiyon komutuna gore surer
void VehicleController::drive(int throttlePct, float steeringDeg) {
    // 1. Gaz komutunu guvenli yuzde araliginda (-100 ile +100) tut
    throttlePct = constrain(throttlePct, -100, 100);
    _currentThrottle = throttlePct;

    // 2. Direksiyonu ayarla (steering sinifi kendi icinde +-29 dereceyi constrain eder)
    _steering.setAngle(steeringDeg);
    _currentSteering = _steering.getAngle();

    // 3. Motorlarin yonunu ve hizini ayarla
    if (throttlePct > 0) {
        uint8_t pwm = throttleToPwm(throttlePct);
        _motors.setMotor(MOTOR_LEFT, DIR_FORWARD, pwm);
        _motors.setMotor(MOTOR_RIGHT, DIR_FORWARD, pwm);
    } else if (throttlePct < 0) {
        uint8_t pwm = throttleToPwm(throttlePct);
        _motors.setMotor(MOTOR_LEFT, DIR_BACKWARD, pwm);
        _motors.setMotor(MOTOR_RIGHT, DIR_BACKWARD, pwm);
    } else {
        // Gaz sifir ise motorlari durdur
        _motors.stopAll();
    }
}

// Acil durdurma: Motorlari keser ve direksiyonu duz konuma alir
void VehicleController::emergencyStop() {
    _motors.stopAll();
    _steering.center();
    _currentThrottle = 0;
    _currentSteering = 0.0f;
}

// Anlik gaz yuzdesini dondurur
int VehicleController::getThrottle() const {
    return _currentThrottle;
}

// Anlik tekerlek acisini dondurur
float VehicleController::getSteering() const {
    return _currentSteering;
}

// Yuzde degerini (0..100) TB6612 8-bit PWM degerine (0..255) donusturur
uint8_t VehicleController::throttleToPwm(int throttlePct) const {
    return (uint8_t)((abs(throttlePct) * 255UL) / 100UL);
}
