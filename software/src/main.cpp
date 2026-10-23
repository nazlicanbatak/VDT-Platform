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

// Takip edilen surus parametreleri (Klavye testi icin)
int   currentThrottle = 0;     // -100 ile +100 (%)
float currentSteering = 0.0f;  // -29.0 ile +29.0 (derece)

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

void printHelp() {
    Serial.println("\n==================================================");
    Serial.println("🚗 VDT-Platform: Wi-Fi Mobil Kokpit & Serial");
    Serial.println("==================================================");
    Serial.println("📶 WI-FI ERISIM NOKTASI (Access Point):");
    Serial.println("  Wi-Fi Adi: 'VDT-Car' (Parolasiz)");
    Serial.println("  Tarayici Adresi: http://192.168.4.1");
    Serial.println("  iPhone Safari veya Chrome ile baglanabilirsiniz.");
    Serial.println("🎮 GAZ KONTROLLERI (Serial Monitor):");
    Serial.println("  [W] / [S] : Gaz %10 Artir (Ileri) / Azalt (Geri)");
    Serial.println("  [X]       : Gaz Kes (%0)");
    Serial.println("🎮 DIREKSIYON KONTROLLERI:");
    Serial.println("  [A] / [D] : Sola / Saga 3 derece normal adim");
    Serial.println("  [Q] / [E] : Sola / Saga 1 derece hassas adim");
    Serial.println("  [Z] / [V] : Tam Sol (-29 deg) / Tam Sag (+29 deg)");
    Serial.println("  [C]       : Direksiyonu Duzelt (0 deg Merkez)");
    Serial.println("🚨 GUVENLIK:");
    Serial.println("  [BOSLUK]  : ACIL DURDURMA");
    Serial.println("  [H]       : Bu yardim menusunu goster");
    Serial.println("==================================================");
    Serial.println(">> Sistem basliyor. Arac beklemede.\n");
}

void printStatus() {
    Serial.print(">> [DURUM] Gaz: ");
    if (vehicle.getThrottle() > 0) Serial.print("+");
    Serial.print(vehicle.getThrottle());
    Serial.print("%  |  Tekerlek: ");
    if (vehicle.getSteering() > 0) Serial.print("+");
    Serial.print(vehicle.getSteering(), 1);
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

    // 1 elemanlik komut kuyrugu olustur (En yeni komut her zaman eskini ezer)
    commandQueue = xQueueCreate(1, sizeof(DriveCommand));

    // Wi-Fi Access Point ve Web/WebSocket sunucusunu baslat
    wifiService.begin(commandQueue, "VDT-Car");

    // Motor gorevini Core 1 uzerinde calistir
    xTaskCreatePinnedToCore(
        vehicleTask,
        "VehicleTask",
        4096,
        nullptr,
        2,              // Yuksek oncelik
        nullptr,
        1               // Core 1
    );

    printHelp();
    printStatus();
}

void loop() {
    // Wi-Fi ve WebSocket isteklerini isle
    wifiService.loop();

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
            case 'P': {
                currentThrottle = 0;
                currentSteering = 0.0f;
                DriveCommand stopCmd = { 0, 0.0f };
                xQueueOverwrite(commandQueue, &stopCmd);
                Serial.println("\n🚨 [ACIL DURDURMA] Motorlar kesildi ve direksiyon duzeltildi! 🚨");
                delay(50);
                printStatus();
                return;
            }

            // --- Yardim Menusu ---
            case 'h':
            case 'H':
                printHelp();
                return;

            default:
                Serial.println("[!] Gecersiz tus! (H tusuna basarak yardimi gorebilirsiniz)");
                return;
        }

        // Sinirlamalari uygula
        currentThrottle = constrain(currentThrottle, -100, 100);
        currentSteering = constrain(currentSteering, -29.0f, 29.0f);

        // Komutu kuyruga ilet (Motor gorevi tek merkezden calistirsin)
        DriveCommand keyCmd = { static_cast<int8_t>(currentThrottle), currentSteering };
        xQueueOverwrite(commandQueue, &keyCmd);

        // Durumu ekrana yazdir
        delay(50);
        printStatus();
    }
}