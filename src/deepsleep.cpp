/*
 * file   : deepsleep.cpp
 * date   : 2021-04-11
 * 
 *  lip_deps = 
 *     https://github.com/durydevelop/arduino-lib-oled
*/
#include <Arduino.h>
#include "main.h"
#include "GPIO_PINS.h"
#include <StopWatch.h>

// deep Sleep
int GPIO_reason = 0;
// #define BUTTON_PIN_BITMASK 0x8004
// #define BUTTON_PIN_BITMASK 0x200000000 // GPIO33 2^33 in hex
#define BUTTON_PIN_BITMASK 0x000000000 // GPIO0 2^0 in hex
// unsigned int deepSleepInterval = 60000 * 10; // 60k = 1min
unsigned long deepSleepInterval = 60000 * 10; // 60k = 10min
// https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/

// ---------------------------------------------------------------------------------------------------------
void deepSleepWakeUpReason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.println("Wakeup was not caused by deep sleep.");
        break;
    }
} // end of function

// ---------------------------------------------------------------------------------------------------------
void deepSleepEnable()
{
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("Going to sleep now.");
    esp_deep_sleep_start();
} // end of function

// ---------------------------------------------------------------------------------------------------------
void deepSleepSetup(StopWatch *MyDeepSleepTimer)
{
    deepSleepWakeUpReason(); //Print wakeup Reason

    // ext0 -  Use it when you want to wake-up the chip by one particular pin only.
    //  esp_sleep_enable_ext0_wakeup(GPIO_PIN, LOGIC_LEVEL);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)0, 0); //1 = High, 0 = Low

    /*
  // ext1 – Use it when you have several buttons for the wake-up.
  GPIO_reason = esp_sleep_get_ext1_wakeup_status();

  if (GPIO_reason != 0)
  {
    Serial.print("GPIO that triggered the wake up: GPIO ");
    Serial.println((log(GPIO_reason)) / log(2), 0);
  }
  */

    MyDeepSleepTimer->start();
    Serial.printf("deepSleepSetup> wakeup on GPIO %d\n", 0);

} // end of function

// ---------------------------------------------------------------------------------------------------------
// paramater: elapsed time
// check mode upfront
void deepSleepLoop(stopwatch_modes cur_sw_mode, StopWatch *MyDeepSleepTimer)
{
    // every second
    if (MyDeepSleepTimer->elapsed() > deepSleepInterval)
    {
        Serial.printf("deepSleepLoop> timer elapsed %d\n", MyDeepSleepTimer->elapsed());
            beepLow();
            deepSleepEnable();
    }
} // end of functio
