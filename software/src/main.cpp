#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"
#include "servo_driver.h"
#include "steering.h"
#include "vehicle_controller.h"
#include "protocol.h"
#include "wifi_service.h"

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

// --- FreeRTOS ve Wi-Fi Katmani (Hafta 3) ---
QueueHandle_t commandQueue = nullptr;
WifiService   wifiService;

// Tek motor gorevi: Motor ve direksiyon donanimini yalnizca bu gorev surer (SOLID & Eşzamanlilik Guvenligi)
void vehicleTask(void* pvParameters) {
    DriveCommand cmd = {0, 0.0f};
    bool wasTimedOut = false;

    while (true) {
        // 1. Fail-Safe / Watchdog Kontrolu
        // Eger istemci bagliysa ve 500ms boyunca heartbeat gelmediyse acil durdur
        if (wifiService.isHeartbeatTimedOut()) {
            if (!wasTimedOut) {
                vehicle.emergencyStop();
                wasTimedOut = true;
                Serial.println("\n🚨 [WATCHDOG / FAIL-SAFE] Baglanti zaman asimi! Motorlar durduruldu.");
            }
        } else {
            wasTimedOut = false;
        }

        // 2. Kuyruktan yeni surus komutunu bekle (20ms periyot)
        if (xQueueReceive(commandQueue, &cmd, pdMS_TO_TICKS(20)) == pdPASS) {
            vehicle.drive(cmd.throttle, cmd.steering);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Baslatma bilgilerini yazdir
void printStartupBanner() {
    Serial.println("\n==================================================");
    Serial.println("🚗 VDT-Platform: Wi-Fi Mobil Kokpit Yayinda");
    Serial.println("==================================================");
    Serial.println("📶 Wi-Fi Erisim Noktasi: 'VDT-Car' (Sifresiz)");
    Serial.println("🌐 Tarayici Adresi    : http://192.168.4.1");
    Serial.println("📱 Telefonunuzdan baglanip surus yapabilirsiniz.");
    Serial.println("==================================================\n");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    // B kanalina bagli sol motor mekanik yerlesimden dolayi ters dondugu icin duzeltildi
    motors.invertMotor(MOTOR_LEFT, true);

    // Araci baslat (motorlari kurar, servoyu baslatir ve guvenli durusa alir)
    vehicle.begin();

    // 1 elemanlik komut kuyrugu olustur (En yeni komut her zaman eskini ezer)
    commandQueue = xQueueCreate(1, sizeof(DriveCommand));

    // Wi-Fi Access Point ve Web/WebSocket sunucusunu baslat
    wifiService.begin(commandQueue, "VDT-Car");

    // Motor gorevini Core 1 uzerinde calistir (Katı zamanlama ve guvenlik)
    xTaskCreatePinnedToCore(
        vehicleTask,
        "VehicleTask",
        4096,
        nullptr,
        2,              // Yuksek oncelik
        nullptr,
        1               // Core 1
    );

    printStartupBanner();
}

void loop() {
    // Wi-Fi ve WebSocket ag olaylarini isle (Core 0 / Standart Dongu)
    wifiService.loop();
}