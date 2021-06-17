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
#define I2S_LRCK_PIN 25
#define I2S_BCLK_PIN 26 // LoRa LORA_DI0_PIN
#define I2S_DATA_PIN 22

// pin 34...39 input only, no PWM
// ADC ranging from 0 to 4095
// ADC1 pin 32..39
// ADC2 pin 0..26 - ADC2 is used by the Wi-Fi driver. Therefore the application can only use ADC2 when the Wi-Fi driver has not started

#define PIN_LED_GATE 12 // show status of gate
#define PIN_LED_LORA 2  // show status of LoRa
#define PIN_LED_RUN 13  // fast:run, single:countdown, double: ping

// Battery - analog digital converter
#define PIN_ADC_EN 14  // #ADC_EN is the ADC detection enable port
#define PIN_ADC_BAT 34 // #gpio32 to 39 or 255 if not used

// Buttons
#define PIN_BTN_1 0  // Start/Stop, on board button
#define PIN_BTN_2 17 // Mode
#define PIN_BTN_3 35 // Light barrier --- was 12 

// Buzzer -- ESP32: GPIO 34-39 can't be used
#define PIN_BUZ_1 21 // on board button
#define buzzerTone1 500
#define buzzerTone2 550

#endif
