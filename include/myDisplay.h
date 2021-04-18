/*
  myDisplay.h - Library for mqtt client
*/
#ifndef MYDISPLAY_h
#define MYDISPLAY_h

#include "Arduino.h"

//Libraries for OLED Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED pins
#undef OLED_SDA
#define OLED_SDA 4
#undef OLED_SCL
#define OLED_SCL 15
#define OLED_RST 16
// #define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


#include "main.h"

#define DEG2RAD 0.0174532925

class myDisplay // define class
{
public:
  myDisplay(void); // constructor, which is used to create an instance of the class
  bool begin(void);
  void clear(void);
  void loop(void);
  int value(void);
  void print(const char *msg);
  void println(const char *msg);
  void getScreenSize(void);
  int version(void);
  void Text(const char *co2Char, const char *tempChar);
  void Gui1(sensor_data_struct sensorData);
  void Gui2(sensor_data_struct sensorData);
  void Gui3(sensor_data_struct sensorData);
  void Gui4(int min, int max, int value, sensor_data_struct sData);
  void Gui5(sensor_data_struct sensorData);

private:
  unsigned long timer_output_0 = 0,
                timer_output_5 = 5000;
  int printlines = 0;
  int16_t tft_h = 1;
  int16_t tft_w = 1;
  void wfiSignal(int my_x, int my_y, int my_max, int level);
};

#endif
