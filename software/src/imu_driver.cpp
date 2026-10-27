#include "imu_driver.h"

// ==============================================================================
// VDT Platform - MPU6050 6-DOF IMU Surucu Gerceklesimi (Hafta 4)
// ==============================================================================

// MPU6050 Ic Register Adresleri
static constexpr uint8_t REG_SMPLRT_DIV   = 0x19;
static constexpr uint8_t REG_CONFIG       = 0x1A;
static constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
static constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
static constexpr uint8_t REG_PWR_MGMT_1   = 0x6B;
static constexpr uint8_t REG_WHO_AM_I     = 0x75;

// Olcek Donusum Katsayilari (Scale Factors)
// ±4g icin: 8192 LSB/g
static constexpr float ACCEL_SCALE_4G     = 8192.0f;
// ±500°/s icin: 65.5 LSB/(°/s)
static constexpr float GYRO_SCALE_500DPS  = 65.5f;

ImuDriver::ImuDriver(uint8_t sdaPin, uint8_t sclPin, uint8_t address)
    : _sdaPin(sdaPin),
      _sclPin(sclPin),
      _address(address),
      _connected(false),
      _errorCount(0),
      _gyroBiasX(0.0f),
      _gyroBiasY(0.0f),
      _gyroBiasZ(0.0f),
      _data{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0} {
}

bool ImuDriver::begin(uint32_t i2cClock) {
    Serial.printf("[IMU] MPU6050 baslatiliyor (SDA: %d, SCL: %d, Adres: 0x%02X)...\n",
                  _sdaPin, _sclPin, _address);

    // 1. I2C donanimini baslat
    Wire.begin(_sdaPin, _sclPin, i2cClock);

    // 2. Cihaz kimligini (WHO_AM_I) dogrula
    uint8_t whoAmI = 0;
    if (!readRegisters(REG_WHO_AM_I, &whoAmI, 1)) {
        Serial.println("❌ [IMU] HATA: MPU6050 ile I2C iletisimi kurulamadi! Kablolari kontrol edin.");
        _connected = false;
        return false;
    }

    Serial.printf("[IMU] WHO_AM_I okundu: 0x%02X ", whoAmI);
    if (whoAmI == 0x68) {
        Serial.println("(Orijinal MPU6050)");
    } else if (whoAmI == 0x70 || whoAmI == 0x72) {
        Serial.println("(Uyumlu Klon MPU6050)");
    } else {
        Serial.printf("(Bilinmeyen ID: 0x%02X, yine de baslatilmaya calisiliyor)\n", whoAmI);
    }

    // 3. Sensoru uyandir ve en kararli saat kaynagini (PLL with X-gyro) sec
    if (!writeRegister(REG_PWR_MGMT_1, 0x01)) {
        _connected = false;
        return false;
    }
    delay(10);

    // 4. Ornekleme frekansi bolucusu (Sample Rate = 1 kHz / (1 + 0) = 1 kHz)
    writeRegister(REG_SMPLRT_DIV, 0x00);

    // 5. Dijital Alcaktan Geciren Filtre (DLPF): ~42 Hz bant genisligi (Motor titresimlerini suzer)
    writeRegister(REG_CONFIG, 0x03);

    // 6. Gyro Olcegi: ±500°/s (FS_SEL = 1 -> 0x08)
    writeRegister(REG_GYRO_CONFIG, 0x08);

    // 7. Ivmeolcer Olcegi: ±4g (AFS_SEL = 1 -> 0x08)
    writeRegister(REG_ACCEL_CONFIG, 0x08);

    _connected = true;
    Serial.println("✅ [IMU] MPU6050 basariyla yapilandirildi (±4g, ±500°/s, DLPF 42Hz).");

    // 8. Baslangic kalibrasyonunu calistir (Varsayilan 200 ornek)
    calibrateGyro();

    return true;
}

void ImuDriver::calibrateGyro(uint16_t samples) {
    if (!_connected) return;

    Serial.printf("[IMU] Gyro sifir noktasi kalibre ediliyor (%d ornek)... Lutfen araci kipirdatmayin!\n", samples);

    float sumGx = 0.0f;
    float sumGy = 0.0f;
    float sumGz = 0.0f;
    uint16_t validCount = 0;

    for (uint16_t i = 0; i < samples; i++) {
        uint8_t buffer[6];
        // Gyro verileri 0x43 adresinden baslar (6 bayt)
        if (readRegisters(0x43, buffer, 6)) {
            int16_t rawGx = (int16_t)((buffer[0] << 8) | buffer[1]);
            int16_t rawGy = (int16_t)((buffer[2] << 8) | buffer[3]);
            int16_t rawGz = (int16_t)((buffer[4] << 8) | buffer[5]);

            sumGx += (float)rawGx / GYRO_SCALE_500DPS;
            sumGy += (float)rawGy / GYRO_SCALE_500DPS;
            sumGz += (float)rawGz / GYRO_SCALE_500DPS;
            validCount++;
        }
        delay(5);
    }

    if (validCount > 0) {
        _gyroBiasX = sumGx / (float)validCount;
        _gyroBiasY = sumGy / (float)validCount;
        _gyroBiasZ = sumGz / (float)validCount;
        Serial.printf("✅ [IMU] Gyro Bias hesaplandi -> X: %+.2f °/s, Y: %+.2f °/s, Z: %+.2f °/s\n",
                      _gyroBiasX, _gyroBiasY, _gyroBiasZ);
    }
}

bool ImuDriver::read() {
    if (!_connected) return false;

    // 0x3B adresinden baslayarak tek seferde 14 baytlik veriyi oku
    uint8_t rawBytes[14];
    if (!readRegisters(REG_ACCEL_XOUT_H, rawBytes, 14)) {
        _errorCount++;
        return false;
    }

    // 16-bit Buyuk Uclu (Big-Endian) donusumleri
    int16_t rawAx   = (int16_t)((rawBytes[0]  << 8) | rawBytes[1]);
    int16_t rawAy   = (int16_t)((rawBytes[2]  << 8) | rawBytes[3]);
    int16_t rawAz   = (int16_t)((rawBytes[4]  << 8) | rawBytes[5]);
    int16_t rawTemp = (int16_t)((rawBytes[6]  << 8) | rawBytes[7]);
    int16_t rawGx   = (int16_t)((rawBytes[8]  << 8) | rawBytes[9]);
    int16_t rawGy   = (int16_t)((rawBytes[10] << 8) | rawBytes[11]);
    int16_t rawGz   = (int16_t)((rawBytes[12] << 8) | rawBytes[13]);

    // SI Birimlerine Donusturme
    _data.ax   = (float)rawAx / ACCEL_SCALE_4G;
    _data.ay   = (float)rawAy / ACCEL_SCALE_4G;
    _data.az   = (float)rawAz / ACCEL_SCALE_4G;
    _data.temp = ((float)rawTemp / 340.0f) + 36.53f;

    // Gyro verisinden kalibre edilmis bias degerini cikar
    _data.gx   = ((float)rawGx / GYRO_SCALE_500DPS) - _gyroBiasX;
    _data.gy   = ((float)rawGy / GYRO_SCALE_500DPS) - _gyroBiasY;
    _data.gz   = ((float)rawGz / GYRO_SCALE_500DPS) - _gyroBiasZ;

    _data.timestampMs = millis();
    return true;
}

const ImuData& ImuDriver::getData() const {
    return _data;
}

bool ImuDriver::isConnected() const {
    return _connected;
}

uint32_t ImuDriver::getErrorCount() const {
    return _errorCount;
}

bool ImuDriver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    uint8_t status = Wire.endTransmission();
    return (status == 0);
}

bool ImuDriver::readRegisters(uint8_t reg, uint8_t* buffer, size_t length) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) { // Repeated start
        return false;
    }

    size_t received = Wire.requestFrom(_address, (uint8_t)length);
    if (received != length) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
