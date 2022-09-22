#include <Arduino.h>
#include <Wifi.h>
#include <Adafruit_NeoPixel.h>

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


#define RANGE_START 39
#define RANGE_END 40

bool isConnected = false;
unsigned long lastConnectionCheckTime = 0;



// Forward function declarations
void processPacket();

#if BUTTON_TEST

#include "SimpleButton.h"

SimpleButton testButton(32);

#endif

void Button()
{
  Serial.println("Button");
}

void setup() 
{
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

  #elif BUZZO_BUTTON

    Serial.print("Connecting to wifi... ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    //auto flash = LOW;
    isConnected = false;
    
    // while (WiFi.status() != WL_CONNECTED) 
    // {
    //   digitalWrite(LED_PIN, flash);
    //   flash = flash == LOW ? HIGH : LOW;
    //   Serial.print('.');
    //   delay(200);
    // }

    // WiFi.setAutoReconnect(true);
    
    digitalWrite(LED_PIN, HIGH);

    Serial.println(WiFi.localIP());  

    BuzzoButton::GetInstance()->Initialize();
    BuzzoButton::GetInstance()->SetState(BuzzoButton::ANSWERING);

  #elif BUTTON_TEST

  testButton.SetBeginPressCallback(Button);

  #endif

}

#if BUZZO_CONTROLLER

void loop() 
{
  BuzzoController::GetInstance()->Update(); 
}

#elif BUZZO_BUTTON

void loop() 
{  
  if(WiFi.status() != WL_CONNECTED)
  {
    if(isConnected == true)
    {
      isConnected = false;
      BuzzoButton::GetInstance()->SetState(BuzzoButton::DISCONNECTED);
    }
  }
  else
  {
    if(isConnected == false)
    {
      isConnected = true;
      WiFi.setAutoReconnect(true);

      BuzzoButton::GetInstance()->SetState(BuzzoButton::IDLE);
    }
  }
  
  BuzzoButton::GetInstance()->Update(); 
}

#elif BUTTON_TEST

void loop() 
{
  testButton.Update();
}

#endif