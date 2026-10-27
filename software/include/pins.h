#pragma once

// Tek pin kaynagi: ESP32 pin atamalari burada tutulur.
// Degisiklik yaparsan docs/pinout.md dosyasini da guncelle.

// --- TB6612FNG motor surucu (Hafta 1) ---
// Donanim Baglantisi: Kanal A -> Sag Motor, Kanal B -> Sol Motor

// Sag Motor (Kanal A)
constexpr int PIN_MOTOR_RIGHT_PWM = 25;  // PWMA: Sag motor hizi (PWM)
constexpr int PIN_MOTOR_RIGHT_IN1 = 26;  // AIN1: Sag motor yon 1
constexpr int PIN_MOTOR_RIGHT_IN2 = 27;  // AIN2: Sag motor yon 2

// Sol Motor (Kanal B)
constexpr int PIN_MOTOR_LEFT_PWM  = 14;  // PWMB: Sol motor hizi (PWM)
constexpr int PIN_MOTOR_LEFT_IN1  = 32;  // BIN1: Sol motor yon 1 (12 strapping, 16 DevKit V1'de yok -> 32)
constexpr int PIN_MOTOR_LEFT_IN2  = 13;  // BIN2: Sol motor yon 2

// Surucu aktif/standby (Her iki motor icin ortak)
constexpr int PIN_MOTOR_STBY      = 33;

// TB6612FNG sematik pin isimleri karsiligi (Takas / kolay basvuru icin)
constexpr int PIN_MOTOR_PWMA = PIN_MOTOR_RIGHT_PWM;
constexpr int PIN_MOTOR_AIN1 = PIN_MOTOR_RIGHT_IN1;
constexpr int PIN_MOTOR_AIN2 = PIN_MOTOR_RIGHT_IN2;

constexpr int PIN_MOTOR_PWMB = PIN_MOTOR_LEFT_PWM;
constexpr int PIN_MOTOR_BIN1 = PIN_MOTOR_LEFT_IN1;
constexpr int PIN_MOTOR_BIN2 = PIN_MOTOR_LEFT_IN2;

// --- MG995 direksiyon servosu (Hafta 2) ---
constexpr int PIN_SERVO_STEERING = 4;   // Direksiyon servo sinyal pini (PWM)

// --- MPU6050 6-DOF IMU (Hafta 4) ---
constexpr int PIN_IMU_SDA = 21;  // I2C Veri (Serial Data)
constexpr int PIN_IMU_SCL = 22;  // I2C Saat (Serial Clock)

