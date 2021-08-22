#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define echoPin 5 //D2
#define trigPin 4 //D1


const char* ssid = "SSID";
const char* password = "KEY";
IPAddress targ(0, 0, 0, 0);
int targport = 4202;
int locaport = 9001;

WiFiUDP Udp;

long duration;
int distance;
int timesbelow = 0;
int maxdist = 40;
unsigned long lastsent = 0;
unsigned long lastalert = 0;


void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //Serial.begin(115200);
  //Serial.println("Ultrasonic Sensor HC-SR04 Test");
  //Serial.println("with Arduino UNO R3");
  //Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println(" connected");
  Udp.begin(locaport);
}
void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println(" cm");
  if(distance < maxdist) {
    timesbelow++;
    if(timesbelow > 3 && millis() - 1000 > lastalert) {
      for(int i = 0; i < 3; i++) {
        Udp.beginPacket(targ, targport);
        Udp.write(1);
        Udp.endPacket();
      }
      lastsent = millis();
      lastalert = millis();
      timesbelow = 0;
    }
  } else {
    timesbelow = 0;
  }
  if(millis() - 2000 > lastsent) {
    Udp.beginPacket(targ, targport);
    Udp.write(0);
    Udp.endPacket();
    lastsent = millis();
  }
  delay(10);
}