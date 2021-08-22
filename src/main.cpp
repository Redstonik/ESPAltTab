#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "config.h"



int locaport = 9001;

WiFiUDP Udp;

long duration;
int distance;
int timesbelow = 0;
int maxdist = 40;
unsigned long lastsent = 0;
unsigned long lastalert = 0;


void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  WiFi.begin(WLAN_SSID, WLAN_KEY);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Udp.begin(locaport);
}
void loop() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2;
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