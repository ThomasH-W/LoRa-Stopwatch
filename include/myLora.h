/*
  myLora.h - Library for mqtt client

  inject two lines in main.cpp
    #include <myLora.h>
    myLora myLora1;

*/
#ifndef MYLORA_h
#define MYLORA_h

#include "Arduino.h"

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the LoRa transceiver module
#define PIN_LORA_SCK 5   // GPIO5  -- SX1278's SCK
#define PIN_LORA_MISO 19 // GPIO19 -- SX1278's MISnO
#define PIN_LORA_MOSI 27 // GPIO27 -- SX1278's MOSI
// LoRa.setPins(SS, RST, DIO0);
#define PIN_LORA_CS 18  // GPIO18 -- SX1278's CS
#define PIN_LORA_RST 14 // GPIO14 -- SX1278's RESET
#define PIN_LORA_IRQ 26 // GPIO26 -- SX1278's IRQ(Interrupt Request)

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

#include <main.h>

#define LORA_MESSAGE_LENGTH 100

#define LOOP_SECONDS_XXX 10
#define SCAN_SECONDS_XXX 30

struct lora_data_struct
{
  unsigned int count;
  char message[LORA_MESSAGE_LENGTH];
  unsigned int size;
};

class myLora // define class
{
public:
  myLora(void); // constructor, which is used to create an instance of the class
  bool begin(void);
  void loop(void);
  int value(void);
  void status(char *value);
  int version(void);
  void getLoRaData(sensor_data_struct *sData);
  void classInterruptHandler(void);
  void onReceiveCallback(int packetSize);     // must be public
  void onReceiveCallback2(int packetSize);     // must be public
  char LoRaDataChar[LORA_MESSAGE_LENGTH + 1]; // must be public
  lora_data_struct lora_data;

private:
  void (*localPointerToCallback)(const int);
  unsigned long currentMillisLoop = 0,
                previousMillis = 0, previousMillisScan = 0;
  unsigned long intervalLoop = LOOP_SECONDS_XXX * 1000;
  unsigned long intervalScanDevice = SCAN_SECONDS_XXX * 1000;
  float temp_ds; //Temperatur des DS18B20
  char statusChar[100];
  bool sensorFound;
  bool loraMsgReceived;
};

#endif
