#include <Wire.h>
#include <MPU6050.h>
#include <DFRobot_QMC5883.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


MPU6050 mpu1(0x68); // AD0 GND'ye bağlı olan sensör
MPU6050 mpu2(0x69); // İkinci sensör

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-123456789abc"

// BLE Karakteristik
BLECharacteristic *pCharacteristic;

// BLE Sunucusu
BLEServer *pServer;

bool deviceConnected = false;

DFRobot_QMC5883 compass(&Wire, /*I2C addr*/QMC5883_ADDRESS);


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client connected.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client disconnected.");
    // Restart advertising when disconnected
    pServer->startAdvertising();
    Serial.println("Restarted advertising.");
  }
};


void setup() {
  
  Serial.begin(115200);
  
  Wire.begin(21, 22);//I2C

  mpu1.initialize();
  mpu2.initialize();

  bool compassInitialized = compass.begin();

  if (mpu1.testConnection() && mpu2.testConnection() && compassInitialized) {
    Serial.println("Both MPU6050 connections and gyro successful");
  } else {
    Serial.println(" connection failed");
    while (true);  // Stop execution if connection fails
  }

  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  compass.setDeclinationAngle(declinationAngle);

  // BLE initialization
  BLEDevice::init("ESP32-6");//change name of the esp
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  //pAdvertising->setMinInterval(100);
  //pAdvertising->setMaxInterval(200);
  pAdvertising->start();
  Serial.println("BLE advertising started.");

}

void loop() {

  if (deviceConnected) {
    int16_t ax1, ay1, az1;
    int16_t gx1, gy1, gz1;
    int16_t ax2, ay2, az2;
    int16_t gx2, gy2, gz2;

    // MPU readings
    mpu1.getMotion6(&ax1, &ay1, &az1, &gx1, &gy1, &gz1);
    mpu2.getMotion6(&ax2, &ay2, &az2, &gx2, &gy2, &gz2);

    // Magnetometer readings
    sVector_t mag = compass.readRaw();
    compass.getHeadingDegrees();

    Serial.print("MPU1 -> a/g: ");
    Serial.print("ax1: ");Serial.print(ax1); Serial.print("\t");
    Serial.print("ay1: ");Serial.print(ay1); Serial.print("\t");
    Serial.print("az1: ");Serial.print(az1); Serial.print("\t");
    Serial.print("gx1: ");Serial.print(gx1); Serial.print("\t");
    Serial.print("gy1: ");Serial.print(gy1); Serial.print("\t");
    Serial.print("gz1: "); Serial.println(gz1);

    Serial.print("MPU2 -> a/g: ");
    Serial.print("ax2: ");Serial.print(ax2); Serial.print("\t");
    Serial.print("ay2: ");Serial.print(ay2); Serial.print("\t");
    Serial.print("az2: ");Serial.print(az2); Serial.print("\t");
    Serial.print("gx2: ");Serial.print(gx2); Serial.print("\t");
    Serial.print("gy2: ");Serial.print(gy2); Serial.print("\t");
    Serial.print("gz2: ");Serial.println(gz2);

    Serial.print("HMC -> X: ");
    Serial.print(mag.XAxis); Serial.print("\t");
    Serial.print("Y: ");
    Serial.print(mag.YAxis); Serial.print("\t");
    Serial.print("Z: ");
    Serial.print(mag.ZAxis); Serial.print("\t");
    Serial.print("Heading (degrees): ");
    Serial.println(mag.HeadingDegress);

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
                  ",\"HMCx\":" + String(mag.XAxis) +
                  ",\"HMCy\":" + String(mag.YAxis) +
                  ",\"HMCz\":" + String(mag.ZAxis) +
                  ",\"Heading\":" + String(mag.HeadingDegress) +
                  "}";

    pCharacteristic->setValue(data.c_str());
    pCharacteristic->notify();
    Serial.println("Data sent: " + data);

    delay(500); //kaldır galiba araştır
  }
}