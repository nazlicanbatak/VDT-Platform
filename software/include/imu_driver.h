#pragma once
#include <Arduino.h>
#include <Wire.h>

// ==============================================================================
// VDT Platform - MPU6050 6-DOF IMU Surucu Sinifi (Hafta 4)
// Tek Sorumluluk (SRP): Yalnizca I2C uzerinden MPU6050 ile haberlesmek,
// ham verileri okumak, gyro kalibrasyonunu yapmak ve SI birimlerine donusturmek.
// ==============================================================================

// Standartlastirilmis IMU veri paketi (SI Birimleri)
struct ImuData {
    float ax;              // X ekseni ivmesi (g) [Ileri / Geri]
    float ay;              // Y ekseni ivmesi (g) [Sol / Sag yanal ivme]
    float az;              // Z ekseni ivmesi (g) [Dusey yercekimi ivmesi]
    float gx;              // X ekseni acisal hizi (°/s) [Roll hizi]
    float gy;              // Y ekseni acisal hizi (°/s) [Pitch hizi]
    float gz;              // Z ekseni acisal hizi (°/s) [Yaw / Donus hizi]
    float temp;            // Sensör ic sicakligi (°C)
    uint32_t timestampMs;  // Okuma zamani (ms)
};

class ImuDriver {
public:
    // Kurucu fonksiyon: I2C pinlerini ve MPU6050 adresini (varsayilan 0x68) alir
    ImuDriver(uint8_t sdaPin, uint8_t sclPin, uint8_t address = 0x68);

    // I2C hattini baslatir, MPU6050'yi uyandirir, ±4g ve ±500°/s araliklarini ayarlar
    bool begin(uint32_t i2cClock = 400000);

    // Sensorden 14 baytlik veriyi tek seferde okur ve SI birimlerine donusturur
    bool read();

    // Arac hareketsizken gyro sapmasini (bias/offset) olcer ve sifirlar
    void calibrateGyro(uint16_t samples = 200);

    // En son okunan temizlenmis telemetri verisini dondurur
    const ImuData& getData() const;

    // Sensorun hatta bagli ve saglikli olup olmadigini dondurur
    bool isConnected() const;

    // I2C okuma sirasinda meydana gelen hata sayisini dondurur
    uint32_t getErrorCount() const;

private:
    uint8_t   _sdaPin;
    uint8_t   _sclPin;
    uint8_t   _address;
    bool      _connected;
    uint32_t  _errorCount;

    // Gyro sifir noktasi kaymasi (Bias)
    float     _gyroBiasX;
    float     _gyroBiasY;
    float     _gyroBiasZ;

    // En son guncel veri
    ImuData   _data;

    // I2C yardimci fonksiyonlari
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);
};
