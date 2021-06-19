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
#define READS 20

// update setting: lipo_cells, lipo_R1, lipo_R2, lipo_conv_adj
const double
    lipo_cells = 4,                                                       // number of cells
    lipo_full = 4.2,                                                      // max voltage when one cell is full
    lipo_v_max = (lipo_full * lipo_cells),                                // combined max voltage of all cells
    lipo_R1 = 10,                                                         // R to GND
    lipo_R2 = 47,                                                         // R to Vcc
    lipo_R_div = (lipo_R1 / (lipo_R1 + lipo_R2)),                         // Divider - caclulcated from ideal value
    lipo_ADC_V_MAX = lipo_v_max * lipo_R_div,                             // max V in at ADC PIN
    lipo_conv_2 = 3.3 / lipo_ADC_V_MAX,                                   // same like lipo_conv_factor
    lipo_ADC_D_LIMIT = 4096,                                              // max digital value of ADC
    lipo_ADC_D_MAX = lipo_ADC_D_LIMIT / lipo_conv_2,                      // max digital value at lipo_v_max
    lipo_conv_adj = 1.0543,                                                    // manual adjust as ADC is not precise
    lipo_conv_factor = lipo_ADC_D_LIMIT / lipo_ADC_D_MAX * lipo_conv_adj; // conv for library

Pangodream_18650_CL BL(PIN_ADC_BAT, lipo_conv_factor, READS);

void setup_battery()
{
    pinMode(PIN_ADC_EN, OUTPUT);
    digitalWrite(PIN_ADC_EN, HIGH);
}

int battery_info()
{
    Serial.printf("battery_info> lVmax: %.2f, R-Div:%.4f, A-Vmax:%.3f, D-Vmax:%.0f, conv2:%.3f ,conv: %.3f\n",
                  lipo_v_max, lipo_R_div, lipo_ADC_V_MAX, lipo_ADC_D_MAX, lipo_conv_2, lipo_conv_factor);

    Serial.printf("battery_info> avg. value from pin %d: ", PIN_ADC_BAT);
    Serial.print(BL.pinRead());
    Serial.print(" - Volts: ");
    Serial.print(BL.getBatteryVolts() * lipo_cells);
    Serial.print(" ( ");
    Serial.print(BL.getBatteryVolts());
    Serial.print(" ) - Charge level: ");
    int batteryLevel = BL.getBatteryChargeLevel();
    Serial.print(batteryLevel);
    Serial.println("%");

    if (BL.getBatteryVolts() >= MIN_USB_VOL)
    {
        Serial.print("battery_info> Charging");
    }
    return batteryLevel;
} // end of function
