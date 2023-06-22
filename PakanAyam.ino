#include <WiFi.h>
#include <FirebaseESP32.h>
#include <ESP32Servo.h>
//#include <RtcDS1302.h>

//void printDateTime(const RtcDateTime& dt);

unsigned long p1, p2, p3, p4, p5;

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
}

void loop() {
  unsigned long ct = millis();

  if (ct - p1 >= 1000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Lost Connection");
    }
    p1 = ct;
  }

//  if (ct - p2 >= 1000) {
//    RtcDateTime now = Rtc.GetDateTime();
//    printDateTime(now);
//    Serial.println();
//    p2 = ct;
//    delay(1000);
//  }

  if (ct - p3 >= 1000) {
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
      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
    } else if (ldrVal2 <= 1000) {
      dataLDR = 25;
      Serial.println("Pakan Tinggal 25%");
      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
    } else if (ldrVal3 <= 1000) {
      dataLDR = 0;
      Serial.println("Pakan Telah Habis");
      Firebase.setInt(firebaseData, "/isiPakan", dataLDR);
    } else {
      Serial.println("Pakan Terisi");
    }

    p3 = ct;
    delay(1000);
  }

  if (ct - p4 >= 1000) {
    int data = random(1, 20);

    if (Firebase.setInt(firebaseData, "/path/suhu", data) && Firebase.setInt(firebaseData, "/path/kelembapan", data)) {
      Serial.println("Data berhasil ditulis ke Firebase!");
    } else {
      Serial.println("Gagal menulis data ke Firebase. Kesalahan: " + firebaseData.errorReason());
    }

    if (Firebase.getInt(firebaseData, "/servo")) {
      int servoState = firebaseData.intData();
      Serial.println("Servo State : " + servoState);
      if(servoState == 1){
        servo.write(150);
      }else{
        servo.write(0);  
      }
    }

    p4 = ct;
  }
}
