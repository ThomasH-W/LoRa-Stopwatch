/*
  GPIO_PINS.h - Library for mqtt client
*/
#ifndef GPIO_PINS_h
#define GPIO_PINS_h

// SX1272 pins
#define LORA_CSL_PIN 18 // LoRa radio chip select
#define LORA_RST_PIN 14 // LoRa radio reset
#define LORA_DI0_PIN 26 // change for your board; must be a hardware interrupt pin
#define LORA_BAND 866E6

#include <U8x8lib.h> // OLED display Library
// the OLED used
#define OLED_SCK 5     // GPIO5  -- SX1278's SCK
#define OLED_MISO 19   // GPIO19 -- SX1278's MISnO
#define OLED_MOSI 27   // GPIO27 -- SX1278's MOSI
#define OLED_TH_CLK 15 // GPIO5  -- SX1278's clock -- SCL
#define OLED_TH_SDA 4  // GPIO19 -- SX1278's data  -- SDA
#define OLED_TH_RST 16 // GPIO27 -- SX1278's reset

// I2S
#define PIN_I2S_LRCK 25
#define PIN_I2S_BCLK 26
#define PIN_I2S_DATA 22

// Battery - analog digital converter
#define PIN_ADC_BAT 34 // #gpio32 to 39 or 255 if not used
#define PIN_ADC_EN 14  // #ADC_EN is the ADC detection enable port

// Buttons
#define PIN_BTN_1 0 // on board button
#define PIN_BTN_2 36
#define PIN_BTN_3 37

// Buzzer -- ESP32: GPIO 34-39 can't be used
#define PIN_BUZ_1 21 // on board button
#define buzzerTone1 500
#define buzzerTone2 550

#endif
