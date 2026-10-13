#pragma once

// Tek pin kaynagi: ESP32 pin atamalari burada tutulur.
// Degisiklik yaparsan docs/pinout.md dosyasini da guncelle.

// --- TB6612FNG motor surucu (Hafta 1) ---
constexpr int PIN_MOTOR_PWMA = 25;  // Sol motor hizi (PWM)
constexpr int PIN_MOTOR_AIN1 = 26;  // Sol motor yon 1
constexpr int PIN_MOTOR_AIN2 = 27;  // Sol motor yon 2
constexpr int PIN_MOTOR_PWMB = 14;  // Sag motor hizi (PWM)
constexpr int PIN_MOTOR_BIN1 = 32;  // Sag motor yon 1 (12 strapping, 16 DevKit V1'de yok)
constexpr int PIN_MOTOR_BIN2 = 13;  // Sag motor yon 2
constexpr int PIN_MOTOR_STBY = 33;  // Surucu aktif/standby
