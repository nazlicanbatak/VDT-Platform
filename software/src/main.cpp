#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"

// Pin tanimlamalarini pins.h dosyasindan aliyoruz (Single Source of Truth)
// Donanim lehimlemesi: Kanal A = Sag Motor (AIN1/2, PWMA), Kanal B = Sol Motor (BIN1/2, PWMB)
MotorPins leftPins  = { .pwm = PIN_MOTOR_LEFT_PWM,  .in1 = PIN_MOTOR_LEFT_IN1,  .in2 = PIN_MOTOR_LEFT_IN2 };
MotorPins rightPins = { .pwm = PIN_MOTOR_RIGHT_PWM, .in1 = PIN_MOTOR_RIGHT_IN1, .in2 = PIN_MOTOR_RIGHT_IN2 };

// Motor surucu nesnesi"
MotorDriver motors(leftPins, rightPins, PIN_MOTOR_STBY);

void setup() {
    // Seri haberlesme baslatma (Debug ve telemetri loglari icin)
    Serial.begin(115200);
    delay(1500); // Seri portun oturmasi icin kisa bekleme

    Serial.println();
    Serial.println("==================================================");
    Serial.println("🚗 VDT-Platform: ESP32 + TB6612FNG Tezgâh Testi");
    Serial.println("==================================================");

    // Motor surucuyu guvenli sekilde baslat
    motors.begin();

    // B pinlerine bagli sol motor mekanik yerlesimden dolayi ters dondugu icin yonu yazilimla duzeltildi
    motors.invertMotor(MOTOR_LEFT, true);

    Serial.println(">> Motor surucu hazir. Test dongusu basliyor...");
}

void loop() {
    // --- TEST 1: Ileri Yon (Orta Hiz: 140/255) ---
    Serial.println("[TEST 1] Ileri Yon (PWM: 140)");
    motors.setMotor(MOTOR_LEFT, DIR_FORWARD, 140);
    motors.setMotor(MOTOR_RIGHT, DIR_FORWARD, 140);
    delay(2000);

    // --- TEST 2: Guvenli Durdurma ---
    Serial.println("[TEST 2] Durdurma (1 saniye)");
    motors.stopAll();
    delay(1000);

    // --- TEST 3: Geri Yon (Orta Hiz: 140/255) ---
    Serial.println("[TEST 3] Geri Yon (PWM: 140)");
    motors.setMotor(MOTOR_LEFT, DIR_BACKWARD, 140);
    motors.setMotor(MOTOR_RIGHT, DIR_BACKWARD, 140);
    delay(2000);

    // --- TEST 4: Guvenli Durdurma ---
    Serial.println("[TEST 4] Durdurma (1 saniye)");
    motors.stopAll();
    delay(1000);

    // --- TEST 5: Yumusak Hizlanma (Akim Koruma / Ramp-up Testi) ---
    Serial.println("[TEST 5] Yumusak Hizlanma (60 -> 220 PWM)");
    for (uint8_t speed = 60; speed <= 220; speed += 20) {
        motors.setMotor(MOTOR_LEFT, DIR_FORWARD, speed);
        motors.setMotor(MOTOR_RIGHT, DIR_FORWARD, speed);
        delay(150);
    }
    delay(1000);

    // --- TEST 6: Dongu Sonu Bekleme ---
    Serial.println("[TEST 6] Dongu tamamlandi, 3 saniye bekleniyor...\n");
    motors.stopAll();
    delay(3000);
}