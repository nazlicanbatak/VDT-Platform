#pragma once
#include <Arduino.h>

// ==============================================================================
// VDT Platform - Iletisim Protokolu ve Surus Tanimlamalari (Hafta 3)
// Bu dosya telefon/web ve ESP32 arasindaki komut yapisini ve sinirlari belirler.
// ==============================================================================

// --- Fiziksel Guvenlik Sinirlari ---
constexpr int8_t  PROTOCOL_THROTTLE_MIN = -100;   // Maksimum geri gaz (%)
constexpr int8_t  PROTOCOL_THROTTLE_MAX =  100;   // Maksimum ileri gaz (%)
constexpr float   PROTOCOL_STEERING_MIN = -29.0f; // Tam sol aci (derece)
constexpr float   PROTOCOL_STEERING_MAX =  29.0f; // Tam sag aci (derece)

// Baglanti koptugunda veya veri akisi durdugunda fail-safe devreye girme suresi
constexpr uint32_t PROTOCOL_HEARTBEAT_TIMEOUT_MS = 500; // milisaniye

// --- Surus Komut Paketi (DriveCommand) ---
// WebSocket veya klavyeden gelen komutlar bu kutu icinde FreeRTOS kuyruguna aktarilir.
struct DriveCommand {
    int8_t throttle;  // Gaz yuzdesi (-100 ile +100)
    float  steering;  // Direksiyon acisi (-29.0 ile +29.0)

    // Gelen degerlerin fiziksel sinirlar icinde oldugunu garanti eder (Clamping)
    void validate() {
        if (throttle < PROTOCOL_THROTTLE_MIN) throttle = PROTOCOL_THROTTLE_MIN;
        if (throttle > PROTOCOL_THROTTLE_MAX) throttle = PROTOCOL_THROTTLE_MAX;
        if (steering < PROTOCOL_STEERING_MIN) steering = PROTOCOL_STEERING_MIN;
        if (steering > PROTOCOL_STEERING_MAX) steering = PROTOCOL_STEERING_MAX;
    }
};
