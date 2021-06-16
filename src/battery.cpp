/*
 * file   : battery.cpp
 * date   : 2021-05-09
 * 
 * https://www.youtube.com/watch?v=osAOdmZuvDc
 * https://github.com/pangodream/18650CL
 * https://www.pangodream.es/esp32-getting-battery-charging-level
 * 
*/

#include <Arduino.h>
#include "main.h"
#include "GPIO_PINS.h"

#include <Pangodream_18650_CL.h>

#define MIN_USB_VOL 4.9
#define CONV_FACTOR 1.8
#define READS 20

Pangodream_18650_CL BL(PIN_ADC_BAT, CONV_FACTOR, READS);

void setup_battery()
{
    pinMode(PIN_ADC_EN, OUTPUT);
    digitalWrite(PIN_ADC_EN, HIGH);
}

int battery_info()
{
    Serial.printf("battery_info> Avrg value from pin %d: ", PIN_ADC_BAT);
    Serial.print(BL.pinRead());
    Serial.print(" - Volts: ");
    Serial.print(BL.getBatteryVolts());
    Serial.print(" - Charge level: ");
    int batteryLevel = BL.getBatteryChargeLevel();
    Serial.print(batteryLevel);
    Serial.println("%");

    if (BL.getBatteryVolts() >= MIN_USB_VOL)
    {
        Serial.print("battery_info> Charging");
    }
    return batteryLevel;
} // end of function
