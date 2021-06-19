/*
  myXmodule.h - Library for mqtt client

  inject two lines in main.cpp
    #include <myXmodule.h>
    myXmodule myXmodule1;

*/
#ifndef MYLED_h
#define MYLED_h

#include "Arduino.h"

// includes for external libraries
// #include <OneWire.h>

// define for items which could close a global config , e.g. PINs
// #define DS18B20_PIN 18 // GPIO-Pin für Daten des DS18B20

#define LOOP_SECONDS_LED 10
#define BLINK_MS_1x 600
#define BLINK_MS_2x 300
#define BLINK_MS_3x 150

class myLED // define class
{
public:
  myLED(unsigned int LED_PIN); // constructor, which is used to create an instance of the class
  bool begin(void);
  void loop(void);
  void on(void);
  void off(void);
  void blink(unsigned long blinkTime);
  void blink1x(void);
  void blink2x(void);
  void blink3x(void);
  unsigned int read(void);
  int version(void);

private:
  unsigned long currentMillisLoop = 0, previousMillis = 0, previousMillisScan = 0;
  unsigned long intervalLoop = LOOP_SECONDS_LED * 1000;
  unsigned int _LED_PIN;
  bool _ledBlink = false;
  bool _ledOn = false;
  unsigned long _ledBlinkTime = BLINK_MS_1x;
};

#endif
