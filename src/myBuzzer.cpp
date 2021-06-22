
#include <Arduino.h>
#include "EasyBuzzer.h"
unsigned int frequencyHigh = 797;
unsigned int frequencyLow = 641;
unsigned int frequencyMid = 400;
unsigned int duration = 200;

bool buzzerActive = true;

// ---------------------------------------------------------------------------------------------------------
void beepOff()
{
    // Serial.println("beepOff");
    EasyBuzzer.stopBeep();
} // end of function

// ---------------------------------------------------------------------------------------------------------
void beepHigh()
{
    // Serial.println("beepHigh");
    if (buzzerActive == true)
        EasyBuzzer.singleBeep(frequencyHigh, duration);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void beepMid()
{
    // Serial.println("beepMid");
    if (buzzerActive == true)
        EasyBuzzer.singleBeep(frequencyMid, duration);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void beepLow()
{
    // Serial.println("beepLow");
    if (buzzerActive == true)
        EasyBuzzer.singleBeep(frequencyLow, duration);
} // end of function

void buzzerOff()
{
    buzzerActive = false;
} // end of function

void buzzerOn()
{
    buzzerActive = true;
} // end of function

bool buzzerEnabled()
{
    if (buzzerActive == true)
        return true;
    else
        return false;
} // end of function

// ---------------------------------------------------------------------------------------------------------
void setupBuzzer(unsigned int PIN_BUZ_1)
{
    Serial.printf("setupBuzzer on PIN %d\n", PIN_BUZ_1);
    EasyBuzzer.setPin(PIN_BUZ_1);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void buzzerLoop()
{
    EasyBuzzer.update(); // play buzzer if requested
} // end of function