#include <Arduino.h>
#include <ESP8266WiFi.h.>
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <string.h>
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// dht11
#include "DHT.h"
#include "Servo.h"
#include "Adafruit_Sensor.h"
DHT dht2(2, DHT11);
#define WIFI_SSID "Samay's Phone"
#define WIFI_PASSWORD "aad_0352"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCFTXjoIIZgmFWGNGDicT7Fy5QP4Rx96Ts"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://hydroponicsiot-e2ffc-default-rtdb.asia-southeast1.firebasedatabase.app/"
FirebaseData fbdo;
FirebaseData gbdo;

FirebaseAuth auth;
FirebaseConfig config;

uint8_t Pwm1 = D1;
int a0 = 15;      // Gpio-15 of nodemcu esp8266
int a1 = 13;      // Gpio-13 of nodemcu esp8266
int l1 = 16;      // led pin
uint8_t wl1 = 12; // water level pin
bool signupOK = false;

int s1 = 12;
int s2 = 14;
Servo myservo;
float c1 = 320.00;
float c2 = 0.3;
float c3 = 24;
unsigned long sendDataPrevMillis = 0;

// ultrasonic and temp sensor
float temp;
float humidity;
const int check = 15;
long duration; // variable for the duration of sound wave travel
int distance;
// const int trigPin = 12;
// const int echoPin = 14;
// char path[]="Locations/PeriyarRiver";
void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  SPI.begin(); // Init SPI bus
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  myservo.attach(4);
  dht2.begin();
  pinMode(a0, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(l1, OUTPUT);
  pinMode(wl1, INPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
}

void loop()
{

  digitalWrite(s2, LOW);
  digitalWrite(s1, HIGH);
  distance = analogRead(a0);
  Serial.print("Water Level:");
  Serial.println(distance);

  temp = dht2.readTemperature();
  delay(2000);
  humidity = dht2.readHumidity();
  delay(2000);
  Serial.print("temp:");
  Serial.println(temp);
  Serial.print("Humidity:");
  Serial.println(humidity);

  digitalWrite(s1, LOW);
  digitalWrite(s2, HIGH);
  int sensorValue = analogRead(A0);             // read the input on analog pin 0
  float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
  Serial.println(voltage);                      // print out the value you read

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getFloat(&gbdo, "/Conditions/WaterCon"))
    {
      if (fbdo.dataType() == "float")
      {
        c1 = fbdo.floatData();
        Serial.println(c1);
        Serial.println("C1 read");
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.getFloat(&fbdo, "/Conditions/LightCon"))
    {
      if (fbdo.dataType() == "float")
      {
        c2 = fbdo.floatData();
        Serial.println(c2);
        Serial.println("C2 read");
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.getFloat(&fbdo, "/Conditions/TempCon"))
    {
      if (fbdo.dataType() == "float")
      {
        c3 = fbdo.floatData();
        Serial.println(c3);
        Serial.println("C3 read");
      }
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
  }

  if (distance < c1)
  {
    myservo.write(0);
    Serial.println("Water level Conditions satisfied");
  }
  if (voltage < c2)
  {
    digitalWrite(l1, HIGH); // Led on
  }
  if (temp > c3)
  {
    digitalWrite(a0, HIGH); // fan moter
    digitalWrite(a1, LOW);
  }

  if (Firebase.ready() && signupOK)
  {
    if (Firebase.RTDB.setString(&fbdo, "Plants/Temperature", temp))
    {

      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "Plants/Humidity", humidity))
    {

      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "Plants/WaterLevel", distance))
    {

      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "Plants/LightSensitivity", voltage))
    {

      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "Plants/PlantName", "Lettuce"))
    {

      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}