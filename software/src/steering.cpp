#include "steering.h"

// Direksiyon sinirlari ve mekanik limitler (vehicle_params.md degerleri)
static const float MAX_WHEEL_DEG = 29.0f;   // Her yonde maksimum tekerlek acisi
static const float MIN_SERVO_DEG = 38.0f;   // Servo sol limiti (4 derece guvenlik payli)
static const float MAX_SERVO_DEG = 125.0f;  // Servo sag limiti (4 derece guvenlik payli)

// Olcum tablosu: ayni siradaki iki deger birbirine karsilik gelir
// Tekerlek acisi (efektif, sol = negatif) -> Servo acisi
// NOT: Tekerlek dizisi kucukten buyuge sirali olmali (interpolasyon icin)
static const float WHEEL_TABLE[] = { -29.0f, -18.0f, -9.5f, -5.5f,  0.0f, 13.0f, 20.5f,  26.0f,  32.5f };
static const float SERVO_TABLE[] = {  38.0f,  50.0f, 62.0f, 74.0f, 84.0f, 94.0f, 104.0f, 114.0f, 125.0f };
static const int   TABLE_SIZE    = sizeof(WHEEL_TABLE) / sizeof(WHEEL_TABLE[0]);

// Kurucu fonksiyon: servo surucuye referans alir
Steering::Steering(ServoDriver& servoDriver)
    : _servo(servoDriver), _currentAngle(0.0f) {}

// Servoyu baslatir, mekanik limitleri uygular ve direksiyonu duz konuma getirir
void Steering::begin() {
    _servo.begin();
    _servo.setLimits(MIN_SERVO_DEG, MAX_SERVO_DEG);
    center();
}

// Hedef tekerlek acisini sinirlar, servo acisina cevirir ve servoya gonderir
void Steering::setAngle(float wheelDeg) {
    // Guvenlik: Tekerlek acisini +-28 derece araliginda tut
    wheelDeg = constrain(wheelDeg, -MAX_WHEEL_DEG, MAX_WHEEL_DEG);
    _currentAngle = wheelDeg;

    _servo.setAngle(calculateServoDeg(wheelDeg));
}

// Direksiyonu duz (0 derece) konuma getirir
void Steering::center() {
    setAngle(0.0f);
}

// Son komut verilen tekerlek acisini dondurur
float Steering::getAngle() const {
    return _currentAngle;
}

// Tablodaki iki komsu nokta arasinda dogrusal interpolasyon yapar
float Steering::calculateServoDeg(float wheelDeg) const {
    // Hedefin icinde bulundugu aralik: WHEEL_TABLE[i] ile WHEEL_TABLE[i + 1]
    for (int i = 0; i < TABLE_SIZE - 1; i++) {
        if (wheelDeg <= WHEEL_TABLE[i + 1]) {
            // t: hedefin aralik icindeki orani (0.0 = alt nokta, 1.0 = ust nokta)
            float t = (wheelDeg - WHEEL_TABLE[i]) / (WHEEL_TABLE[i + 1] - WHEEL_TABLE[i]);
            return SERVO_TABLE[i] + t * (SERVO_TABLE[i + 1] - SERVO_TABLE[i]);
        }
    }

    // Tablonun disinda kalirsa en son servo degerini kullan (normalde olmaz)
    return SERVO_TABLE[TABLE_SIZE - 1];
}