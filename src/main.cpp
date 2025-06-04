#if defined(NATIVE_TEST)

#else
#include <Arduino.h>
#include <WiFi.h>

#if defined(BUZZO_BUTTON_ADAFRUIT) || defined(BUZZO_CONTROLLER_ADAFRUIT)
#include "Adafruit_MAX1704X.h"

Adafruit_MAX17048 maxlipo;
#endif

#if defined(BUZZO_CONTROLLER_ADAFRUIT) || defined(BUZZO_CONTROLLER_ALIEXPRESS)

  #include <BuzzoController.h>

  IPAddress local_IP(192,168,1,1);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);  

  #define DISCONNECTED_SLEEP_TIMER  (4 * 60 * 1000)
  #define CONNECTED_SLEEP_TIMER     (40 * 60 * 1000)

  #define LOW_POWER_PIN   11

#else

  #include <BuzzoButton.h>

  #define DISCONNECTED_SLEEP_TIMER  (2 * 60 * 1000)
  #define CONNECTED_SLEEP_TIMER     (15 * 60 * 1000)

#endif


// const char* ssid     = "Trivia Computer Access Point";
// const char* password = "123456789";

#define LED_PIN 13

#define RANGE_START 39
#define RANGE_END 40

#define BATTERY_MAX 4.2f
#define BATTERY_MIN 3.3f

//bool isConnected = false;
//unsigned long lastConnectionCheckTime = 0;

unsigned long wakeTime = 0;

unsigned long batteryReportInterval = 0;
unsigned long lastClientReportTime = 0;

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) 
{
  if(in_max == in_min)
  {
    return out_min;
  }
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned int GetBatteryLevel()
{
  #if defined(BUZZO_BUTTON_ADAFRUIT) || defined(BUZZO_CONTROLLER_ADAFRUIT)
    float cellVoltage = maxlipo.cellVoltage();
    if (isnan(cellVoltage)) 
    {
      Serial.println("Failed to read cell voltage, check battery is connected!");
      delay(2000);
      return 100;
    }
    return (int)maxlipo.cellPercent();
  #else
    int sensorValue = analogReadMilliVolts(BATT_MONITOR);
    sensorValue *= 2;
    float voltage = sensorValue / 1000.0f;

    float batteryPercentage = mapFloat(voltage, BATTERY_MIN, BATTERY_MAX, 0.0, 100.0);    

    batteryPercentage = max(0.0f, batteryPercentage);
    batteryPercentage = min(100.0f, batteryPercentage);

    // if(millis() - batteryReportInterval > 5000)
    // {
    //   batteryReportInterval = millis();
    //   Serial.print("Battery: ");
    //   Serial.print(voltage);
    //   Serial.print("v (");
    //   Serial.print(batteryPercentage);    
    //   Serial.println("%)");
    // }

    return (unsigned int)batteryPercentage;
  #endif
}

void Sleep()
{
    #if BUZZO_BUTTON
      BuzzoButton::GetInstance()->SetState(BuzzoButton::GO_TO_SLEEP);
      return;
    #else
      BuzzoController::GetInstance()->SetAllClientsToSleep();

      delay(50);

      WiFi.disconnect(true);
      digitalWrite(LED_PIN, LOW);
      delay(100);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, LOW);
      esp_deep_sleep_start();
    #endif


    // #if defined(BUZZO_BUTTON_ALIEXPRESS)
    //   esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
    // #elif defined(BUZZO_BUTTON_ADAFRUIT)
    //   esp_sleep_enable_ext0_wakeup(GPIO_NUM_6, LOW);
    // #endif    
}


void setup() 
{
  Serial.begin(115200);
  Serial.println();

  wakeTime = millis();

  #if defined(ESP32)  
    setCpuFrequencyMhz(80);
  #endif

  #if !defined(BUZZO_BUTTON_ALIEXPRESS)
    while (!maxlipo.begin()) {
      Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
      delay(2000);
    }
    Serial.print(F("Found MAX17048"));
    Serial.print(F(" with Chip ID: 0x")); 
    Serial.println(maxlipo.getChipID(), HEX);  
  #endif

  #if BUZZO_CONTROLLER

    pinMode(LOW_POWER_PIN, ANALOG);
    analogWrite(LOW_POWER_PIN, 64);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    BuzzoController::GetInstance()->Initialize();

    Serial.println("Buzzo Controller Initialized");
    
  #elif BUZZO_BUTTON

    digitalWrite(LED_PIN, HIGH);

    BuzzoButton::GetInstance()->Initialize();
    BuzzoButton::GetInstance()->SetState(BuzzoButton::DISCONNECTED);

    BuzzoButton::GetInstance()->SetBatteryLevel(GetBatteryLevel());
    BuzzoButton::GetInstance()->ShowBatteryLevelOnButton();

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

  if(BuzzoController::GetInstance()->IsGoingToSleep())
  {
    //Serial.println("Going to sleep");
    analogWrite(LOW_POWER_PIN, 0);
  }
  else if(GetBatteryLevel() < 10)
  {
    //Serial.println("Low battery");
    analogWrite(LOW_POWER_PIN, (millis() % 1000) < 500 ? 256 : 0);
  }
  else if(BuzzoController::GetInstance()->GetMinBatteryLevelForClients() < 10)
  {
    //Serial.println("Low client battery");
    analogWrite(LOW_POWER_PIN, (millis() % 2000) < 200 ? 10 : 0);
  }
  else
  {
    //Serial.println("Normal");
    analogWrite(LOW_POWER_PIN, 2);
  }

  if(GetBatteryLevel() < 2)
  {
    Sleep();
  }

  if(millis() - lastClientReportTime > 5000)
  {
    lastClientReportTime = millis();
    Serial.print("Controller Battery: ");
    Serial.println(GetBatteryLevel());
    Serial.print("Clients: ");
    Serial.println(BuzzoController::GetInstance()->GetActiveClientCount());
    if(BuzzoController::GetInstance()->GetActiveClientCount() > 0)
    {
      Serial.print("Min Client Battery: ");
      Serial.println(BuzzoController::GetInstance()->GetMinBatteryLevelForClients());
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
    
    if(idleTime > CONNECTED_SLEEP_TIMER)
    {
      Sleep();
    }  
        
  }
  
  BuzzoButton::GetInstance()->SetBatteryLevel(GetBatteryLevel());
  BuzzoButton::GetInstance()->Update(); 

  if(GetBatteryLevel() < 2.0f)
  {
    Sleep();
  }

}


#endif
#endif