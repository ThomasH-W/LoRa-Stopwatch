/*
 * file   : lora-stopwatch.cpp
 * date   : 2021-04-11
 * 
 * https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaDuplexCallback/LoRaDuplexCallback.ino
 * https://github.com/alexantoniades/encrypted-lora-gateway/blob/master/Encrypted_LoRa_Gateway.ino
 * 
 * lip_deps = 
 *    sandeepmistry/LoRa
 *     https://github.com/durydevelop/arduino-lib-oled
 *
 * ToDo:
 *  add lap timer
 *            https://github.com/evert-arias/EasyBuzzer
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
bool loraMessageReceived = false;

#include <U8x8lib.h> // OLED display Library
// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(OLED_TH_CLK, OLED_TH_SDA, OLED_TH_RST);
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/15, /* data=*/4, /* reset=*/16);
bool oledUpdate = false;

#include "AESLib.h" // AES Encryption Library
// Initialise and configure AES Encryption settings
AESLib aesLib;
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

StopWatch MySW, MySwCountdown;
//StopWatch SWarray[5];

#define COUNTDOWN_STEPS 5
unsigned int countDownStep = COUNTDOWN_STEPS;
unsigned int swTime = 0;

system_modes sys_mode = SYS_BOOT;
stopwatch_modes sw_mode = SW_IDLE, sw_mode_old = SW_RESET, sw_mode_received;
const char sw_mode_name[7][10] = {"Zzzz",   // 0 SW_IDLE
                                  "Beep",   // 1 SW_COUNTDOWN
                                  "Run",    // 2 SW_RUNNING
                                  "FALSE",  // 3 SW FALSESTART
                                  "Lap",    // 4 SW_LAP
                                  "Stop",   // 5 SW_STOP
                                  "Reset"}; // 6 RESET

button1_modes btn1_mode = BTN1_START;
const char btn1_name[4][10] = {"Start",  // 0 BTN1_START
                               "Stop",   // 1 BTN1_STOP
                               "Lap",    // 2 BTN1_LAP
                               "Reset"}; // 6 BTN1_RESET

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
// AceButton button3(BUTTON3_PIN);
void handleEvent(AceButton *, uint8_t, uint8_t);
const int LED_PIN = 2; // for ESP32

bool timerRunning = false;
bool timerCountdownRunning = false;
bool timerRemoteStart = false;
unsigned timerRemoteOffset = 101;

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
  static unsigned long loraLoopTimerOld, loraAliveintervall = 5000;

  if (loraMessageReceived == true)
  {
    loraMessageReceived = false;

    Serial.printf("loraLoop> incoming mode: %d\n", incomingSwMode);
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
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
    } // switch
  }   // lora msg received

  if (sw_mode == SW_IDLE)
  {
    // do countdown, update oled every 200ms
    if (millis() - loraLoopTimerOld > loraAliveintervall)
    {
      // Serial.printf("loraLoop> send alive\n");
      sendMessage(SW_IDLE, "Alive");
      loraLoopTimerOld = millis(); // timestamp the message
    }                              // if timer expired
  }                                // if SW_IDLE

} // end of function

// ---------------------------------------------------------------------------------------------------------
// set timerRunning = false,
void sw_stop()
{
  MySW.stop();
  swTime = MySW.elapsed();
  Serial.printf("sw_stop> time: %u\n", swTime);
  if (timerRemoteStart == true)
  {
    Serial.printf("sw_stop> timer started remotely. timer %u - %u", swTime, timerRemoteOffset);
    swTime -= timerRemoteOffset;
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
  MySW.stop();
  MySW.reset();
  timerRunning = false;
  countDownStep = COUNTDOWN_STEPS;
  MySwCountdown.stop();
  MySwCountdown.reset();
  timerCountdownRunning = false;
  sw_mode_old = sw_mode;
  sw_mode = SW_IDLE;
  swTime = 0;
  btn1_mode = BTN1_START;
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  next action countdown
void sw_start()
{
  // Serial.printf("sw_start> start countdown\n");
  MySwCountdown.start();
  timerCountdownRunning = true;
  timerRunning = false;
  oledUpdate = true;
  sw_mode_old = sw_mode;
  sw_mode = SW_COUNTDOWN;
  btn1_mode = BTN1_STOP;
} // end of function

// ---------------------------------------------------------------------------------------------------------
// set currentTime = 0
void sw_run()
{
  MySW.start();
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
  static unsigned long swLoopTimerOld, swCountDownIntervall = 1000;
  char buf[20];

  if (true == timerCountdownRunning)
  {
    // check if countdown expired
    if (MySwCountdown.elapsed() > 1000 * COUNTDOWN_STEPS)
    {
      sw_run();
      MySwCountdown.stop();
      timerCountdownRunning = false;
      Serial.printf("stopwatchLoop> MySwCountdown elapsed %d\n", MySwCountdown.elapsed());
      MySwCountdown.reset();
    }

    // do countdown, update oled every 200ms
    if (millis() - swLoopTimerOld > swCountDownIntervall)
    {
      swLoopTimerOld = millis(); // timestamp the message
      Serial.printf("stopwatchLoop> countdown = %d at %d\n", countDownStep, MySwCountdown.elapsed());

      // if (countDownStep > 0)
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
  switch (cur_mode)
  {
  case SW_RESET:
    sendMessage(SW_RESET, "Reset");
    Serial.println("call sw_reset");
    sw_reset();
    break;
  case SW_STOP:
    sendMessage(SW_STOP, "Stop 2");
    Serial.println("call sw_stop");
    sw_stop();
    timeMillis2Char(buf);
    sendMessage(SW_STOP, buf);
    break;
  case SW_IDLE:
    sendMessage(SW_COUNTDOWN, "Start");
    Serial.println("call sw_start");
    sw_start();
    break;
  case SW_COUNTDOWN:
    sw_reset();
    break;
  case SW_RUNNING:
    sendMessage(SW_STOP, "Run->Stop");
    sw_stop();
    // Serial.println("call sw_stop"); // will casue delay
    sprintf(buf, "%u", swTime);
    // timeMillis2Char(buf);
    sendMessage(SW_STOP, buf);
    break;
  case SW_FALSESTART:
    sw_mode = SW_STOP;
    break;
  default:
    // if nothing else matches, do the default
    // default is optional
    break;
  }
  Serial.printf("nextStopwatchMode>: %d %s -> %d %s\n", sw_mode_old, sw_mode_name[sw_mode_old], sw_mode, sw_mode_name[sw_mode]);

} // end of function

// ---------------------------------------------------------------------------------------------------------
// convert unsgined long time into readable format
void timeMillis2Char(char *buf)
{
  if (swTime == 0)
  {
    sprintf(buf, "--:--:--"); // nicer way of showing idle timr
    Serial.println(buf);
  }
  else
  {
    float durMSf = (swTime % 1000);               // use float so we can round later on
    unsigned int durMS2 = int(durMSf / 10 + 0.5); // addd 0.5 to correct wrong rounding
    unsigned int durSS = (swTime / 1000) % 60;    //Seconds
    unsigned int durMM = (swTime / (60000)) % 60; //Minutes
    sprintf(buf, "%2u:%02u:%02u", durMM, durSS, durMS2);
    // Serial.printf("%2u:%2u:%2u / %f\n", durMM, durSS, durMS2, durMSf);
  }
}

// ---------------------------------------------------------------------------------------------------------
void oledStatus()
{
  static unsigned long oledTimerOld, oledUpdateIntervall = 84;
  static stopwatch_modes oled_sw_mode_old = SW_RESET;
  static system_modes oled_sys_mode_old;
  char buf[20];

  if ((millis() - oledTimerOld > oledUpdateIntervall) || oledUpdate == true)
  {

    if (sw_mode == SW_RUNNING)
    {
      swTime = MySW.elapsed();
      timeMillis2Char(buf);
      oled2xPrint(0, 3, buf);
    } // mode changed, udate required

    if (sw_mode == SW_RESET)
    {
      // Serial.printf("oledStatus> time: %u\n", swTime);
      timeMillis2Char(buf);
      oled2xPrint(0, 3, buf);
    } // mode changed, udate required

    if (sw_mode != oled_sw_mode_old)
    {
      oledClearRow(1);
      oledPrint(0, 1, btn1_name[btn1_mode]);
      oledPrint(11, 1, sw_mode_name[sw_mode]); // void oledPrint(int x, int y, const char *message)

      if (sw_mode == SW_IDLE)
      {
        swTime = 0;
        timeMillis2Char(buf);
        oledClearRow(3);
        oled2xPrint(0, 3, buf);
      } // SW_RESET
      oled_sw_mode_old = sw_mode;
    } // mode changed, udate required

    if (sys_mode != oled_sys_mode_old)
    {
      oledClearRow(7);
      oledPrint(0, 7, "Mode");
      sprintf(buf, "%2x", localAddress);
      oledPrint(14, 7, buf);
      // oledPrint(0, 1, sys_mode_name[sys_mode]); // void oledPrint(int x, int y, const char *message)
      oled_sys_mode_old = sys_mode;
    } // mode changed, udate required

    /*
    // show mode: current, send, received
    itoa(sw_mode, buf, 10);
    oled2xPrint(6, 6, buf);
    itoa(outgoingSwMode, buf, 10);
    oled2xPrint(9, 6, buf);
    itoa(incomingSwMode, buf, 10);
    oled2xPrint(12, 6, buf);
*/
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
  /*
  if (button3.isPressedRaw())
  {
    Serial.println(F("setup(): button 3 was pressed while booting"));
  }
  */

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

// ---------------------------------------------------------------------------------------------------------
// void sendMessage(const char *outgoing)
void sendMessage(stopwatch_modes outSwMode, const char *outgoing)
{
  outgoingSwMode = outSwMode;
  outgoingSysMode = sys_mode;

  // Encrypt
  byte enc_iv[AES_N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // iv_block gets written to, provide own fresh copy...
  String encrypted_outgoing = encrypt(outgoing, enc_iv);                       // send an encrypted message
  LoRa.beginPacket();                                                          // start packet
  LoRa.write(destination);                                                     // add destination address
  LoRa.write(localAddress);                                                    // add sender address
  LoRa.write(outgoingSwMode);                                                  // add stopwatch mode
  LoRa.write(outgoingSysMode);                                                 // add system mode
  LoRa.write(msgCount);                                                        // add message ID
  LoRa.write(encrypted_outgoing.length());                                     // add payload length
  LoRa.print(encrypted_outgoing);                                              // add payload
  LoRa.endPacket();                                                            // finish packet and send it
  LoRa.receive();                                                              // switch back to receive mode
  Serial.printf("------------------------- sendMessage> sending id %d: swm %d = %s / %s\n", msgCount, outgoingSwMode, sw_mode_name[outgoingSwMode], outgoing);
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

  // Decrypt
  //  byte dec_iv[AES_N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // iv_block gets written to, provide own fresh copy...
  //String decr_incoming = decrypt(incoming, dec_iv);

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
String decrypt(char *msg, byte iv[])
{
  // unsigned long ms = micros();
  int msgLen = strlen(msg);
  char decrypted[msgLen]; // half may be enough
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
} // end of function

// ---------------------------------------------------------------------------------------------------------
//  The true ESP32 chip ID is essentially its MAC address
void setupID()
{
  uint32_t chipId = 0; // 4 byte
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  localAddress = chipId & 0xFF; // get last byte
  Serial.printf("setup ID> %x - %x - using %x\n", chipId, chipId & 0xFF, localAddress);
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
  Serial.printf("setupBuzzer on PIN %d", PIN_BUZ_1);
  EasyBuzzer.setPin(PIN_BUZ_1);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(50);
  Serial.println("---------------------------");
  //while (!Serial)
  //  ;

  setupID();      // set sender id by using last byte from chipID
  setupBuzzer();  // assign pin to buzzer
  setup_button(); // assign buttons and register callback
  setup_oled();   // Initialise OLED Display settings
  setup_lora();   // Initialise and configure LoRa Radio (SX1272)

  delay(500);
  oledClear();
  MySW.setResolution(StopWatch::MILLIS);
} // end of function

// ---------------------------------------------------------------------------------------------------------
void loop()
{

  oledStatus();    // update display
  stopwatchLoop(); // check if countdown is running
  loraLoop();      // check if message was received and process it; when idle send message every 5 sec
  button1.check(); // check if button was pressed
  button2.check(); // check if button was pressed
  EasyBuzzer.update();
  yield();

} // end of function
