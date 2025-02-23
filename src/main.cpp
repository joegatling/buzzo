#include <Arduino.h>

#if defined(ESP32)
#include <Wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Adafruit_NeoPixel.h>


#if BUZZO_CONTROLLER

  #include "BuzzoController.h"

  IPAddress local_IP(192,168,1,1);
  IPAddress gateway(192,168,1,1);
  IPAddress subnet(255,255,255,0);  

  #define DISCONNECTED_SLEEP_TIMER  (4 * 60 * 1000)
  #define CONNECTED_SLEEP_TIMER     (20 * 60 * 1000)

  #define LOW_POWER_PIN   27

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

#define BATTERY_MAX 4.2f
#define BATTERY_MIN 3.25f

bool isConnected = false;
unsigned long lastConnectionCheckTime = 0;

unsigned long wakeTime = 0;

unsigned long batteryReportInterval = 0;

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

unsigned int GetBatteryLevel()
{
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
}

void Sleep()
{
    #if defined(ESP8266)
      return;
    #endif
    
    WiFi.disconnect(true);

    #if BUZZO_BUTTON
      BuzzoButton::GetInstance()->DisableLightsAndSound();
    #else
      digitalWrite(LED_PIN, LOW);
    #endif

    delay(100);

    #if defined(ESP32)
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
    #endif  

    #if BUZZO_CONTROLLER
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW);
      //esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
    #endif    

    #if defined(ESP32)
      esp_deep_sleep_start();
    #endif      
}

// void setupLEDC()
// {
//   #if defined(BUZZO_CONTROLLER)
//   // Set the LEDC timer configuration
//     ledc_timer_config_t ledc_timer = {
//         .speed_mode       = LEDC_HIGH_SPEED_MODE,
//         .duty_resolution  = LEDC_TIMER_13_BIT,
//         .timer_num        = LEDC_TIMER_0,
//         .freq_hz          = 5000,  // Frequency in Hertz
//         .clk_cfg          = LEDC_AUTO_CLK
//     };
//     ledc_timer_config(&ledc_timer);

//     // Set the LEDC channel configuration
//     ledc_channel_config_t ledc_channel = {
//         .speed_mode     = LEDC_HIGH_SPEED_MODE,
//         .channel        = LEDC_CHANNEL_0,
//         .timer_sel      = LEDC_TIMER_0,
//         .intr_type      = LEDC_INTR_FADE_END,
//         .gpio_num       = LOW_POWER_PIN,  // GPIO number
//         .duty           = 0,        // Initial duty cycle
//         .hpoint         = 0
//     };
//     ledc_channel_config(&ledc_channel);
//   #endif
// }



void setup() 
{
  Serial.begin(115200);
  Serial.println();

  wakeTime = millis();

  #if defined(ESP32)  
    setCpuFrequencyMhz(80);
    //setupLEDC();
  #endif

  #if BUZZO_CONTROLLER

    Serial.print("Setting soft-AP configuration... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP... ");
    Serial.println(WiFi.softAP(ssid, password, 1, 0, 9) ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());


    BuzzoController::GetInstance()->Initialize();

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(LOW_POWER_PIN, ANALOG);

  #elif BUZZO_BUTTON

    Serial.print("Connecting to wifi... ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    isConnected = false;
    
    digitalWrite(LED_PIN, HIGH);

    Serial.println(WiFi.localIP());  

    BuzzoButton::GetInstance()->Initialize();
    BuzzoButton::GetInstance()->SetState(BuzzoButton::DISCONNECTED);

    BuzzoButton::GetInstance()->SetBatteryLevel(GetBatteryLevel());
    BuzzoButton::GetInstance()->ShowBatteryLevelOnButton();
    delay(1500);

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

  if(GetBatteryLevel() < 10)
  {
    analogWrite(LOW_POWER_PIN, (millis() % 1000) < 500 ? 256 : 0);
  }
  else if(BuzzoController::GetInstance()->GetMinBatteryLevelForClients() < 10)
  {
    analogWrite(LOW_POWER_PIN, (millis() % 2000) < 200 ? 10 : 0);
  }
  else
  {
    analogWrite(LOW_POWER_PIN, 2);
  }

  if(GetBatteryLevel() < 2)
  {
    Sleep();
  }

  //Serial.println(GetBatteryLevel());

  //Serial.println(BuzzoController::GetInstance()->GetActiveClientCount());

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
        wakeTime = millis();
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
        wakeTime = millis();
        WiFi.setAutoReconnect(true);

        BuzzoButton::GetInstance()->SetState(BuzzoButton::IDLE);
      }

      if(idleTime > CONNECTED_SLEEP_TIMER)
      {
        Sleep();
      }    
    }
  }
  
  BuzzoButton::GetInstance()->SetBatteryLevel(GetBatteryLevel());
  BuzzoButton::GetInstance()->Update(); 

  if(GetBatteryLevel() < 2.0f)
  {
    Sleep();
  }

  //Serial.println(GetBatteryLevel());
}


#endif