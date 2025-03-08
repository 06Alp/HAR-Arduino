#include <Wire.h>
#include <MPU6050.h>
#include <DFRobot_QMC5883.h>


MPU6050 mpu1(0x68); // AD0 GND'ye bağlı olan sensör
MPU6050 mpu2(0x69); // İkinci sensör

DFRobot_QMC5883 compass(&Wire, /*I2C addr*/HMC5883L_ADDRESS);

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

}

void loop() {
  int16_t ax1, ay1, az1;
  int16_t gx1, gy1, gz1;
  int16_t ax2, ay2, az2;
  int16_t gx2, gy2, gz2;

  // MPU'lar 
  mpu1.getMotion6(&ax1, &ay1, &az1, &gx1, &gy1, &gz1);
  mpu2.getMotion6(&ax2, &ay2, &az2, &gx2, &gy2, &gz2);

  //HMC
  sVector_t mag = compass.readRaw();
  compass.getHeadingDegrees();


  Serial.print("MPU1 -> a/g: ");
  Serial.print(ax1); Serial.print("\t");
  Serial.print(ay1); Serial.print("\t");
  Serial.print(az1); Serial.print("\t");
  Serial.print(gx1); Serial.print("\t");
  Serial.print(gy1); Serial.print("\t");
  Serial.println(gz1);

  Serial.print("MPU2 -> a/g: ");
  Serial.print(ax2); Serial.print("\t");
  Serial.print(ay2); Serial.print("\t");
  Serial.print(az2); Serial.print("\t");
  Serial.print(gx2); Serial.print("\t");
  Serial.print(gy2); Serial.print("\t");
  Serial.println(gz2);

  Serial.print("HMC -> X: ");
  Serial.print(mag.XAxis); Serial.print("\t");
  Serial.print("Y: ");
  Serial.print(mag.YAxis); Serial.print("\t");
  Serial.print("Z: ");
  Serial.print(mag.ZAxis); Serial.print("\t");
  Serial.print("Heading (degrees): ");
  Serial.println(mag.HeadingDegress);

  Serial.println();

  delay(500);

}
