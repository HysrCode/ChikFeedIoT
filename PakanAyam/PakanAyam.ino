#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "time.h"
#include "sntp.h"
#include "EEPROM.h"
#include <Wire.h>
#include <DS3231.h>

RTClib myRTC;

// Pin Definitions
#define ONE_WIRE_BUS 19
#define DHTPIN 15       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22
#define LDR_PIN1 32     // LDR pin 1
#define LDR_PIN2 35     // LDR pin 2
#define LDR_PIN3 34     // LDR pin 3
#define LED_PIN1 33     // LED pin 1
#define LED_PIN2 25     // LED pin 2
#define LED_PIN3 28     // LED pin 3
#define SERVO_PIN 5     // Servo pin
#define EEPROM_SIZE 64  // Eprom Size (64 kb)
// WiFi Credentials
const char* WIFI_SSID = "Mia";
const char* WIFI_PASSWORD = "12345678";

// Firebase Credentials
const char* FIREBASE_HOST = "iot-feeder-f8e4f-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_AUTH = "38190e2c3fafb7bfca6bc2381dbc8f629e034d5c";

// Objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DSTemp(&oneWire);
DHT dht(DHTPIN, DHTTYPE);
Servo servo;
FirebaseData firebaseData;
FirebaseJson json;

// Variables
unsigned long p1, p2, p3, p4, p5, p6;
int temperatureDHT, temperatureDS, humidity;
int dataLDR = 100;
const int minRange = 0;
const int maxRange = 100;
String defult, time1, time2, time3, time4, time5;
void setup() {
  Serial.begin(9600);

  p1 = millis();
  p2 = millis();
  p3 = millis();
  p4 = millis();
  p5 = millis();

  pinMode(LDR_PIN1, INPUT);
  pinMode(LDR_PIN2, INPUT);
  pinMode(LDR_PIN3, INPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  servo.attach(SERVO_PIN);

  Wire.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  waitForWifiConnection();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  DSTemp.begin();
  dht.begin();

  EEPROM.begin(EEPROM_SIZE);
}

void loop() {
  unsigned long ct = millis();

  DateTime now = myRTC.now();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();
  String waktuSekarang = String(hour) + ":" + String(minute) + ":" + String(second);

  // Check WiFi connection every 5 seconds
  if (ct - p1 >= 5000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost Connection");
      waitForWifiConnection();
    }
    p1 = ct;
  }

  // Read sensor data and control LEDs every 1 second
  if (ct - p2 >= 1000) {
    readLDR();
    readSensorData();
    p2 = ct;
  }

  // Upload sensor data to Firebase every 2 seconds
  if (ct - p3 >= 2000) {
    if (WiFi.status() == WL_CONNECTED) {
      uploadSensorData(waktuSekarang);
    }
    p3 = ct;
  }

  // Control servo based on Firebase data every 2 seconds
  if (ct - p4 >= 1000) {
    if (WiFi.status() == WL_CONNECTED) {
      controlServo();
    }
    p4 = ct;
  }

  if (ct - p5 >= 60000) {
    if (WiFi.status() == WL_CONNECTED) {
      uploadHistory(waktuSekarang);
    }
    p5 = ct;
  }

  if (ct - p6 >= 20000) {
    if (WiFi.status() == WL_CONNECTED) {
      schedule(time1);
      schedule(time2);
      schedule(time3);
      schedule(time4);
      schedule(time5);
    }
    p6 = ct;
  }

}

void waitForWifiConnection() {
  for (int attempts = 0; attempts < 10; attempts++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi!");
      break;
    }
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void readSensorData() {
  temperatureDHT = dht.readTemperature();
  humidity = dht.readHumidity();
  DSTemp.requestTemperatures();
  temperatureDS = DSTemp.getTempCByIndex(0);
  if (isnan(temperatureDHT) || isnan(humidity) || temperatureDS < 0) {
    temperatureDHT = 0;
    humidity = 0;
    temperatureDS = 0;
  }
}

void readLDR() {
  int ldrVal1 = analogRead(LDR_PIN1);
  int ldrVal2 = analogRead(LDR_PIN2);
  int ldrVal3 = analogRead(LDR_PIN3);
  Serial.println(ldrVal1);
  Serial.println(ldrVal2);
  Serial.println(ldrVal3);

  digitalWrite(LED_PIN1, HIGH);
  digitalWrite(LED_PIN2, HIGH);
  digitalWrite(LED_PIN3, HIGH);

  if (ldrVal1 <= 2000) {
    dataLDR = 75;
    Serial.println("Pakan Tinggal 75%");
  } else if (ldrVal2 <= 2000) {
    dataLDR = 50;
    Serial.println("Pakan Tinggal 50%");
  } else if (ldrVal3 <= 2000 && ldrVal3 > 1000) {
    dataLDR = 20;
    Serial.println("akan Tinggal 20%");
  } else if (ldrVal3 <= 3000 && ldrVal2 <= 3000 && ldrVal1 <= 3000) {
    dataLDR = 0;
    Serial.println("Pakan Ha bis");
  }
  else {
    dataLDR = 100;
    Serial.println("Pakan Terisi");
  }
}
void uploadSensorData(const String& waktuSekarang) {
  json.clear();
  json.set("/feed", dataLDR);
  json.set("/temp", temperatureDHT);
  json.set("/humidity", humidity);
  json.set("/feedTemp", temperatureDS);

  Firebase.setString(firebaseData, "/timeNow", waktuSekarang);

  if (Firebase.setTimestamp(firebaseData, "/timestamp")) {
    int timestamp = firebaseData.intData();
    json.set("/time", timestamp);
    Firebase.set(firebaseData, "/sensors", json);
  } else {
    Serial.print("Gagal menulis data ke Firebase. Kesalahan: ");
    Serial.println(firebaseData.errorReason());
  }
}

void uploadHistory(const String& waktuSekarang) {
  json.clear();
  json.set("/feed", dataLDR);
  json.set("/temp", temperatureDHT);
  json.set("/humidity", humidity);
  json.set("/feedTemp", temperatureDS);
  if (Firebase.get(firebaseData, "/timestamp")) {
    int timestamp = firebaseData.intData();
    json.set("/time", timestamp);
    Firebase.push(firebaseData, "use/history", json);
  } else {
    Serial.print("Gagal menulis data ke Firebase. Kesalahan: ");
    Serial.println(firebaseData.errorReason());
  }
}

void schedule(const String& times) {
  //schedule set from firebase
  dafult = "0:0:0";
  String timeSaved = EEPROM.read(eepromAddress);
  if (Firebase.get(firebaseData, "/time/1/data") && Firebase.get(firebaseData, "/time/1/set")) {
    String getTimes = firebaseData.stringData();
    bool getTime1State = firebaseData.boolData();
    if (timeSaved != getTimes) {
      EEPROM.writeString(0, getTimes);
      EEPROM.commit();
      timeSaved = getTimes;
    }
    if (getTime1State && timeSaved == times) {
      controlServo();
    }
  }
}

void controlServo() {
  if (Firebase.get(firebaseData, "/servo")) {
    int servoState = firebaseData.intData();
    if (servoState == 1) {
      openServo();
      delay(1000);
      closeServo();
    }
    else {
      servo.write(0);
    }
  }
}

void openServo() {
  for (int angle = 0; angle <= 150; angle += 50) {
    servo.write(angle);
    delay(50);  // Tunggu selama 50 milidetik sebelum mengubah posisi servo
  }

}

void closeServo() {
  for (int angle = 150; angle >= 0; angle -= 50) {
    servo.write(angle);
    delay(50);  // Tunggu selama 50 milidetik sebelum mengubah posisi servo
  }
  Firebase.setInt(firebaseData, "/servo", 0);
}
