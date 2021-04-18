/*

https://github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x64_i2c/ssd1306_128x64_i2c.ino

*/
#include "Arduino.h"
#include "myDisplay.h"

Adafruit_SSD1306 myOLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// ------------------------------------------------------------------------------------------------------------------------
// Class members
// constructor, which is used to create an instance of the class
myDisplay::myDisplay(void)
{

} // end of function

// ------------------------------------------------------------------------------------------------------------------------
bool myDisplay::begin(void)
{
  Serial.print("Display> begin() ... ");

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!myOLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, false))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  Serial.println(" complete - reset display");
  getScreenSize();
  clear();
  return true;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::clear(void)
{
  myOLED.clearDisplay();
  myOLED.setTextColor(WHITE);
  myOLED.setTextSize(1);
  myOLED.setCursor(0, 0);
  myOLED.display();
  printlines = 1;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::getScreenSize(void)
{
  tft_w = myOLED.width();
  tft_h = myOLED.height();
  Serial.printf("Screen size %u * %u\n", tft_w, tft_h);
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::println(const char *msg)
{
  bool errorFlag = strncmp(msg, "ERR", 3);

  if (++printlines > 8)
  {
    myOLED.clearDisplay();
    myOLED.setCursor(0, 0);
    myOLED.setTextSize(1);
    printlines = 1;
  }
  myOLED.println(msg);
  myOLED.display();
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::print(const char *msg)
{
  bool errorFlag = strncmp(msg, "ERR", 3);
  myOLED.print(msg);
  myOLED.display();
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
/*
  version() returns the version of the library:
*/
int myDisplay::value(void)
{
  return 1; // temp_ds
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
/*
  version() returns the version of the library:
*/
int myDisplay::version(void)
{
  return 1;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::Text(const char *co2Char, const char *tempChar)
{
  // myOLED.setTextColor(TFT_WHITE, TFT_BLACK);
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// 1 Time/WLAN -- CO2 -- Temp/Humi
void myDisplay::Gui1(sensor_data_struct sData)
{
  // 240 * 135
  //myOLED.fillScreen(GREY);

  wfiSignal(200, 30, 18, sData.rssiLevel); // x=100, y=100, max=22

  printlines = 0;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// 2 CO2 -- Temp
void myDisplay::Gui2(sensor_data_struct sData)
{

  printlines = 0;

} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// 3 CO2 Text w/ color
void myDisplay::Gui3(sensor_data_struct sData)
{


} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// 4 CO2 Gauge
void myDisplay::Gui4(int min, int max, int value, sensor_data_struct sData)
{

  printlines = 0;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
// 5 Admin: WiFi, Sensor Version, Sensor ID
void myDisplay::Gui5(sensor_data_struct sData)
{

} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myDisplay::wfiSignal(int my_x, int my_y, int my_max, int level) // 100,100,22
{
  // int my_x = 100;
  // int my_y = 100;
  int my_w = 3;
  int my_step = my_w + 5;
  // int my_i = 4; // 3 * 4 + 10 = 12+10 // my_i = (max -10) / 3
  int my_i = (my_max - 10) / 3;

} // end of function
