#include <Wire.h>
#include <MPU6050.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// MPU6050 Nesneleri
MPU6050 mpu1(0x68);  // AD0 GND'ye bağlı olan sensör
MPU6050 mpu2(0x69);// İkinci sensör

// BLE UUID'leri
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-123456789abc"

// BLE Karakteristik
BLECharacteristic *pCharacteristic;

// BLE Sunucusu
BLEServer *pServer;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
   mpu1.initialize();
   mpu2.initialize();

  // BLE Başlatma
  BLEDevice::init("ESP32-BLE");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE başlatıldı.");
}

void loop() {
  // MPU6050 #1 Verilerini Oku
 int16_t ax1, ay1, az1;
  int16_t gx1, gy1, gz1;
  int16_t ax2, ay2, az2;
  int16_t gx2, gy2, gz2;

  // MPU6050-1'den veri oku
  mpu1.getMotion6(&ax1, &ay1, &az1, &gx1, &gy1, &gz1);

  // MPU6050-2'den veri oku
  mpu2.getMotion6(&ax2, &ay2, &az2, &gx2, &gy2, &gz2);

  // Verileri JSON Formatında Birleştir
  String data = String("{") +
                "\"mpu1\":{\"ax\":" + String(ax1) +
                ",\"ay\":" + String(ay1) +
                ",\"az\":" + String(az1) +
                ",\"gx\":" + String(gx1) +
                ",\"gy\":" + String(gy1) +
                ",\"gz\":" + String(gz1) + "}," +
                "\"mpu2\":{\"ax\":" + String(ax2) +
                ",\"ay\":" + String(ay2) +
                ",\"az\":" + String(az2) +
                ",\"gx\":" + String(gx2) +
                ",\"gy\":" + String(gy2) +
                ",\"gz\":" + String(gz2) + "}" +
                "}";

  // BLE Üzerinden Verileri Gönder
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();
  Serial.println("Veri gönderildi: " + data);

  // 500ms bekle
  delay(100);
}