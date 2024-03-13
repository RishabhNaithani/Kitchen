#include <SoftwareSerial.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Servo.h>

//Constants

#define phrelay 0
#define tdsrelay 1
#define temprelay 2
#define dhtrelay 3
#define servorelay 4
#define buzzerrelay 5 
#define servopin 6
#define trigPin 7 //8
#define phpin A0
#define TdsSensorPin A1
#define DHTPIN 8
#define ds18pin A2
#define echoPin 9
#define irsensor 12

#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial Serial2(10,11);
StaticJsonDocument <256> doc;
int tds=1,PH=2,Temperature=4,VesselTemperature=3,Humidity=5;

int irSensorState = 0;
int previousIrSensorState = 0;
#define ONE_WIRE_BUS A0 

OneWire oneWire(ONE_WIRE_BUS); 
 
DallasTemperature sensors(&oneWire);

#define TdsSensorPin A1
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature;

Servo servo;

void servomove(){
  digitalWrite(servorelay,LOW);
  int i =0;
  while(i<2){
  servo.write(0);
  delay(3000);
  servo.write(90);
  delay(3000);
  i++;
  }
  servo.write(180);
  digitalWrite(servorelay, HIGH);
  previousIrSensorState = irSensorState;
}

void ultrasonic(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  int duration = pulseIn(echoPin, HIGH);
  
  int distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);
  if(distance>= 10){
    servomove();
  }
}

void setup() {
 Serial.begin(9600);
 Serial2.begin(9600);
 pinMode(phrelay, OUTPUT);
  pinMode(tdsrelay, OUTPUT);
  pinMode(dhtrelay, OUTPUT);
  pinMode(temprelay, OUTPUT);
  pinMode(servorelay, OUTPUT);
  pinMode(buzzerrelay, OUTPUT);
 servo.attach(servopin);
 pinMode(trigPin, OUTPUT); //8
  pinMode(phpin,INPUT);
  pinMode(ds18pin,INPUT);
  pinMode(echoPin,INPUT);
  pinMode(irsensor,INPUT);

digitalWrite(phrelay,HIGH);
digitalWrite(buzzerrelay,HIGH);
digitalWrite(servorelay,HIGH);
digitalWrite(temprelay,HIGH);
digitalWrite(dhtrelay,HIGH);
digitalWrite(tdsrelay,HIGH);

 dht.begin();
pinMode(TdsSensorPin,INPUT);
}

void loop() {
  irSensorState = digitalRead(irsensor);

if (irSensorState != previousIrSensorState) {
    if (irSensorState == HIGH) {
      ultrasonic();
    } else {
      delay(1);
    }
}

vesselTemp();
delay(1000);
tdsread();
delay(1000);
dhtval();
delay(1000);

doc["TDS"] = tds;
doc["Humidity"] = Humidity;
doc["Temperature"] = Temperature;
doc["PH"] = PH;
doc["VesselTemperature"] = VesselTemperature;
serializeJson(doc, Serial2);
delay(1000);
}

void dhtval(){
  digitalWrite(dhtrelay, LOW);
  Humidity = dht.readHumidity();
  Temperature= dht.readTemperature();
  digitalWrite(dhtrelay, HIGH);
}
void vesselTemp(){
digitalWrite(temprelay, LOW);
 Serial.print(" Requesting temperatures..."); 
 sensors.requestTemperatures(); // Send the command to get temperature readings 
 Serial.println("DONE"); 

 Serial.print("Temperature is: "); 
 Serial.print(sensors.getTempCByIndex(0));   
 
   delay(1000); 

VesselTemperature = sensors.getTempCByIndex(0);
if(VesselTemperature>150){
Serial.println("High temperature");
digitalWrite(buzzerrelay,LOW);
}
else{
  digitalWrite(buzzerrelay,HIGH);
}
digitalWrite(temprelay, HIGH);
}
void tdsread(){
  digitalWrite(tdsrelay, LOW);
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; 
     
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
   }
   digitalWrite(tdsrelay, HIGH);
}
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

