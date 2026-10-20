#include "motor_driver.h"
#include "pins.h"
#include "servo_driver.h"
#include <Arduino.h>

// Donanim nesneleri
// Motorlar su an guvenlik amaciyla kapali tutulacak
MotorPins leftPins = {.pwm = PIN_MOTOR_LEFT_PWM,
                      .in1 = PIN_MOTOR_LEFT_IN1,
                      .in2 = PIN_MOTOR_LEFT_IN2};
MotorPins rightPins = {.pwm = PIN_MOTOR_RIGHT_PWM,
                       .in1 = PIN_MOTOR_RIGHT_IN1,
                       .in2 = PIN_MOTOR_RIGHT_IN2};
MotorDriver motors(leftPins, rightPins, PIN_MOTOR_STBY);

// Direksiyon servosu (GPIO 4)
ServoDriver servo(PIN_SERVO_STEERING);

// Takip edilen kalibrasyon acisi (Olculen merkez trim: 84 derece)
float currentAngle = 84.0f;

void printHelp() {
  Serial.println("\n==================================================");
  Serial.println("🚗 VDT-Platform: MG995 Direksiyon Kalibrasyonu");
  Serial.println("==================================================");
  Serial.println("Tekerlekler su an kapali (durumda).");
  Serial.println("Guvenli Aci Sinirlari: 38 deg - 125 deg");
  Serial.println("Komutlar:");
  Serial.println("  [A] / [D] : Sola / Saga 1 derece hassas adim");
  Serial.println("  [Q] / [E] : Sola / Saga 5 derece hizli adim");
  Serial.println("  [C]       : 84 derece merkez konumu");
  Serial.println("==================================================");
  Serial.print(">> Merkez Aci: ");
  Serial.print(currentAngle);
  Serial.println(" derece\n");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  // Motorlari guvenli sekilde baslat ve tamamen durdur
  motors.begin();
  motors.stopAll();

  // Servoyu baslat, guvenlik limitlerini uygula ve 84 derece merkeze al
  servo.begin();
  servo.setLimits(38.0f, 125.0f);
  servo.setAngle(84.0f);
  currentAngle = servo.getAngle();

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
      currentAngle -= 1.0f;
      break;
    case 'd':
    case 'D':
      currentAngle += 1.0f;
      break;
    case 'q':
    case 'Q':
      currentAngle -= 5.0f;
      break;
    case 'e':
    case 'E':
      currentAngle += 5.0f;
      break;
    case 'c':
    case 'C':
      currentAngle = 84.0f;
      Serial.println("[BILGI] Merkeze alindi.");
      break;
    case 'h':
    case 'H':
      printHelp();
      return;
    default:
      Serial.println(
          "[!] Gecersiz tus! (A/D: +-1, Q/E: +-5, C: Merkez, H: Yardim)");
      return;
    }

    // Acıyı guvenli limitler (38 - 125) arasinda tut ve servoya gonder
    currentAngle = constrain(currentAngle, 38.0f, 125.0f);
    servo.setAngle(currentAngle);

    // Anlik aciyi ekrana yaz
    delay(200);
    Serial.print(">> Aci: ");
    Serial.print(currentAngle, 1);
    Serial.println(" derece");
  }
}