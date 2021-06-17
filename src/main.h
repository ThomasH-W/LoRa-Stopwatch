/*
  main.h - Library for mqtt client
*/
#ifndef MAIN_h
#define MAIN_h

#include "Arduino.h"
#include <StopWatch.h>

#define FIRMWARE_VERSION "1.0"

// #include "FS.h"
// #include "LITTLEFS.h" //this needs to be first, or it all crashes and burns...
#define FORMAT_LITTLEFS_IF_FAILED true
#define FlashFS LittleFS

#define TIME_LAPS_MAX 10

/*
  [btn1:start]: btn1 triggered: function of btn1 was start
  [btn1=stop]: btn1 function changing to stop

Order of operation in mode SYS_STOPWATCH
    SW_IDLE -> [btn1:start] -> SW_COUNTDOWN
    SW_STOP -> [btn1:start] -> SW_RESET + [btn1=stop]
    SW_COUNTDOWN -> [btn1:stop] -> SW_RESET + [btn1=start]
    SW_RUNNING -> [btn1:stop] -> SW_RESET + [btn1=reset]
    SW_RESET -> [btn1:reset] -> SW_IDLE + [btn1=start]
    SW_FALSESTART -> [btn1:stop] -> SW_RESET + [btn1=reset]

Order of operation in mode SYS_STARTLOOP
    SW_IDLE -> [btn1:start] -> SW_COUNTDOWN
    SW_COUNTDOWN -> [delay x sec] -> SW_COUNTDOWN -> .... // mode does not change
    SW_COUNTDOWN -> [btn1:stop] -> SW_IDLE
*/

enum module_modes
{
    MOD_BASIC,  // 0 - no trigger function
    MOD_START,  // 1 - light barrier at start, will detect falsestart
    MOD_FINISH, // 2 - light barrier at start, will stop watch
    MOD_LAP     // 3 - light barrier at start, will detect laps
};

enum system_modes
{
    SYS_STOPWATCH, // 0 - Countdown, run, stop, reset, idle
    SYS_STARTLOOP, // 1 - continous loop: countdown, pause
    SYS_ADMIN,     // 2 - configuration, measure lora signal
    SYS_GATE,      // 3 - configuration, show light barrier
    SYS_BOOT       // 4 - initial state when starting module
};

enum stopwatch_modes
{
    SW_IDLE,       // 0 - do nothing
    SW_COUNTDOWN,  // 1 - countdown, 6 steps: 5 short beeps, 1 long beep
    SW_RUNNING,    // 2 - timer is running
    SW_FALSESTART, // 3 - after running light barrier at start triggered
    SW_LAP,        // 4 - save current time but don't stop stopwatch
    SW_STOP,       // 5 - stop stopwatch
    SW_RESET,      // 6 - clear all timers
    SW_PING,       // 7 - send ping
    SW_PONG,       // 8 - send pong after ping received
    SW_GATE_START, // 9 - light barrier at start gate
    SW_GATE_STOP   // 10 - light barrier at finish gate
};

enum beam_modes
{
    BEAM_INIT,   // 0 - initializing system
    BEAM_ACTIVE, // 1 - beam received, waiting for disturbance
    BEAM_LOST    // 2 - object blocking beam or mirror out of line
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
    BTN2_LAP,
    BTN2_DOWN,
    BTN2_RESET,
    BTN2_BACK
};

enum gui_modes
{
    GUI_BOOT, // 0 pure text
    GUI_1,    // 1
    GUI_2,    // 2 Volume
    GUI_3,    // 3 Preset
    GUI_4,    // 4 LightBarrier
    GUI_5     // 5 Admin
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

enum led_modes
{
    LED_INIT,   // 0 - pinmode
    LED_ON,          // 1 - on
    LED_OFF,         // 2 - off
    LED_BREATHE,     // 3 - breathing
    LED_BLINK_FAST,  // 4 - fast
    LED_BLINK_MID,   // 5 - mid
    LED_BLINK_SLOW,  // 6 - slow
    LED_BLINK_ONCE,  // 7 - once
    LED_BLINK_DOUBLE // 8 - twice
};

void oledInit();
void configLora();
void configWifi();
void connectWifi();

void setup_oled();
void oledPrint(int x, int y, const char *message);
void oled2xPrint(int x, int y, const char *message);
void oled3xPrint(int x, int y, const char *message);
void oledClear();
void oledClearRow(int row);
void oledsetFont(const uint8_t *fontName);

void onReceive(int packetSize);
void aes_init();
String encrypt(const char *msg, byte iv[]);
String decrypt(const char *msg, byte iv[]);
void sendMessage(stopwatch_modes outSwMode, system_modes outSysMode, const char *outgoing);

void nextStopwatchMode(stopwatch_modes cur_mode);
void timeMillis2Char(unsigned int curTime, char *buf);
void nextSysMode(system_modes cur_sys_mode);

void deepSleepSetup(StopWatch *MyDeepSleepTimer);
void deepSleepLoop(stopwatch_modes cur_sw_mode, StopWatch *MyDeepSleepTimer);

void beepHigh();
void beepLow();
void beepMid();

void sw_reset();
void sw_start();
void sw_stop();
void sw_lap();

uint32_t setup_ID();
void setup_WiFiAP(uint32_t curMAC);
void WiFiAP_loop();
void send_SW_Mode();
void send_SW_Count();
void send_Admin();
void wsSendMode(int sys_mode, int sw_mode, int mod_mode);
void wsSendTimer(unsigned int sw_timer, unsigned int timerUsed, unsigned int timeLaps[]);
void wsSendAdmin(byte localAddress, int incomingRSSI, float incomingSNR, unsigned int swRoundtrip,
                 unsigned int lightBarrierActive, unsigned int lightBarrierBeam);
void wsSendCountdown(int count);
void send_SW_Timer();
void ws_SysMode(system_modes wsSysMode);
void ws_ModMode(module_modes trgt_mod_mode);
system_modes mySysMode();

void save_preferences();
void setup_preferences();

void beepOff();
void setup_battery();
int battery_info();

#endif
