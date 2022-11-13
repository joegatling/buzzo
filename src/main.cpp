#include <Arduino.h>
#include <Wifi.h>
#include <Adafruit_NeoPixel.h>

#if BUZZO_CONTROLLER

  #include "BuzzoController.h"

  IPAddress local_IP(192,168,1,1);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);  

  #define DISCONNECTED_SLEEP_TIMER  (4 * 60 * 1000)
  #define CONNECTED_SLEEP_TIMER     (20 * 60 * 1000)

#else

  #include "BuzzoButton.h"
  #define DISCONNECTED_SLEEP_TIMER  (2 * 60 * 1000)
  #define CONNECTED_SLEEP_TIMER     (15 * 60 * 1000)

#endif


const char* ssid     = "Trivia Computer Access Point";
const char* password = "123456789";

#define LED_PIN 13

#define RANGE_START 39
#define RANGE_END 40

bool isConnected = false;
unsigned long lastConnectionCheckTime = 0;

unsigned long wakeTime = 0;

void Sleep()
{
    #if BUZZO_BUTTON
      BuzzoButton::GetInstance()->DisableLights();
    #else
      digitalWrite(LED_PIN, LOW);
    #endif

    delay(100);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);

    #if BUZZO_CONTROLLER
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
    #endif    

    esp_deep_sleep_start();
}

void setup() 
{
  Serial.begin(115200);
  Serial.println();

  wakeTime = millis();

  #if BUZZO_CONTROLLER

    Serial.print("Setting soft-AP configuration... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP... ");
    Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());

    BuzzoController::GetInstance()->Initialize();

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

  #elif BUZZO_BUTTON

    Serial.print("Connecting to wifi... ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    isConnected = false;
    
    digitalWrite(LED_PIN, HIGH);

    Serial.println(WiFi.localIP());  

    BuzzoButton::GetInstance()->Initialize();
    BuzzoButton::GetInstance()->SetState(BuzzoButton::DISCONNECTED);

  #endif

}

#if BUZZO_CONTROLLER

void loop() 
{
  BuzzoController::GetInstance()->Update(); 

  unsigned long timeSinceWake = millis() - wakeTime;
  unsigned long idleTime = min(timeSinceWake, BuzzoController::GetInstance()->TimeSinceLastButtonPress());

  if(BuzzoController::GetInstance()->GetActiveClientCount() > 0)
  {
    if(idleTime > CONNECTED_SLEEP_TIMER)
    {
      Sleep();
    }    
  }
  else
  {
    if(idleTime > DISCONNECTED_SLEEP_TIMER)
    {
      Sleep();
    }
  }  

}

#elif BUZZO_BUTTON

void loop() 
{  
  if(BuzzoButton::GetInstance()->GetState() != BuzzoButton::NONE)
  {
    unsigned long timeSinceWake = millis() - wakeTime;
    unsigned long idleTime = min(timeSinceWake, BuzzoButton::GetInstance()->TimeSinceLastButtonPress());

    if(WiFi.status() != WL_CONNECTED)
    {
      if(isConnected == true)
      {
        Serial.println("Disconnected");
        isConnected = false;
        BuzzoButton::GetInstance()->SetState(BuzzoButton::DISCONNECTED);
      }

      if(idleTime > DISCONNECTED_SLEEP_TIMER)
      {
        Sleep();
      }
    }
    else
    {
      if(isConnected == false)
      {
        Serial.println("Reconnected");
        isConnected = true;
        WiFi.setAutoReconnect(true);

        BuzzoButton::GetInstance()->SetState(BuzzoButton::IDLE);
      }

      if(idleTime > CONNECTED_SLEEP_TIMER)
      {
        Sleep();
      }    
    }
  }
  
  BuzzoButton::GetInstance()->Update(); 
}


#endif