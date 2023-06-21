#include <WiFi.h>
#include <FirebaseESP32.h>

const int LDR_PIN1 = 32;
const int LDR_PIN2 = 35;
const int LDR_PIN3 = 34;
const int LED_PIN1 = 33;
const int LED_PIN2 = 26;
const int LED_PIN3 = 28;

const char* WIFI_SSID = "Mia";
const char* WIFI_PASSWORD = "12345678";

const char* FIREBASE_HOST = "pakanayamrtdb-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_AUTH = "ad683ccba7203140aa07568385caa651ea2e81aa";

FirebaseData firebaseData;

void setup() {
  Serial.begin(9600);

  pinMode(LDR_PIN1, INPUT);
  pinMode(LDR_PIN2, INPUT);
  pinMode(LDR_PIN3, INPUT);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  int ldr1Value = analogRead(LDR_PIN1);
  int ldr2Value = analogRead(LDR_PIN2);
  int ldr3Value = analogRead(LDR_PIN3);

  Firebase.setInt(firebaseData, "/sensor/value", ldr1Value);

  if(Firebase.getInt(firebaseData, "/sensor/value")){
    int value = firebaseData.intData();
    if (ldr1Value < 1000) {
    digitalWrite(LED_PIN1, HIGH);  // Menyalakan LED jika nilai LDR < 1000
    } else {
      digitalWrite(LED_PIN1, LOW);   // Mematikan LED jika nilai LDR >= 1000
    }
    Serial.println("Data dari Data Base = "+ value);
  }

  
  Serial.println("Data set Firebase = " + ldr1Value);
  

  delay(1000);
}
