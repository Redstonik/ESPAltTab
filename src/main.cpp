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
  int cMaxDist;
  bool alarmState;
};



int locaport = 9001;

WiFiUDP Udp;

long duration;
char incomingPacket[8];

M_Client clients[MAX_CLIENTS];
uint8_t n_clients = 0;
int distances[3] = {0};
int distances_i = 0;

void blinkStatus(uint8_t n, uint16_t dur = 100, uint16_t off = 400) {
  for (uint8_t i = 0; i < n; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(dur);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(off);
  }
  
}

int findClient(IPAddress ip, uint16_t port)
{
  for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if(clients[i].ip == Udp.remoteIP() && clients[i].port == Udp.remotePort())
      {
        return i;
      }
    }
    return -1;
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
  distances_i++;
  if (distances_i == 3)
  {
    distances_i = 0;
  }
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  duration = pulseIn(PIN_ECHO, HIGH);
  distances[distances_i] = duration * 0.034 / 2;

  int packetSize = 0;
  packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Udp.read(incomingPacket, 8);
    int usrIndex = findClient(Udp.remoteIP(), Udp.remotePort());
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
            clients[n_clients].cMaxDist = DEF_MAXD;
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
        if (usrIndex < 0)
        {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('F');
          Udp.endPacket();
        }
        else
        {
          for (; usrIndex < MAX_CLIENTS - 1; usrIndex++)
          {
            clients[usrIndex] = clients[usrIndex+1];
          }
          n_clients--;
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('B');
          Udp.endPacket();
        }
        break;
      case 'M':
        if (usrIndex < 0)
        {
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('F');
          Udp.endPacket();
        }
        else
        {
          clients[usrIndex].cMaxDist = incomingPacket[1];
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write('K');
          Udp.endPacket();
        }
        break;
    }
  }

  unsigned long threshold = millis() - 2000;
  unsigned long rawThreshold = millis() - 100;
  unsigned long alrThreshold = millis() - 1000;
  for (int i = 0; i < n_clients; i++)
  {
    int alert;
    switch (clients[i].sType)
    {
      case Alert:
        alert = 0;
        for (uint8_t j = 0; j < 3; j++)
        {
          if (distances[j] < clients[i].cMaxDist)
          {
            alert++;
          }
        }
        if (alert >= (clients[i].alarmState ? 1 : 3))
        {
          if (!clients[i].alarmState || clients[i].lastP < alrThreshold)
          {
            for (int j = 0; j < 3; j++)
            {
              Udp.beginPacket(clients[i].ip, clients[i].port);
              Udp.write(1);
              Udp.endPacket();
            }
            clients[i].lastP = millis();
            clients[i].alarmState = true;
          }
        }
        else
        {
          if (clients[i].lastP < threshold)
          {
            Udp.beginPacket(clients[i].ip, clients[i].port);
            Udp.write(0);
            Udp.endPacket();
            clients[i].lastP = millis();
          }
          clients[i].alarmState = false;
        }
        break;
      
      case Raw:
        if (clients[i].lastP < rawThreshold)
        {
          Udp.beginPacket(clients[i].ip, clients[i].port);
          Udp.write('R');
          Udp.write(distances[distances_i]);
          Udp.endPacket();
          clients[i].lastP = millis();
        }
        
      default:
        break;
    }
  }
  delay(10);
}