#include <Wire.h>
#include <MPU6050.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define pins and connections
const char* ssid = "YEY";
const char* password = "qwerty12345Y";
const char* mqtt_server = "192.168.3.146";  // Raspberry Pi IP address
const int mqtt_port = 1883;  // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x69);

// Setup function
void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  // Connect to MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT connected");
    } else {
      delay(1000);
      Serial.println("Attempting MQTT connection...");
    }
  }

  // Initialize MPU6050 sensors
  Wire.begin();
  mpu1.initialize();
  mpu2.initialize();
  if (!mpu1.testConnection() || !mpu2.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }
}

// Send sensor data function
void sendData() {
  int16_t ax1, ay1, az1;
  int16_t gx1, gy1, gz1;
  int16_t ax2, ay2, az2;
  int16_t gx2, gy2, gz2;
  int16_t temp1, temp2;

  // Read data from the first MPU6050
  mpu1.getAcceleration(&ax1, &ay1, &az1);
  mpu1.getRotation(&gx1, &gy1, &gz1);
  temp1 = mpu1.getTemperature();

  // Read data from the second MPU6050
  mpu2.getAcceleration(&ax2, &ay2, &az2);
  mpu2.getRotation(&gx2, &gy2, &gz2);
  temp2 = mpu2.getTemperature();

  // Calculate temperature in Celsius (temperature is in raw value, apply formula)
  float tempC1 = (temp1 / 340.0) + 36.53;
  float tempC2 = (temp2 / 340.0) + 36.53;

  // Print data to the Serial Monitor
  Serial.println("Sending sensor data:");
  Serial.print("MPU1 - Acc: ");
  Serial.print(ax1); Serial.print(", ");
  Serial.print(ay1); Serial.print(", ");
  Serial.print(az1); Serial.print(" | ");
  Serial.print("Gyro: ");
  Serial.print(gx1); Serial.print(", ");
  Serial.print(gy1); Serial.print(", ");
  Serial.print(gz1); Serial.print(" | ");
  Serial.print("Temp: ");
  Serial.println(tempC1);

  Serial.print("MPU2 - Acc: ");
  Serial.print(ax2); Serial.print(", ");
  Serial.print(ay2); Serial.print(", ");
  Serial.print(az2); Serial.print(" | ");
  Serial.print("Gyro: ");
  Serial.print(gx2); Serial.print(", ");
  Serial.print(gy2); Serial.print(", ");
  Serial.print(gz2); Serial.print(" | ");
  Serial.print("Temp: ");
  Serial.println(tempC2);

  // Publish data to MQTT topics for both sensors
  String mpu1Data = String(ax1) + "," + String(ay1) + "," + String(az1) + "," + String(gx1) + "," + String(gy1) + "," + String(gz1) + "," + String(tempC1);
  String mpu2Data = String(ax2) + "," + String(ay2) + "," + String(az2) + "," + String(gx2) + "," + String(gy2) + "," + String(gz2) + "," + String(tempC2);

  Serial.println("Publishing MPU1 data...");
  client.publish("sensor/mpu1", mpu1Data.c_str()); // Send data for MPU1
  delay(200);  // Add a delay to prevent flooding

  Serial.println("Publishing MPU2 data...");
  client.publish("sensor/mpu2", mpu2Data.c_str()); // Send data for MPU2
  delay(200);  // Add a delay between both publishes
}

void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      if (client.connect("ESP8266Client")) {
        Serial.println("MQTT reconnected");
      } else {
        delay(1000);
        Serial.println("Attempting MQTT connection...");
      }
    }
  }
  client.loop();

  // Send sensor data every 1 second
  sendData();
  delay(1000);  // Send data every 1 second
}
