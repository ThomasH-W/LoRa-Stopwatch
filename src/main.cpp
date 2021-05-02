/*
 * file   : main.cpp
 * date   : 2021-04-28
 * 
 * https://github.com/ThomasH-W/LoRa-Stopwatch
 * 
 * https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaDuplexCallback/LoRaDuplexCallback.ino
 * https://github.com/alexantoniades/encrypted-lora-gateway/blob/master/Encrypted_LoRa_Gateway.ino
 * 
 * lip_deps = 
 *    sandeepmistry/LoRa
 *     https://github.com/evert-arias/EasyBuzzer
 *
 * ToDo:
 *  ws: send countdown step, vorlauf
 *  roundtrip with 3+ devices
 *  solve timer difference of remote module
 *  show 1 digits  ms
 *  battery monitor
*/

#include <Arduino.h>
#include "main.h"
#include "GPIO_PINS.h"

//Libraries for Lora
#include <SPI.h> // include libraries
#include <LoRa.h>
byte incomingSwMode;  // incoming stopwatch mode
byte incomingSysMode; // incoming system mode
byte outgoingSwMode;  // incoming stopwatch mode
byte outgoingSysMode; // incoming system mode
int incomingRSSI;     // incoming RSSI
float incomingSNR;    // incoming SNR
bool loraMessageReceived = false;

bool oledUpdate = false;

// encrypt/decrypt will result in an additional 60ms roundtrip time
// #define LORA_CRYPT
#include "AESLib.h" // AES Encryption Library
AESLib aesLib;
// Initialise and configure AES Encryption settings
char cleartext[256];
char ciphertext[512];
#define AES_N_BLOCK 16

// AES Encryption key
byte aes_key[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
byte aes_iv[AES_N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// LoRa duplex config
String outgoing; // outgoing message
String plaintext = outgoing;

#include <StopWatch.h>

StopWatch timerStopWatch, timerCountdown, timerPing, MyDeepSleep;

#define COUNTDOWN_STEPS 5
unsigned int countDownStep = COUNTDOWN_STEPS;
unsigned int swTime = 0, swPong = 0, swRoundtrip = 0;

unsigned int timeLaps[TIME_LAPS_MAX]; // store elapsed time
unsigned int timeLapsUsed = 0;

system_modes sys_mode = SYS_STOPWATCH, sys_mode_old = SYS_BOOT, sys_mode_received;
const char sys_mode_name[4][10] = {"single", // 0 SYS_STOPWATCH
                                   "loop",   // 1 SYS_STARTLOOP
                                   "admin",  // 2 SYS_ADMIN
                                   "boot"};  // 3 SYS_BOOT

stopwatch_modes sw_mode = SW_IDLE, sw_mode_old = SW_RESET, sw_mode_received;
const char sw_mode_name[9][10] = {"Zzzz",  // 0 SW_IDLE
                                  "Beep",  // 1 SW_COUNTDOWN
                                  "Run",   // 2 SW_RUNNING
                                  "FALSE", // 3 SW FALSESTART
                                  "Lap",   // 4 SW_LAP
                                  "Stop",  // 5 SW_STOP
                                  "Reset", // 6 SW_RESET
                                  "Ping",  // 7 SW_PING
                                  "Pong"}; // 8 SW_PONG

button1_modes btn1_mode = BTN1_START;
const char btn1_name[4][10] = {"Start",  // 0 BTN1_START
                               "Stop",   // 1 BTN1_STOP
                               "Lap",    // 2 BTN1_LAP
                               "Reset"}; // 3 BTN1_RESET

button2_modes btn2_mode = BTN2_MODE;
const char btn2_name[5][10] = {"Mode",  // 0 BTN2_MODE
                               "Lap",   // 1 BTN2_LAP
                               "Down",  // 2 BTN2_DOWN
                               "Reset", // 3 BTN2_RESET
                               "Back"}; // 4 BTN2_BACK

gui_modes gui_mode = GUI_BOOT;

byte msgCount = 0;        // count of outgoing messages
byte localAddress = 0xBB; // address of this device
byte destination = 0xFF;  // destination to send to 0xFF = broadcast
long lastSendTime = 0;    // last send time
int interval = 2000;      // interval between sends

#include <AceButton.h>
using namespace ace_button;
const int BUTTON1_PIN = PIN_BTN_1;
const int BUTTON2_PIN = PIN_BTN_2;
const int BUTTON3_PIN = PIN_BTN_3; // 37;

AceButton button1(BUTTON1_PIN);
AceButton button2(BUTTON2_PIN);
AceButton button3(BUTTON3_PIN);
void handleEvent(AceButton *, uint8_t, uint8_t);
const int LED_PIN = 2; // for ESP32

bool timerRunning = false;
bool timerCountdownRunning = false;
bool timerRemoteStart = false;
unsigned timerRemoteOffset = 30;

#include "EasyBuzzer.h"
unsigned int frequencyHigh = 797;
unsigned int frequencyLow = 641;
unsigned int frequencyMid = 400;
unsigned int duration = 200;

// ---------------------------------------------------------------------------------------------------------
//  process messages if something has been recived
//  when idle, send alive to all
void loraLoop()
{
  static unsigned long loraLoopTimerOld, loraAliveIntervall = 5000;
  system_modes incSysMode = sys_mode;

  if (loraMessageReceived == true)
  {
    loraMessageReceived = false;

    /*
    Serial.printf("loraLoop> current  sw mode: %d %s, sys mode %d %s\n",
                  sw_mode, sw_mode_name[sw_mode],
                  sys_mode, sys_mode_name[sys_mode]);
    Serial.printf("loraLoop> incoming sw mode: %d %s, sys mode %d %s\n",
                  incomingSwMode, sw_mode_name[incomingSwMode],
                  incomingSysMode, sys_mode_name[incomingSysMode]);
*/

    incSysMode = (system_modes)incomingSysMode;

    if (incSysMode != sys_mode) // if mode is different, switch this module
    {
      Serial.printf("loraLoop> incoming sys mode: %d\n", incSysMode);
      sw_reset();
      sys_mode = incSysMode;
    }
    else // no change in sysmode
    {
      // Serial.printf("loraLoop> incoming sw mode: %d\n", incomingSwMode);
      switch (incomingSwMode)
      {
      case SW_IDLE:
        if (sw_mode != SW_IDLE)
        {
          Serial.println("call sw_reset");
          sw_reset();
        }
        break;
      case SW_RESET:
        Serial.println("call sw_reset");
        sw_reset();
        break;
      case SW_STOP:
        // Serial.println("call sw_stop");
        sw_stop();
        timerRemoteStart = false;
        break;
      case SW_COUNTDOWN:
        // Serial.println("call sw_start");
        sw_start();
        timerRemoteStart = true;
        break;
      case SW_FALSESTART:
        sw_mode = SW_STOP;
        break;
      case SW_LAP:
        // Serial.println("call sw_lap");
        sw_lap();
        break;
      case SW_PING:
        sendMessage(SW_PONG, sys_mode, "pong");
        break;
      case SW_PONG:
        timerPing.stop();
        swPong = timerPing.elapsed();
        swRoundtrip = swPong;
        timerPing.reset();
        Serial.printf("LoRa - pong received: roundtrip took %u ms\n", swPong);
        send_Admin();
        break;
      default:
        // if nothing else matches, do the default
        // default is optional
        break;
      } // switch

    } // incoming sysmode

  } // lora msg received

  // when idle, send ping in order to measure time for roundtrip
  if (sw_mode == SW_IDLE)
  {
    if (millis() - loraLoopTimerOld > loraAliveIntervall)
    {
      Serial.println("LoRa - starting roundtrip, send ping");
      if (timerPing.isRunning() == true)
      {
        timerPing.stop();
        timerPing.reset();
      }
      timerPing.start();
      swPong = 0;
      sendMessage(SW_PING, sys_mode, "ping");
      loraLoopTimerOld = millis(); // timestamp the message
    }                              // if timer expired
  }                                // if SW_IDLE

} // end of function

// ---------------------------------------------------------------------------------------------------------
// save time for last lap
void sw_lap()
{
  unsigned int sw_time_lap;

  sw_time_lap = timerStopWatch.elapsed();
  if (timerRemoteStart == true)
  {
    Serial.printf("sw_lap> timer started remotely. timer %u - %u", sw_time_lap, timerRemoteOffset);
    sw_time_lap -= timerRemoteOffset;
    sw_time_lap -= swRoundtrip;
    Serial.printf(" = %d\n", sw_time_lap);
  }
  beepMid();
  if (TIME_LAPS_MAX > timeLapsUsed) // don't write if max entries stored
  {
    timeLaps[timeLapsUsed] = sw_time_lap;
    timeLapsUsed++;
    Serial.printf("sw_lap> time for lap %u: %u\n", timeLapsUsed, sw_time_lap);
  }
  else
    Serial.printf("sw_lap> lap %u exceeded limit of %u entries\n", timeLapsUsed + 1, TIME_LAPS_MAX);

  send_SW_Timer(); // send timer to wifi clients
} // end of function

// ---------------------------------------------------------------------------------------------------------
// set timerRunning = false,
void sw_stop()
{
  timerStopWatch.stop();
  swTime = timerStopWatch.elapsed();
  Serial.printf("sw_stop> time: %u\n", swTime);
  if (timerRemoteStart == true)
  {
    Serial.printf("sw_stop> timer started remotely. timer %u - %u", swTime, timerRemoteOffset);
    swTime -= timerRemoteOffset;
    swTime -= swRoundtrip;
    Serial.printf(" = %d\n", swTime);
    timerRemoteStart = false;
  }
  timerRunning = false;
  sw_mode_old = sw_mode;
  sw_mode = SW_RESET;
  oledUpdate = true;
  btn1_mode = BTN1_RESET;
  beepMid();
} // end of function

// ---------------------------------------------------------------------------------------------------------
// set currentTime = 0
void sw_reset()
{
  Serial.printf("sw_reset> reset timer\n");
  timerStopWatch.reset(); // timer set to zero
  timerRunning = false;
  countDownStep = COUNTDOWN_STEPS;
  timerCountdown.reset();
  oledClearRow(3);
  timerCountdownRunning = false;
  timerRemoteStart = false;
  sw_mode_old = sw_mode;
  sw_mode = SW_IDLE;
  MyDeepSleep.reset();
  MyDeepSleep.start();
  swTime = 0;
  btn1_mode = BTN1_START;
  btn2_mode = BTN2_MODE;
  timeLapsUsed = 0;
  for (int i = 0; i < TIME_LAPS_MAX; i++) // clear old laps
    timeLaps[i] = 0;
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  next action countdown
void sw_start()
{
  // Serial.printf("sw_start> start countdown\n");
  timerCountdown.start();
  timerCountdownRunning = true;
  timerRunning = false;
  oledUpdate = true;
  sw_mode_old = sw_mode;
  sw_mode = SW_COUNTDOWN;
  btn1_mode = BTN1_STOP;
  countDownStep = COUNTDOWN_STEPS;
} // end of function

// ---------------------------------------------------------------------------------------------------------
// set currentTime = 0
void sw_run()
{
  timerStopWatch.start();
  // Serial.printf("sw_run> start timer\n");
  timerRunning = true;
  oledUpdate = true;
  sw_mode_old = sw_mode;
  sw_mode = SW_RUNNING;
  countDownStep = COUNTDOWN_STEPS;
  btn1_mode = BTN1_STOP;
  beepHigh();
} // end of function

// ---------------------------------------------------------------------------------------------------------
void stopwatchLoop()
{
  char buf[20];

  if (true == timerCountdownRunning)
  {

    // check if countdown expired
    if (timerCountdown.elapsed() > 1000 * COUNTDOWN_STEPS)
    {
      timerCountdown.stop();
      if (sys_mode == SYS_STOPWATCH) //system in stopwatch mode
      {
        sw_run(); // only in system stopwatch mode
        btn2_mode = BTN2_LAP;
        timerCountdownRunning = false;
        timerCountdown.reset();
        Serial.printf("stopwatchLoop> single timerCountdown elapsed %d, mode=%d %s\n", timerCountdown.elapsed(), sys_mode, sys_mode_name[sys_mode]);
      }
      else if (sys_mode == SYS_STARTLOOP) //system in continous loop mode
      {
        Serial.printf("stopwatchLoop> loop timerCountdown elapsed %d, mode=%d %s\n", timerCountdown.elapsed(), sys_mode, sys_mode_name[sys_mode]);
        timerCountdown.reset();
        delay(2000); // pause for 2sec
        sw_start();
      }
    }

    // every second
    if (timerCountdown.elapsed() > 1000 * (COUNTDOWN_STEPS - countDownStep))
    {
      send_SW_Count();
      Serial.printf("stopwatchLoop> countdown = %d at %d\n", countDownStep, timerCountdown.elapsed());
      beepLow();

      itoa(countDownStep, buf, 10);
      oledClearRow(3);
      oled2xPrint(6, 3, buf); // void oledPrint(int x, int y, const char *message

      countDownStep--;
    }

  } // if SW_COUNTDOWN

} // end of function

// ---------------------------------------------------------------------------------------------------------
// only when button has been pressed, trigger next actions
void nextStopwatchMode(stopwatch_modes cur_mode)
{
  char buf[20];

  sw_mode_old = sw_mode;

  if (sys_mode == SYS_STOPWATCH) //system in stopwatch mode
  {
    switch (cur_mode)
    {
    case SW_IDLE: // current state IDLE, requested start
      sendMessage(SW_COUNTDOWN, sys_mode, "Start");
      delay(swRoundtrip / 2); // grant some time so the other device will be started at the same time
      // Serial.println("call sw_start");
      sw_start();
      break;
    case SW_STOP:
      sendMessage(SW_STOP, sys_mode, "Stop 2");
      Serial.println("call sw_stop");
      sw_stop();
      timeMillis2Char(swTime, buf);
      sendMessage(SW_STOP, sys_mode, buf);
      break;
    case SW_COUNTDOWN:
      sendMessage(SW_RESET, sys_mode, "Countdown");
      sw_reset();
      break;
    case SW_RUNNING:
      sendMessage(SW_STOP, sys_mode, "Run->Stop");
      sw_stop();
      // Serial.println("call sw_stop"); // will casue delay
      sprintf(buf, "%u", swTime);
      // timeMillis2Char(buf);
      sendMessage(SW_STOP, sys_mode, buf);
      break;
    case SW_RESET:
      sendMessage(SW_RESET, sys_mode, "Reset");
      Serial.println("call sw_reset");
      sw_reset();
      break;
    case SW_FALSESTART:
      sw_mode = SW_STOP;
      sw_stop();
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
    }                                 // end switch
  }                                   // end system in stopwatch mode
  else if (sys_mode == SYS_STARTLOOP) //system in continous countdown mode
  {
    switch (cur_mode)
    {
    case SW_IDLE:
      sendMessage(SW_COUNTDOWN, sys_mode, "Start loop");
      delay(swRoundtrip); // grant some time so the other device will be started at the same time
      // Serial.println("call sw_start");
      sw_start();
      break;
    case SW_COUNTDOWN:
      sendMessage(SW_STOP, sys_mode, "Stop loop");
      Serial.println("call sw_stop");
      sw_stop();
      sw_mode = SW_IDLE;
      break;
    } // end switch
  }   // end system in continous countdown mode

  Serial.printf("nextStopwatchMode>: %d %s -> %d %s\n", sw_mode_old, sw_mode_name[sw_mode_old], sw_mode, sw_mode_name[sw_mode]);

} // end of function

// ---------------------------------------------------------------------------------------------------------
//
void nextSysMode(system_modes cur_sys_mode)
{
  if (sw_mode == SW_IDLE)
  {
    sys_mode_old = sys_mode;
    switch (cur_sys_mode)
    {
    case SYS_STOPWATCH:
      sys_mode = SYS_STARTLOOP;
      break;
    case SYS_STARTLOOP:
      sys_mode = SYS_ADMIN;
      break;
    case SYS_ADMIN:
      sys_mode = SYS_STOPWATCH;
      sw_mode = SW_IDLE;
      break;
    case SYS_BOOT:
      sys_mode = SYS_STARTLOOP;
      break;
    }
    sendMessage(SW_IDLE, sys_mode, "sysmode");
    sw_reset();
    Serial.printf("nextSysMode>: %d %s -> %d %s\n", sys_mode_old, sys_mode_name[sys_mode_old], sys_mode, sys_mode_name[sys_mode]);
  } // SW_IDLE
  else if (sw_mode == SW_RUNNING)
  {
    sendMessage(SW_LAP, sys_mode, "lap");
    sw_lap();
  }

} // end of function

// ---------------------------------------------------------------------------------------------------------
// convert unsgined long time into readable format
void timeMillis2Char(unsigned int curTime, char *buf)
{
  if (curTime == 0)
  {
    sprintf(buf, "--:--.--"); // nicer way of showing idle timr
    // Serial.println(buf);
  }
  else
  {
    float durMSf = (curTime % 1000);               // use float so we can round later on
    unsigned int durMS2 = int(durMSf / 10 + 0.5);  // addd 0.5 to correct wrong rounding
    unsigned int durSS = (curTime / 1000) % 60;    //Seconds
    unsigned int durMM = (curTime / (60000)) % 60; //Minutes
    sprintf(buf, "%2u:%02u.%02u", durMM, durSS, durMS2);
    // Serial.printf("%2u:%2u:%2u / %f\n", durMM, durSS, durMS2, durMSf);
  }
}

// ---------------------------------------------------------------------------------------------------------
// shown firmware, roundtrip, rssi
void oledAdmin(bool initAdmin)
{
  char buf[20];

  if (initAdmin == true)
  {
    oledClear();
    oledPrint(0, 1, "Firmware");
    oledPrint(0, 3, "  ID");
    oledPrint(0, 4, "RSSI");
    oledPrint(0, 5, " SNR");
    oledPrint(0, 7, "ping");
  }

  oledPrint(10, 1, FIRMWARE_VERSION);

  sprintf(buf, "%2x", localAddress);
  oledPrint(7, 3, buf);

  sprintf(buf, "%d", incomingRSSI);
  oledPrint(7, 4, buf);

  sprintf(buf, "%2.1f", incomingSNR);
  oledPrint(7, 5, buf);

  sprintf(buf, "%d", swRoundtrip);
  oledPrint(7, 7, buf);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void oledLoop()
{
  static unsigned long oledTimerOld, oledUpdateIntervall = 94;
  static stopwatch_modes oled_sw_mode_old = SW_RESET;
  static system_modes oled_sys_mode_old = SYS_BOOT;
  static unsigned int oldTimeLapsUsed = 0;
  char buf[20];

  if ((millis() - oledTimerOld > oledUpdateIntervall) || oledUpdate == true)
  {
    if (sw_mode == SW_RUNNING)
    {
      swTime = timerStopWatch.elapsed();
      timeMillis2Char(swTime, buf);
      oled2xPrint(0, 3, buf);

      if (oldTimeLapsUsed != timeLapsUsed) // new lap to be shown
      {
        // Serial.printf("oledLoop> show %u laps\n", timeLapsUsed);
        if (timeLapsUsed == 1)
        {
          timeMillis2Char(timeLaps[0], buf);
          oledPrint(8, 5, buf);
        }
        else if (timeLapsUsed == 2)
        {
          timeMillis2Char(timeLaps[1], buf);
          oledPrint(8, 5, buf);
          timeMillis2Char(timeLaps[0], buf);
          oledPrint(8, 6, buf);
        }
        else if (timeLapsUsed > 2)
        {
          timeMillis2Char(timeLaps[timeLapsUsed - 1], buf);
          oledPrint(8, 5, buf);
          timeMillis2Char(timeLaps[timeLapsUsed - 2], buf);
          oledPrint(8, 6, buf);
          timeMillis2Char(timeLaps[timeLapsUsed - 3], buf);
          oledPrint(8, 7, buf);
        }
        oldTimeLapsUsed = timeLapsUsed;
      } // timeLapsUsed updated
    }
    else if (sw_mode == SW_RESET)
    {
      // Serial.printf("oledStatus> time: %u\n", swTime);
      timeMillis2Char(swTime, buf);
      oled2xPrint(0, 3, buf);
      oldTimeLapsUsed = 0;
    }
    else if (sw_mode == SW_IDLE)
    {
      sprintf(buf, "%d ", swRoundtrip);
      oledPrint(7, 7, buf);
    } // SW_RESET

    if (sw_mode != oled_sw_mode_old)
    {
      send_SW_Mode();
      if (sw_mode == SW_RUNNING)
      {
        btn2_mode = BTN2_LAP;
        oledClearRow(7);
      }
      else
        btn2_mode = BTN2_MODE;

      if (sw_mode == SW_IDLE)
      {
        oledClear();
        swTime = 0;
        timeMillis2Char(swTime, buf);
        oled2xPrint(0, 3, buf); // double size - will use line 3+4
      }                         // SW_RESET

      oledClearRow(1);
      oledPrint(0, 1, btn1_name[btn1_mode]);
      oledPrint(10, 1, "      ");                // void oledPrint(int x, int y, const char *message)
      oledPrint(10, 1, sys_mode_name[sys_mode]); // void oledPrint(int x, int y, const char *message)
      oledPrint(0, 7, btn2_name[btn2_mode]);
      // oledPrint(11, 1, sw_mode_name[sw_mode]); // void oledPrint(int x, int y, const char *message)

      oled_sw_mode_old = sw_mode;
    } // mode changed, udate required

    if (sys_mode == SYS_ADMIN)
    {
      oledAdmin(false);
    }

    if (sys_mode != oled_sys_mode_old)
    {
      send_SW_Mode();
      if (oled_sys_mode_old == SYS_ADMIN)
      {
        oledClear();                 // clear screen
        oled_sw_mode_old = SW_RESET; // force re-building of screen
        oledUpdate = true;           // ask for refresh
      }

      if (sys_mode == SYS_ADMIN)
      {
        oledAdmin(true);
        send_Admin();
      }
      else
      {
        oledClearRow(7);
        oledPrint(0, 7, btn2_name[btn2_mode]);
        oledPrint(10, 1, "      ");                // void oledPrint(int x, int y, const char *message)
        oledPrint(10, 1, sys_mode_name[sys_mode]); // void oledPrint(int x, int y, const char *message)
        oledPrint(0, 7, btn2_name[btn2_mode]);
      }

      oled_sys_mode_old = sys_mode;
    } // mode changed, udate required

    oledUpdate = false;
  } // refresh intervall for oled

} // end of function

// --------------------------------------------------------------------------
// https://github.com/bxparks/AceButton
// https://github.com/bxparks/AceButton/blob/develop/examples/TwoButtonsUsingOneButtonConfig/TwoButtonsUsingOneButtonConfig.ino
void setup_button()
{
  // Buttons use the built-in pull up register.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  // Check if the button was pressed while booting
  if (button1.isPressedRaw())
  {
    Serial.println(F("setup(): button 1 was pressed while booting"));
  }
  if (button2.isPressedRaw())
  {
    Serial.println(F("setup(): button 2 was pressed while booting"));
  }
  if (button3.isPressedRaw())
  {
    Serial.println(F("setup(): button 3 was pressed while booting"));
  }

} // end of function

// --------------------------------------------------------------------------
// The event handler for the button.
void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  int butPressed = button->getPin();

  // Print out a message for all events, for both buttons.
  /*
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(butPressed);
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
*/

  // Control the LED only for the Pressed and Released events of Button 1.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType)
  {
  case AceButton::kEventPressed: // kEventPressed , kEventReleased
    if (butPressed == BUTTON1_PIN)
    {
      Serial.printf("handleEvent> button 1: xxxx for pin %d\n", butPressed);
      nextStopwatchMode(sw_mode);
    }
    else if (butPressed == BUTTON2_PIN)
    {
      Serial.printf("handleEvent> button 2: xxxx for pin %d\n", butPressed);
      nextSysMode(sys_mode);
    }
    else if (butPressed == BUTTON3_PIN)
    {
      Serial.printf("handleEvent> button 3: xxxx for pin %d\n", butPressed);
    }
    break;
    /*
  default:
    Serial.printf("handleEvent> unknown for pin %d & %d\n", butPressed, eventType);
  */
  }
} // end of function

// ---------------------------------------------------------------------------------------------------------
void setup_lora()
{
  // Configuring LoRa
  LoRa.setPins(LORA_CSL_PIN, LORA_RST_PIN, LORA_DI0_PIN);
  Serial.println("Starting LoRa...");
  oledPrint(0, 4, "Starting LoRa");
  if (!LoRa.begin(LORA_BAND))
  {
    Serial.println("Starting LoRa... failed!");
    oledPrint(0, 4, "LoRa: Failed");
    while (true)
      ;
  }
  delay(1000);
  Serial.println("LoRa: ON");
  oledClearRow(4);
  oledPrint(0, 4, "LoRa: ON");

  // Set LoRa to listen
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("Mode: Callback");
  oledPrint(0, 5, "Mode: Callback");

} // end of function

// ---------------------------------------------------------------------------------------------------------
// void sendMessage(const char *outgoing)
void sendMessage(stopwatch_modes outSwMode, system_modes outSysMode, const char *outgoing)
{
  outgoingSwMode = outSwMode;
  outgoingSysMode = outSysMode;

#ifdef LORA_CRYPT
  // Encrypt
  byte enc_iv[AES_N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // iv_block gets written to, provide own fresh copy...
  String encrypted_outgoing = encrypt(outgoing, enc_iv);                       // send an encrypted message
#else
  String encrypted_outgoing = (String)outgoing;
#endif

  LoRa.beginPacket();                      // start packet
  LoRa.write(destination);                 // add destination address
  LoRa.write(localAddress);                // add sender address
  LoRa.write(outgoingSwMode);              // add stopwatch mode
  LoRa.write(outgoingSysMode);             // add system mode
  LoRa.write(msgCount);                    // add message ID
  LoRa.write(encrypted_outgoing.length()); // add payload length
  LoRa.print(encrypted_outgoing);          // add payload
  LoRa.endPacket();                        // finish packet and send it
  LoRa.receive();                          // switch back to receive mode
  // Serial.printf("------------------------- sendMessage> sending id %d: swm %d = %s / %s\n", msgCount, outgoingSwMode, sw_mode_name[outgoingSwMode], outgoing);
  msgCount++; // increment message ID

} // end of function

// ---------------------------------------------------------------------------------------------------------
void onReceive(int packetSize)
{
  if (packetSize == 0)
    return; // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();       // recipient address
  byte sender = LoRa.read();         // sender address
  incomingSwMode = LoRa.read();      // incoming stopwatch mode
  incomingSysMode = LoRa.read();     // incoming system mode
  byte incomingMsgId = LoRa.read();  // incoming msg ID
  byte incomingLength = LoRa.read(); // incoming msg length

  String incoming = ""; // payload of packet
  while (LoRa.available())
  {                                // can't use readString() in callback, so
    incoming += (char)LoRa.read(); // add bytes one by one
  }

  if (incomingLength != incoming.length())
  { // check length for error
    Serial.println("ERR: Lora - message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)
  {
    Serial.println("ERR: Lora - This message is not for me.");
    return; // skip rest of function
  }

  incomingRSSI = LoRa.packetRssi();
  incomingSNR = LoRa.packetSnr();

#ifdef LORA_CRYPT
  // Decrypt
  byte dec_iv[AES_N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // iv_block gets written to, provide own fresh copy...
  String decr_incoming = decrypt(incoming.c_str(), dec_iv);
#else
  String decr_incoming = incoming;
#endif

  // if message is for this device, or broadcast, print details:
  // Serial.printf("Lora msg from 0x%x received: id %d, len %d, sw-mode %d\n", sender, incomingMsgId, incomingLength, incomingSwMode);

  //    Serial.println("Received from: 0x" + String(sender, HEX));
  // Serial.println("Sent to: 0x" + String(recipient, HEX));
  // Serial.println("sw  mode: " + String(incomingSwMode));
  // Serial.println("sys mode: " + String(incomingSysMode));
  // Serial.println("Message id: " + String(incomingMsgId));
  // Serial.println("Message length: " + String(incomingLength));
  //Serial.println("Message: " + decr_incoming);
  //Serial.println("RSSI: " + String(LoRa.packetRssi()));
  // Serial.println("Snr: " + String(LoRa.packetSnr()));
  // Serial.println();
  loraMessageReceived = true;
} // end of function

// ---------------------------------------------------------------------------------------------------------
// Generate IV (once)
void aes_init()
{
  Serial.println("gen_iv()");
  aesLib.gen_iv(aes_iv);
  // workaround for incorrect B64 functionality on first run...
  Serial.println("encrypt()");
  Serial.println(encrypt(strdup(plaintext.c_str()), aes_iv));
} // end of function

// ---------------------------------------------------------------------------------------------------------
String encrypt(const char *msg, byte iv[])
{
  int msgLen = strlen(msg);
  // Serial.print("msglen = ");
  // Serial.println(msgLen);
  char encrypted[2 * msgLen];
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  // Serial.print("encrypted = ");
  // Serial.println(encrypted);
  return String(encrypted);
} // end of function

// ---------------------------------------------------------------------------------------------------------
String decrypt(const char *msg, byte iv[])
{
  // unsigned long ms = micros();
  int msgLen = strlen(msg);
  char original[msgLen]; // half may be enough
  strncpy(original, msg, msgLen);
  char decrypted[msgLen]; // half may be enough
  aesLib.decrypt64(original, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
uint32_t setup_ID()
{
  uint32_t chipId = 0; // 4 byte
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  localAddress = chipId & 0xFF; // get last byte
  Serial.printf("setup ID> %x - %x - using %x\n", chipId, chipId & 0xFF, localAddress);
  return chipId;
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
void beepHigh()
{
  // Serial.println("beepHigh");
  EasyBuzzer.singleBeep(frequencyHigh, duration);
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
void beepMid()
{
  // Serial.println("beepMid");
  EasyBuzzer.singleBeep(frequencyMid, duration);
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
void beepLow()
{
  // Serial.println("beepLow");
  EasyBuzzer.singleBeep(frequencyLow, duration);
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
void setupBuzzer()
{
  Serial.printf("setupBuzzer on PIN %d\n", PIN_BUZ_1);
  EasyBuzzer.setPin(PIN_BUZ_1);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void send_SW_Mode()
{
  wsSendMode(sys_mode, sw_mode);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void send_SW_Timer()
{
  wsSendTimer(swTime, timeLapsUsed, timeLaps);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void send_Admin()
{
  wsSendAdmin(localAddress, incomingRSSI, incomingSNR, swRoundtrip);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void send_SW_Count()
{
  wsSendCountdown(countDownStep);
} // end of function
// ---------------------------------------------------------------------------------------------------------
void setup()
{
  uint32_t myMAC;

  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(50);
  Serial.print("\n+++++++++++++++++++++++++++++++++++++++++++++++ STOPWATCH ");
  Serial.println(FIRMWARE_VERSION);
  //while (!Serial)
  //  ;

  myMAC = setup_ID(); // set sender id by using last byte from chipID
  setupBuzzer();      // assign pin to buzzer
  setup_button();     // assign buttons and register callback

  setup_oled(); // Initialise OLED Display settings
  oledPrint(0, 1, "LoRa STOPWATCH");
  oledPrint(0, 3, "Firmware");
  oledPrint(10, 3, FIRMWARE_VERSION);

  setup_lora();        // Initialise and configure LoRa Radio (SX1272)
  setup_WiFiAP(myMAC); // setup access point - this will add 5ms to roundtrip

  // delay(500);
  deepSleepSetup(&MyDeepSleep);

  timerStopWatch.setResolution(StopWatch::MILLIS); // not needed, is default
  sw_reset();
  oledClear();

} // end of function

// ---------------------------------------------------------------------------------------------------------
void loop()
{

  oledLoop();      // update display
  stopwatchLoop(); // check if countdown is running
  loraLoop();      // check if message was received and process it; when idle send message every 5 sec

  button1.check(); // check if button was pressed
  button2.check(); // check if button was pressed
  button3.check(); // check if button was pressed

  EasyBuzzer.update();                  // play buzzer if reuqested
  deepSleepLoop(sw_mode, &MyDeepSleep); // check if device should go into sleep

  WiFiAP_loop();
  yield(); // not sure if this is required

} // end of function
