#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Replace these with your network and MQTT details
const char* ssid = "YEY";
const char* password = "qwerty12345Y";
const char* mqtt_server = "192.168.3.146"; // IP address of your Raspberry Pi

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MPU6050 mpu;

// MQTT topics
const char* mqtt_topic = "sensor/data";

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Setup MQTT client
  client.setServer(mqtt_server, 1883);
  reconnectMQTT();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);


  // Prepare JSON payload with sensor data
  String payload = "{";
  payload += "\"accel_x\":" + String(accel.acceleration.x, 2) + ",";
  payload += "\"accel_y\":" + String(accel.acceleration.y, 2) + ",";
  payload += "\"accel_z\":" + String(accel.acceleration.z, 2) + ",";
  payload += "\"gyro_x\":" + String(gyro.gyro.x, 2) + ",";
  payload += "\"gyro_y\":" + String(gyro.gyro.y, 2) + ",";
  payload += "\"gyro_z\":" + String(gyro.gyro.z, 2) + ",";
  payload += "\"temperature\":" + String(temp.temperature, 2);
  payload += "}";

  // Publish the data to MQTT topic
  if (client.publish(mqtt_topic, (char*) payload.c_str())) {
    Serial.println("Data sent to MQTT broker: ");
    Serial.println(payload);
  } else {
    Serial.println("Failed to send data to MQTT broker");
  }

  delay(1000); // Send data every second
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

