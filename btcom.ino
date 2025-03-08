#include <Wire.h>
#include <MPU6050.h>
#include "BluetoothSerial.h"

MPU6050 mpu;
BluetoothSerial SerialBT;

void setup() {
  // Seri haberleşmeyi başlat
  Serial.begin(115200);
  
  // Bluetooth cihaz adını belirle ve başlat
  SerialBT.begin("ESP32_MPU60502"); // Bluetooth cihaz adı
  Serial.println("Bluetooth Başlatıldı. ESP32_MPU6050 cihazı bağlanmaya hazır.");

  // I2C için SDA ve SCL pinlerini belirle
  Wire.begin(21, 22); // SDA = 21, SCL = 22
  mpu.initialize();

  // MPU6050 bağlantı kontrolü
  if (mpu.testConnection()) {
    Serial.println("MPU6050 bağlantısı başarılı!");
    SerialBT.println("MPU6050 bağlantısı başarılı!");
  } else {
    Serial.println("MPU6050 bağlantısı başarısız!");
    SerialBT.println("MPU6050 bağlantısı başarısız!");
  }
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  // MPU6050'den veri okuma
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Verileri Bluetooth üzerinden gönderme
  SerialBT.print("Ax: "); SerialBT.print(ax);
  SerialBT.print(" Ay: "); SerialBT.print(ay);
  SerialBT.print(" Az: "); SerialBT.print(az);
  SerialBT.print(" | Gx: "); SerialBT.print(gx);
  SerialBT.print(" Gy: "); SerialBT.print(gy);
  SerialBT.print(" Gz: "); SerialBT.println(gz);

  Serial.print("Ax: "); Serial.print(ax);
  Serial.print(" Ay: "); Serial.print(ay);
  Serial.print(" Az: "); Serial.print(az);
  Serial.print(" | Gx: "); Serial.print(gx);
  Serial.print(" Gy: "); Serial.print(gy);
  Serial.print(" Gz: "); Serial.println(gz);

  // Bir sonraki okuma için kısa bir bekleme
  delay(500);
}