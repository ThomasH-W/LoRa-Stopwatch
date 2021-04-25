/*
  main.h - Library for mqtt client
*/
#ifndef MAIN_h
#define MAIN_h

#include "Arduino.h"
// #include "FS.h"
// #include "LITTLEFS.h" //this needs to be first, or it all crashes and burns...
#define FORMAT_LITTLEFS_IF_FAILED true
#define FlashFS LittleFS

#define AUDIO_DEFAULT_VOLUME 12
#define AUDIO_MAX_VOLUME 20

enum system_modes
{
    SYS_STOPWATCH, // 0
    SYS_BEEPLOOP,  // 1
    SYS_ADMIN,     // 2
    SYS_BOOT       // 3
};

enum stopwatch_modes
{
    SW_IDLE,       // 0
    SW_COUNTDOWN,  // 1
    SW_RUNNING,    // 2
    SW_FALSESTART, // 3
    SW_LAP,        // 4
    SW_STOP,       // 5
    SW_RESET,      // 6
    SW_PING,       // 7
    SW_PONG        // 8
};

enum button1_modes
{
    BTN1_START,
    BTN1_STOP,
    BTN1_LAP,
    BTN1_RESET
};

enum button2_modes
{
    BTN2_MODE,
    BTN2_UP,
    BTN2_DOWN,
    BTN2_RESET,
    BTN2_BACK
};

enum gui_modes
{
    GUI_BOOT, // 0 pure text
    GUI_1,    // 1
    GUI_2,    // 2 Volume
    GUI_3,    // 2 Preset
    GUI_4     // 4 Admin
};

struct module_data_struct
{
    int radioCurrentVolume = 0;
    int radioCurrentStation = 0;
    int radioNextStation = 0;
    bool radioMute = false;
    char radioSongTitle[100];
    char radioArtist[100];
    char radioTitleSeperator = ':';
    bool radioArtistFirst = true;
    bool radioRotaryVolume = true;
    bool preSelect = false;
};

struct wifi_data_struct
{
    char rssiChar[20]; // wifi signal strength
    int rssiLevel = 0; // wifi signal strength
    char ssidChar[20]; // SSID - wlan name
    char IPChar[20];   // SSID - wlan name
    char timeOfDayChar[20];
    char dateChar[20];
};

void oledInit();
void configLora();
void configWifi();
void connectWifi();
void oledPrint(int x, int y, const char *message);
void oled2xPrint(int x, int y, const char *message);
void oled3xPrint(int x, int y, const char *message);
void oledClear();
void oledClearRow(int row);
void onReceive(int packetSize);
void aes_init();
String encrypt(const char *msg, byte iv[]);
String decrypt(char *msg, byte iv[]);
void sendMessage(stopwatch_modes outSwMode, system_modes outSysMode, const char *outgoing);
void nextStopwatchMode(stopwatch_modes cur_mode);
void timeMillis2Char(char *buf);
void nextSysMode(system_modes cur_sys_mode);

void beepHigh();
void beepLow();
void beepMid();

void sw_reset();
void sw_start();
void sw_stop();

#endif
