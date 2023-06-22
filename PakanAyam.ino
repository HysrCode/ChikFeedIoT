#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Servo.h>
#include <RtcDS1302.h>

void printDateTime(const RtcDateTime& dt);

//millis var
unsigned long p1, p2, p3, p4, p5; //p for process,

const int LDR_PIN1 = 32;
const int LDR_PIN2 = 35;
const int LDR_PIN3 = 34;
const int minRange = 0;
const int maxRange = 100;
const int LED_PIN1 = 33;
const int LED_PIN2 = 26;
const int LED_PIN3 = 28;
const int SERVO_PIN = 5;

// DS1302 RTC module pin connections
const int RST_PIN = 14;    // RST - Reset
const int DAT_PIN = 27;    // DAT - Data
const int CLK_PIN = 26;    // CLK - Clock

const char* WIFI_SSID = "Mia";
const char* WIFI_PASSWORD = "12345678";

const char* FIREBASE_HOST = "pakanayamrtdb-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_AUTH = "c53269dad3afd66777e106fd41c6c809b9ad33fa"; //

ThreeWire myWire(4, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

Servo.servo;

FirebaseData firebaseData;

void setup() {
  Serial.begin(9600);

  p1 = millis();
  p2 = millis();
  p3 = millis();
  p4 = millis();
  p5 = millis();

  //LDR & LED pinmode
  pinMode(LDR_PIN1, INPUT);
  pinMode(LDR_PIN2, INPUT);
  pinMode(LDR_PIN3, INPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  //Servo attach
  servo.attach(SERVO_PIN);

  //RTC
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  if (!Rtc.IsDateTimeValid())
  {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }

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

  //check wifi connection
  if (ct - p1 >= 1000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("LLost Conection");
    }
    p1=ct;
  }

  //rtc
  if (ct - p2 >= 1000) {
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();
    p2=ct;
         delay(1000);
  }

  //LDR & LED
  if (ct - p3 >= 1000) {
    int LDR1 = map(LDR_PIN1, 0, 4065, minRange, maxRange);
    int LDR2 = map(LDR_PIN2, 0, 4065, minRange, maxRange);
    int LDR3 = map(LDR_PIN3, 0, 4065, minRange, maxRange);
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);

    //Check LDR Respon
    if (LDR1 >= 60) {
      Serial.println("Pakan Tinggal 50%");
    } else if (LDR2 >= 60) {
      Serial.println("Pakan Tinggal 25%");
    } else if (LDR3 >= 60) {
      Serial.println("Pakan Habis");
    } else {
      Serial.println("Pakan Terisi");
    }
    p3 = ct;
    delay(2000);
  }

  //sensor
  if (ct - p4 >= 1000) {
    // Menulis data ke Firebase
    int data = random(1, 10); // Ganti dengan data yang ingin Anda tulis ke Firebase
    if (Firebase.setInt(firebaseData, "/path/data", data)) {
      Serial.println("Data berhasil ditulis ke Firebase!");
    } else {
      Serial.println("Gagal menulis data ke Firebase. Kesalahan: " + firebaseData.errorReason());
    }

    if (Firebase.getInt(firebaseData, "/path/data")) {
      int value = firebaseData.intData();
      Serial.println(value);
    }
    p4 = ct;
  }
}
