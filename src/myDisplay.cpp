/*
 * file   : myDisplay.cpp
 * date   : 2021-04-11
 * 
*/
#include <Arduino.h>
#include "main.h"
#include "GPIO_PINS.h"

#include <U8x8lib.h> // OLED display Library
// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(OLED_TH_CLK, OLED_TH_SDA, OLED_TH_RST);
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/15, /* data=*/4, /* reset=*/16);

// ---------------------------------------------------------------------------------------------------------
void oled2xPrint(int x, int y, const char *message)
{
    u8x8.draw2x2String(x, y, message);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void oledPrint(int x, int y, const char *message)
{
    u8x8.drawString(x, y, message);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void oledClear()
{
    u8x8.clear();
} // end of function

// ---------------------------------------------------------------------------------------------------------
void oledClearRow(int row)
{
    /*
  for (int i = 0; i < 18; i++)
  {
    u8x8.drawString(i, row, " ");
  }
  */
    u8x8.clearLine(row);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void setup_oled()
{
    // Initialize OLED
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r); // Set font
                                               // u8x8.setPowerSave(0); // 1= enable or disables is_enable = 0
} // end of function
