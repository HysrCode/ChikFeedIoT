#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "time.h"
#include "sntp.h"
#define ONE_WIRE_BUS 19
#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DSTemp(&oneWire); // Pass our oneWire reference to Dallas Temperature.
DHT dht(DHTPIN, DHTTYPE); //for DHT

//Time ESP
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 28800;    // GMT offset in seconds (Waktu Standar Tengah Indonesia: UTC+8)
int hour, minute, second;

void updateTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  updateTime();
}

//Variabel
unsigned long p1, p2, p3, p4, p5, p6;

float temperatureDHT = 0.0;
float humidity = 0.0;
float temperatureDS = 0.0;

int dataLDR = 100;
const int LDR_PIN1 = 32;
const int LDR_PIN2 = 35;
const int LDR_PIN3 = 34;
const int minRange = 0;
const int maxRange = 100;
const int LED_PIN1 = 33;
const int LED_PIN2 = 25;
const int LED_PIN3 = 28;
const int SERVO_PIN = 5;

const char* WIFI_SSID = "Mia";
const char* WIFI_PASSWORD = "12345678";

const char* FIREBASE_HOST = "iot-feeder-f8e4f-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_AUTH = "38190e2c3fafb7bfca6bc2381dbc8f629e034d5c";

//Objek
Servo servo;

FirebaseData firebaseData;
FirebaseJson json;

void setup() {
  Serial.begin(9600);

  p1 = millis();
  p2 = millis();
  p3 = millis();
//  p4 = millis();
//  p5 = millis();
//  p6 = millis();

  pinMode(LDR_PIN1, INPUT);
  pinMode(LDR_PIN2, INPUT);
  pinMode(LDR_PIN3, INPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  servo.attach(SERVO_PIN);

  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  // NTP server address could be acquired via DHCP
  sntp_servermode_dhcp(1);    // (optional)
  configTime(gmtOffset_sec, 0, ntpServer1, ntpServer2);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int attempts = 0; attempts < 10; attempts++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi!");
      break;
    }
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  DSTemp.begin();
  dht.begin();
}

void loop() {
  unsigned long ct = millis();
  int timestamp = firebaseData.to<int>();
  // Check Wifi after 5 seconds State (Process)
  if (ct - p1 >= 5000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost Connection");
    }
    p1 = ct;
  }

  //Control LED & LDR as Feed Volume meters (Process) : Offline
  if (ct - p3 >= 1000) {
    temperatureDHT = dht.readTemperature();
    humidity = dht.readHumidity();
    DSTemp.requestTemperatures();
    temperatureDS = DSTemp.getTempCByIndex(0);
    Serial.println(temperatureDHT);
    Serial.println(humidity);
    Serial.println(temperatureDS);

    int ldrVal1 = analogRead(LDR_PIN1);
    int ldrVal2 = analogRead(LDR_PIN2);
    int ldrVal3 = analogRead(LDR_PIN3);
    Serial.println(ldrVal1);
    Serial.println(ldrVal2);
    Serial.println(ldrVal3);
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    if (ldrVal1 <= 1000) {
      dataLDR = 50;
      Serial.println("Pakan Tinggal 50%");
    } else if (ldrVal2 <= 1000) {
      dataLDR = 25;
      Serial.println("Pakan Tinggal 25%");
    } else if (ldrVal3 <= 1000) {
      dataLDR = 0;
      Serial.println("Pakan Telah Habis");
    } else {
      Serial.println("Pakan Terisi");
    }
    p3 = ct;
  }

  //Set Temp and Humy to Firebase (Process) : Onlite
  if (ct - p4 >= 2000) {
    if (WiFi.status() == WL_CONNECTED) {
      //data ldr
      json.set("/isiPakan", dataLDR);
      //data dht & ds
      json.set("/temp", temperatureDHT);
      json.set("/humidity", humidity);
      json.set("/feedTemp", temperatureDS);
      
      if (Firebase.setTimestamp(firebaseData, "/timestamp")){
        int timestamp = firebaseData.intData();
        json.set("/waktu", timestamp);
        Serial.println(Firebase.set(firebaseData, "/sensor", json) ? "Uploaded" : "Not Uploaded");
      } else {
        Serial.print("Gagal menulis data ke Firebase. Kesalahan:. Kesalahan: ");
        Serial.println(firebaseData.errorReason());
      }
    }
    p4 = ct;
  }
}
