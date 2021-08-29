#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "config.h"

#define MAX_CLIENTS 16

enum sub_type : uint8_t
{
  Alert, Raw
};

struct M_Client
{
  IPAddress ip;
  uint16_t port;
  unsigned long lastP;
  sub_type sType;
};



int locaport = 9001;

WiFiUDP Udp;

long duration;
int distance;
int timesbelow = 0;
int maxdist = 40;
unsigned long lastalert = 0;
char incomingPacket[8];

M_Client clients[MAX_CLIENTS];
uint8_t n_clients = 0;

void blinkStatus(uint8_t n, uint16_t dur = 100, uint16_t off = 400) {
  for (uint8_t i = 0; i < n; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(dur);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(off);
  }
  
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  WiFi.begin(WLAN_SSID, WLAN_KEY);
  while (WiFi.status() != WL_CONNECTED)
  {
    blinkStatus(1, 100, 900);
  }
  blinkStatus(1, 500);
  Udp.begin(locaport);
}


void loop()
{
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2;

  int packetSize = 0;
  packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Udp.read(incomingPacket, 8);
    switch(incomingPacket[0])
    {
      case 'S':
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(DEV_NAME);
        Udp.endPacket();
        break;
      case 'C':
        if(n_clients < MAX_CLIENTS)
        {
          clients[n_clients].ip = Udp.remoteIP();
          clients[n_clients].port = Udp.remotePort();
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('K');
          Udp.endPacket();
          clients[n_clients].lastP = millis();
          switch (incomingPacket[1])
          {
          case 'R':
            clients[n_clients].sType = Raw;
            break;
          case 'A':
            clients[n_clients].sType = Alert;
            break;
          }
          n_clients++;
        } 
        else
        {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('F');
          Udp.endPacket();
        }
        break;
      case 'D':
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
          if(clients[i].ip == Udp.remoteIP() && clients[i].port == Udp.remotePort())
          {
            for (; i < MAX_CLIENTS - 1; i++)
            {
              clients[i] = clients[i+1];
            }
            n_clients--;
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.write('B');
            Udp.endPacket();
            break;
          }
        }
        break;
      case 'M':
        maxdist = incomingPacket[1];
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write('K');
        Udp.endPacket();
        break;
    }
  }

  bool alert = false;
  if(distance < maxdist)
  {
    timesbelow++;
    if(timesbelow > 3 && millis() - 1000 > lastalert)
    {
      alert = true;
      lastalert = millis();
    }
  }
  else
  {
    timesbelow = 0;
  }
  unsigned long threshold = millis() - 2000;
  unsigned long rawThreshold = millis() - 100;
  for (int i = 0; i < n_clients; i++)
  {
    switch (clients[i].sType)
    {
      case Alert:
        if (alert)
        {
          for (int j = 0; j < 3; j++)
          {
            Udp.beginPacket(clients[i].ip, clients[i].port);
            Udp.write(1);
            Udp.endPacket();
          }
          clients[i].lastP = millis();
        }
        else if (clients[i].lastP < threshold)
        {
          Udp.beginPacket(clients[i].ip, clients[i].port);
          Udp.write(0);
          Udp.endPacket();
          clients[i].lastP = millis();
        }
        break;
      
      case Raw:
        if (clients[i].lastP < rawThreshold)
        {
          Udp.beginPacket(clients[i].ip, clients[i].port);
          Udp.write('R');
          Udp.write(distance);
          Udp.endPacket();
          clients[i].lastP = millis();
        }
        
      default:
        break;
    }
  }
  delay(10);
}