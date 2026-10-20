#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"
#include "servo_driver.h"
#include "steering.h"

// Donanim nesneleri
// Motorlar su an guvenlik amaciyla kapali tutulacak
MotorPins leftPins = {
    .pwm = PIN_MOTOR_LEFT_PWM,
    .in1 = PIN_MOTOR_LEFT_IN1,
    .in2 = PIN_MOTOR_LEFT_IN2
};
MotorPins rightPins = {
    .pwm = PIN_MOTOR_RIGHT_PWM,
    .in1 = PIN_MOTOR_RIGHT_IN1,
    .in2 = PIN_MOTOR_RIGHT_IN2
};
MotorDriver motors(leftPins, rightPins, PIN_MOTOR_STBY);

// Direksiyon servosu (GPIO 4) ve Direksiyon Katmani (Steering)
ServoDriver servo(PIN_SERVO_STEERING);
Steering steering(servo);

// Takip edilen hedef tekerlek acisi (-28.0 ile +28.0 arasi, 0 = duz)
float targetWheelAngle = 0.0f;

void printHelp() {
    Serial.println("\n==================================================");
    Serial.println("🚗 VDT-Platform: Direksiyon Katmani Testi (Steering)");
    Serial.println("==================================================");
    Serial.println("Tekerlekler su an guvenlik amaciyla kapali.");
    Serial.println("Efektif Tekerlek Aci Araligi: -28 deg (Sol) <-> +28 deg (Sag)");
    Serial.println("Komutlar:");
    Serial.println("  [A] / [D] : Sola / Saga 1 derece hassas adim");
    Serial.println("  [Q] / [E] : Sola / Saga 5 derece hizli adim");
    Serial.println("  [Z] / [X] : Tam Sol (-28 deg) / Tam Sag (+28 deg)");
    Serial.println("  [C]       : 0 derece merkez konumu (Duz)");
    Serial.println("  [H]       : Yardim menusu");
    Serial.println("==================================================");
    Serial.print(">> Baslangic Tekerlek Acisi: ");
    Serial.print(targetWheelAngle, 1);
    Serial.println(" deg\n");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    // Motorlari guvenli sekilde baslat ve tamamen durdur
    motors.begin();
    motors.stopAll();

    // Direksiyon katmanini baslat (servoyu kurar, limitleri yukler ve ortalar)
    steering.begin();
    targetWheelAngle = steering.getAngle();

    printHelp();
}

void loop() {
    // Klavyeden (Serial Monitor) bir karakter girildi mi?
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Satir sonu karakterlerini atla
        if (cmd == '\r' || cmd == '\n') {
            return;
        }

        switch (cmd) {
            case 'a':
            case 'A':
                targetWheelAngle -= 1.0f;
                break;
            case 'd':
            case 'D':
                targetWheelAngle += 1.0f;
                break;
            case 'q':
            case 'Q':
                targetWheelAngle -= 5.0f;
                break;
            case 'e':
            case 'E':
                targetWheelAngle += 5.0f;
                break;
            case 'z':
            case 'Z':
                targetWheelAngle = -29.0f;
                Serial.println("[BILGI] Tam Sol (-29 deg)");
                break;
            case 'x':
            case 'X':
                targetWheelAngle = 29.0f;
                Serial.println("[BILGI] Tam Sag (+29 deg)");
                break;
            case 'c':
            case 'C':
                targetWheelAngle = 0.0f;
                Serial.println("[BILGI] Merkeze alindi (0 deg).");
                break;
            case 'h':
            case 'H':
                printHelp();
                return;
            default:
                Serial.println("[!] Gecersiz tus! (A/D: +-1, Q/E: +-5, Z/X: +-28, C: Duz, H: Yardim)");
                return;
        }

        // Tekerlek acisini uygula (steering katmani icinde constrain ve interpolasyon yapilir)
        steering.setAngle(targetWheelAngle);
        targetWheelAngle = steering.getAngle();

        // Kullaniciya hem tekerlek acisini hem de arka plandaki servo acisini goster
        delay(150);
        Serial.print(">> Tekerlek: ");
        if (targetWheelAngle > 0) Serial.print("+");
        Serial.print(targetWheelAngle, 1);
        Serial.print(" deg  |  Servo: ");
        Serial.print(servo.getAngle(), 1);
        Serial.println(" deg");
    }
}