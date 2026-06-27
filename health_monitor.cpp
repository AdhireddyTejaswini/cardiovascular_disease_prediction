#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "MAX30100_PulseOximeter.h"

#define BUZZER D5
#define ECG_PIN A0
#define BP_PIN A0

LiquidCrystal_I2C lcd(0x27,16,2);
PulseOximeter pox;

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";

WiFiClient client;

unsigned long channelID = YOUR_CHANNEL_ID;
const char *apiKey = "YOUR_WRITE_API";

float heartRate;
float spo2;
int ecgValue;
float bloodPressure;

void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER,OUTPUT);

  lcd.init();
  lcd.backlight();

  WiFi.begin(ssid,password);

  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected");

  ThingSpeak.begin(client);

  if(!pox.begin())
  {
    Serial.println("MAX30100 FAILED");
  }

  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  pox.update();

  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();

  ecgValue = analogRead(ECG_PIN);

  bloodPressure = map(analogRead(BP_PIN),0,1023,80,180);

  // Hybrid AI Risk Model
  int riskScore = 0;

  if(heartRate > 100 || heartRate < 50)
  riskScore++;

  if(spo2 < 92)
  riskScore++;

  if(bloodPressure > 140)
  riskScore++;

  if(ecgValue > 700)
  riskScore++;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HR:");
  lcd.print(heartRate);

  lcd.print(" SpO2:");
  lcd.print(spo2);

  lcd.setCursor(0,1);
  lcd.print("BP:");
  lcd.print(bloodPressure);

  if(riskScore >= 2)
  {
    digitalWrite(BUZZER,HIGH);
    lcd.setCursor(10,1);
    lcd.print("RISK!");
  }
  else
  {
    digitalWrite(BUZZER,LOW);
  }

  ThingSpeak.setField(1,heartRate);
  ThingSpeak.setField(2,spo2);
  ThingSpeak.setField(3,bloodPressure);
  ThingSpeak.setField(4,ecgValue);

  ThingSpeak.writeFields(channelID,apiKey);

  delay(15000);
}