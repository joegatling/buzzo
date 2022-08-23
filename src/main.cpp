#include <Arduino.h>
#include <Wifi.h>
#include <WiFiudp.h>

#include "ButtonClient.h"

#define PACKET_MAX_SIZE 1024
#define MAX_CLIENTS 12

const char* ssid     = "Trivia Computer Access Point";
const char* password = "123456789";

IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

unsigned int localPort = 8888; 

char packetBuffer[PACKET_MAX_SIZE + 1]; 

ButtonClient* _clients[MAX_CLIENTS];

WiFiUDP udp;

// Forward function declarations
void processPacket();

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());  

  udp.begin(localPort);

  for(int i = 0; i < MAX_CLIENTS; i++)
  {
    _clients[i] = NULL;
  }
}

void loop() 
{
  processPacket();
  
}

void processPacket()
{
int packetSize = udp.parsePacket();
  if (packetSize) 
  {    
    Serial.printf("Received packet of size %d from %s:%d\n \n", 
    packetSize, 
    udp.remoteIP().toString().c_str(), 
    udp.remotePort());

    // read the packet into packetBufffer
    int n = udp.read(packetBuffer, PACKET_MAX_SIZE);
    packetBuffer[n] = 0;

    Serial.println("Contents:");
    Serial.println(packetBuffer);    
  }
}