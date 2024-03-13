#include <ArduinoJson.h>
#define BLYNK_TEMPLATE_ID "TMPL3qJ_3E7gI"
#define BLYNK_TEMPLATE_NAME "Kitchen"
#define BLYNK_AUTH_TOKEN "n97445A4Jy60wDc5UOvyQKx1wdbkknAO"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
  char ssid[] = "IoT";
  char pass[] = "123456789";
#define RX2 16
#define TX2 17
int tds,PH,Temperature,VesselTemperature,Humidity;
StaticJsonDocument <256> doc;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
Serial2.begin(9600,SERIAL_8N1,RX2,TX2);
Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();
  // put your main code here, to run repeatedly:
  if(Serial2.available()){
DeserializationError error = deserializeJson(doc, Serial2);

   if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
   }
  tds = doc["TDS"] ;
  Humidity = doc["Humidity"];
  Temperature = doc["Temperature"];
  PH = doc["PH"];
  VesselTemperature = doc["VesselTemperature"];
  Serial.println("Data from the Arduino:");
  Serial.print("TDS:");
  Serial.println(tds);
  Serial.print("Humidity:");
  Serial.println(Humidity);
  Serial.print("Temperature:");
  Serial.println(Temperature);
  Serial.print("PH:");
  Serial.println(PH);
  Serial.print("VesselTemperature:");
  Serial.println(VesselTemperature);
  delay(100);
  Blynk.virtualWrite(V3, VesselTemperature);
  Blynk.virtualWrite(V6, tds);
  Blynk.virtualWrite(V4, PH);
  Blynk.virtualWrite(V1, Temperature);
  Blynk.virtualWrite(V2, Humidity);
  delay(100);
  }
}
