#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "time.h"
#include "sntp.h"


//#include <RtcDS1302.h>

#define ONE_WIRE_BUS 10

#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature DSTemp(&oneWire); // Pass our oneWire reference to Dallas Temperature.
//
//DHT dht(DHTPIN, DHTTYPE); //for DHT

//void printDateTime(const RtcDateTime& dt);

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

//millis
unsigned long p1, p2, p3, p4, p5, p6;

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
//
//ThreeWire myWire(4, 5, 2);
//RtcDS1302<ThreeWire> Rtc(myWire);

Servo servo;

FirebaseData firebaseData;

void setup() {
  Serial.begin(57600);

  p1 = millis();
  p2 = millis();
  p3 = millis();
  p4 = millis();
  p5 = millis();
  p6 = millis();

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

  // Inisialisasi modul RTC
  //  rtc.Begin();
  //  rtc.SetIsWriteProtected(false);
  //  rtc.SetIsRunning(true);

  // Set waktu awal jika diperlukan
  // rtc.SetDateTime(RtcDateTime(2023, 6, 22, 12, 30, 0)); // Tahun, Bulan, Tanggal, Jam, Menit, Detik


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //  DSTemp.begin();
  //  dht.begin();
}

void loop() {
  updateTime();
  unsigned long ct = millis();
  //  // Read temperature(t) and humidty(h) as Celsius(the default) : DHT
  //  float h = dht.readHumidity();
  //  float t = dht.readTemperature();
  //  Serial.print("Suhu Ruagnan : ");
  //  Serial.println(t);
  //  Serial.print("Kelembapan Ruangan : ");
  //  Serial.println(h);
  //  // Read temperature from DS
  //  DSTemp.requestTemperatures();
  //  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  //  float suhuPakan = DSTemp.getTempCByIndex(0);
  //  Serial.print("Suhu Pakan : ");
  //  Serial.println(suhuPakan);

  //async process

  // Check Wifi State (Process)
  if (ct - p1 >= 1000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost Connection");
    }
    p1 = ct;
  }

  //Get Time From RTC (Process)
  //  if (ct - p2 >= 1000) {
  //    RtcDateTime now = Rtc.GetDateTime();
  //    printDateTime(now);
  //    Serial.println();
  //    p2 = ct;
  //    delay(1000);
  //  }


  //  //Control LED & LDR as Feed Volume meters (Process) : Offline
  //  if (ct - p3 >= 1000) {
  //    int ldrVal1 = analogRead(LDR_PIN1);
  //    int ldrVal2 = analogRead(LDR_PIN2);
  //    int ldrVal3 = analogRead(LDR_PIN3);
  //    Serial.println(ldrVal1);
  //    Serial.println(ldrVal2);
  //    Serial.println(ldrVal3);
  //
  //    digitalWrite(LED_PIN1, HIGH);
  //    digitalWrite(LED_PIN2, HIGH);
  //    digitalWrite(LED_PIN3, HIGH);
  //
  //    if (ldrVal1 <= 1000) {
  //      dataLDR = 50;
  //      Serial.println("Pakan Tinggal 50%");
  //      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
  //    } else if (ldrVal2 <= 1000) {
  //      dataLDR = 25;
  //      Serial.println("Pakan Tinggal 25%");
  //      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
  //    } else if (ldrVal3 <= 1000) {
  //      dataLDR = 0;
  //      Serial.println("Pakan Telah Habis");
  //      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
  //    } else {
  //      Serial.println("Pakan Terisi");
  //    }
  //
  //    p3 = ct;
  //  }

  //Set Temp and Humy to Firebase (Process) : Onlite
  if (ct - p4 >= 1000) {
    float randomFloat = random() / static_cast<float>(RAND_MAX);

    // Mengubah rentang bilangan float acak menjadi rentang yang diinginkan
    float minVal = 10.0;  // Nilai minimum rentang
    float maxVal = 20.0;  // Nilai maksimum rentang
    float randomFloatInRange = minVal + randomFloat * (maxVal - minVal);

    //Suhu Dalam Pakan
    //Suhu d Kelembapan Ruangan
    if (WiFi.status() == WL_CONNECTED) {
      if (Firebase.setFloat(firebaseData, "/suhu/ruangan", randomFloatInRange) && Firebase.setFloat(firebaseData, "/suhu/pakan", randomFloatInRange) && Firebase.setFloat(firebaseData, "/kelembapan", randomFloatInRange)) {
        Serial.println("Data berhasil ditulis ke Firebase!");
      } else {
        Serial.println("Gagal menulis data ke Firebase. Kesalahan: " + firebaseData.errorReason());
      }
    }
    p4 = ct;
  }

  //Get Data from Firebase (Process)
  if (ct - p5 >= 1000) {
    if (WiFi.status() == WL_CONNECTED) {
      if (Firebase.getFloat(firebaseData, "/suhu/ruangan")) {
        float suhuRuangan = firebaseData.floatData();
        Serial.print("Suhu Ruangan: ");
        Serial.println(suhuRuangan, 2);
      }

      if (Firebase.getFloat(firebaseData, "/suhu/pakan")) {
        float suhuPakan = firebaseData.floatData();
        Serial.print("Suhu Pakan: ");
        Serial.println(suhuPakan, 2);
      }

      if (Firebase.getFloat(firebaseData, "/kelembapan")) {
        float kelembapan = firebaseData.floatData();
        Serial.print("Kelembapan: ");
        Serial.println(kelembapan, 2);
      }

      if (Firebase.getFloat(firebaseData, "/waktu")) {
        String waktu = firebaseData.stringData();
        Serial.print("Waktu dari Firebase: ");
        Serial.println(waktu);
      }
    }
    p5 = ct;
  }

  if (ct - p6 >= 1000) {
    updateTime();
    // Format waktu menjadi string
    String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);

    // Masukkan waktu ke Firebase Realtime Database
    
    if (Firebase.setString(firebaseData, "/waktu", timeStr)) {
      Serial.println("Waktu berhasil disimpan di Firebase!");
    } else {
      Serial.println("Gagal menyimpan waktu di Firebase.");
      Serial.println("Kesalahan: " + firebaseData.errorReason());
    }
  }
}
