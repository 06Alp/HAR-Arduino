#include <Wire.h>
#include <MPU6050.h>
#include <DFRobot_QMC5883.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// MPU6050 Addresses
MPU6050 mpu1(0x68); // First MPU6050
MPU6050 mpu2(0x69); // Second MPU6050

// GY-271 (HMC5883L) Magnetometer
DFRobot_QMC5883 compass(&Wire, /*I2C addr*/QMC5883_ADDRESS);

// BLE UUIDs
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-123456789abc"

// BLE Characteristic
BLECharacteristic *pCharacteristic;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL for ESP32 I2C

  // Initialize MPU6050 sensors
  mpu1.initialize();
  mpu2.initialize();

  if (!mpu1.testConnection() || !mpu2.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (true);
  }
  Serial.println("MPU6050 sensors initialized.");

  // Initialize the compass
  if (!compass.begin()) {
    Serial.println("GY-271 (HMC5883L) connection failed!");
    while (true);
  }
  Serial.println("GY-271 (HMC5883L) initialized.");

  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  compass.setDeclinationAngle(declinationAngle);

  // BLE Setup
  BLEDevice::init("ESP32-2");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE started.");
}

void loop() {
  // Read data from MPU6050 sensors
  int16_t ax1, ay1, az1, gx1, gy1, gz1;
  int16_t ax2, ay2, az2, gx2, gy2, gz2;

  mpu1.getMotion6(&ax1, &ay1, &az1, &gx1, &gy1, &gz1);
  mpu2.getMotion6(&ax2, &ay2, &az2, &gx2, &gy2, &gz2);

  // Read data from the magnetometer
  sVector_t mag = compass.readRaw();
  float headingDegrees = compass.getHeadingDegrees();

  // Create a JSON payload
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
                ",\"gz\":" + String(gz2) + "}," +
                "\"compass\":{\"x\":" + String(mag.XAxis) +
                ",\"y\":" + String(mag.YAxis) +
                ",\"z\":" + String(mag.ZAxis) +
                ",\"heading\":" + String(headingDegrees) + "}" +
                "}";

  // Send data via BLE
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();

  // Log the data to the Serial Monitor
  Serial.println("Sent data: " + data);

  delay(1000); // Adjust delay as needed
}
