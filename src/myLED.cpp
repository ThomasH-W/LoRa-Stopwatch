/*
https://github.com/WifWaf/MH-Z19/blob/master/examples/BasicUsage/BasicUsage.ino
https://unsinnsbasis.de/co2-sensor-mhz19b/
*/
#include "Arduino.h"
#include "myLED.h"

// Instance of external ibrary  object
// TFT_eSPI myTFT = TFT_eSPI();

// ------------------------------------------------------------------------------------------------------------------------
// Class members
// constructor, which is used to create an instance of the class
myLED::myLED(unsigned int LED_PIN)
{
  // set PIN
  _LED_PIN = LED_PIN;
  pinMode(_LED_PIN, OUTPUT);
  digitalWrite(_LED_PIN, LOW);
  // begin(); // you do not need begin() usually, but if you want too attach sensor during runtime, you should use begin
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
bool myLED::begin(void)
{
  // check is sensor is present
  return 0; // you may indicate success
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::loop(void)
{
  if (_ledBlink == true)
  {

    currentMillisLoop = millis();
    if (currentMillisLoop - previousMillis > _ledBlinkTime)
    {
      // Serial.print("t");
      if (_ledOn == true) // change: on to off
      {
        digitalWrite(_LED_PIN, LOW);
        _ledOn = false;
      }
      else // change: off to on
      {
        digitalWrite(_LED_PIN, HIGH);
        _ledOn = true;
      }

      previousMillis = millis();
    } // end of timer

  } // blink mode
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::on(void)
{
  digitalWrite(_LED_PIN, HIGH);
  _ledOn = true;
  _ledBlink = false;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::off(void)
{
  digitalWrite(_LED_PIN, LOW);
  _ledOn = false;
  _ledBlink = false;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::blink(unsigned long blinkTime)
{
  if (_ledBlink == false)
  {
    digitalWrite(_LED_PIN, HIGH);
    _ledOn = false;
    _ledBlinkTime = blinkTime;
    _ledBlink = true;
  }
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::blink1x(void)
{
  blink(BLINK_MS_1x);
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::blink2x(void)
{
  blink(BLINK_MS_2x);
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
void myLED::blink3x(void)
{
  blink(BLINK_MS_3x);
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
unsigned int myLED::read(void)
{
  unsigned int ledValue = digitalRead(_LED_PIN);
  return ledValue;
} // end of function

// ------------------------------------------------------------------------------------------------------------------------
/*
  version() returns the version of the library:
*/
int myLED::version(void)
{
  return 1;
} // end of function
