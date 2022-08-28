#include <Arduino.h>
#include <Wifi.h>

#if BUZZO_CONTROLLER

  #include "BuzzoController.h"

  IPAddress local_IP(192,168,1,1);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);  

#else

  #include "BuzzoButton.h"

#endif

const char* ssid     = "Trivia Computer Access Point";
const char* password = "123456789";



#if BUZZO_CONTROLLER
  #define LED_PIN LED_BUILTIN
#else
  #define LED_PIN 13
#endif


// Forward function declarations
void processPacket();

void setup() 
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();

  #if BUZZO_CONTROLLER

    Serial.print("Setting soft-AP configuration... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP... ");
    Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());

    BuzzoController::GetInstance()->Initialize();

  #else

    Serial.print("Connecting to wifi... ");
    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.begin(ssid, password));

    auto flash = LOW;
    
    while (WiFi.status() != WL_CONNECTED) 
    {
      digitalWrite(LED_PIN, flash);
      flash = flash == LOW ? HIGH : LOW;
      Serial.print('.');
      delay(200);
    }

    digitalWrite(LED_PIN, HIGH);

    Serial.println(WiFi.localIP());  

    BuzzoButton::GetInstance()->Initialize();

  #endif
  
}

#if BUZZO_CONTROLLER

void loop() 
{
  BuzzoController::GetInstance()->Update(); 
}

#else

void loop() 
{
  BuzzoButton::GetInstance()->Update(); 
}

#endif