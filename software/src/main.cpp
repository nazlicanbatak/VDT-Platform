#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"
#include "servo_driver.h"
#include "steering.h"
#include "vehicle_controller.h"

// Donanim nesneleri
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

// Direksiyon servosu ve katmani
ServoDriver servo(PIN_SERVO_STEERING);
Steering steering(servo);

// Arac Kontrol Unitesi (VCU)
VehicleController vehicle(motors, steering);

// Takip edilen surus parametreleri
int   currentThrottle = 0;     // -100 ile +100 (%)
float currentSteering = 0.0f;  // -29.0 ile +29.0 (derece)

void printHelp() {
    Serial.println("\n==================================================");
    Serial.println("🚗 VDT-Platform: Arac Kontrol Testi (VehicleController)");
    Serial.println("==================================================");
    Serial.println("🎮 GAZ KONTROLLERI:");
    Serial.println("  [W] / [S] : Gaz %10 Artir (Ileri) / Azalt (Geri)");
    Serial.println("  [X]       : Gaz Kes (%0)");
    Serial.println("🎮 DIREKSIYON KONTROLLERI:");
    Serial.println("  [A] / [D] : Sola / Saga 3 derece normal adim");
    Serial.println("  [Q] / [E] : Sola / Saga 1 derece hassas adim");
    Serial.println("  [Z] / [V] : Tam Sol (-29 deg) / Tam Sag (+29 deg)");
    Serial.println("  [C]       : Direksiyonu Duzelt (0 deg Merkez)");
    Serial.println("🚨 GUVENLIK:");
    Serial.println("  [BOSLUK]  : ACIL DURDURMA (Gaz %0 + Direksiyon 0 deg)");
    Serial.println("  [H]       : Bu yardim menusunu goster");
    Serial.println("==================================================");
    Serial.println(">> Test basliyor. Arac beklemede.\n");
}

void printStatus() {
    Serial.print(">> [DURUM] Gaz: ");
    if (currentThrottle > 0) Serial.print("+");
    Serial.print(currentThrottle);
    Serial.print("%  |  Tekerlek: ");
    if (currentSteering > 0) Serial.print("+");
    Serial.print(currentSteering, 1);
    Serial.print(" deg  |  Servo: ");
    Serial.print(servo.getAngle(), 1);
    Serial.println(" deg");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    // B kanalina bagli sol motor mekanik yerlesimden dolayi ters dondugu icin duzeltildi
    motors.invertMotor(MOTOR_LEFT, true);

    // Araci baslat (motorlari kurar, servoyu baslatir ve guvenli durusa alir)
    vehicle.begin();

    printHelp();
    printStatus();
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
            // --- Gaz Kontrolleri ---
            case 'w':
            case 'W':
                currentThrottle += 10;
                break;
            case 's':
            case 'S':
                currentThrottle -= 10;
                break;
            case 'x':
            case 'X':
                currentThrottle = 0;
                Serial.println("[BILGI] Gaz kesildi (%0).");
                break;

            // --- Direksiyon Kontrolleri ---
            case 'a':
            case 'A':
                currentSteering -= 3.0f;
                break;
            case 'd':
            case 'D':
                currentSteering += 3.0f;
                break;
            case 'q':
            case 'Q':
                currentSteering -= 1.0f;
                break;
            case 'e':
            case 'E':
                currentSteering += 1.0f;
                break;
            case 'z':
            case 'Z':
                currentSteering = -29.0f;
                Serial.println("[BILGI] Tam Sol (-29 deg)");
                break;
            case 'v':
            case 'V':
                currentSteering = 29.0f;
                Serial.println("[BILGI] Tam Sag (+29 deg)");
                break;
            case 'c':
            case 'C':
                currentSteering = 0.0f;
                Serial.println("[BILGI] Direksiyon ortalandi (0 deg).");
                break;

            // --- Acil Durdurma ---
            case ' ':
            case 'p':
            case 'P':
                vehicle.emergencyStop();
                currentThrottle = vehicle.getThrottle();
                currentSteering = vehicle.getSteering();
                Serial.println("\n🚨 [ACIL DURDURMA] Motorlar kesildi ve direksiyon duzeltildi! 🚨");
                printStatus();
                return;

            // --- Yardim Menusu ---
            case 'h':
            case 'H':
                printHelp();
                return;

            default:
                Serial.println("[!] Gecersiz tus! (H tuşuna basarak yardimi gorebilirsiniz)");
                return;
        }

        // Sinirlamalari uygula
        currentThrottle = constrain(currentThrottle, -100, 100);
        currentSteering = constrain(currentSteering, -29.0f, 29.0f);

        // Komutu araca ilet
        vehicle.drive(currentThrottle, currentSteering);

        // Durumu ekrana yazdir
        delay(100);
        printStatus();
    }
}