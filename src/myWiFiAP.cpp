/*
 * file   : myWiFi.cpp
 * date   : 2021-05-01
 * 
 * https://github.com/me-no-dev/ESPAsyncWebServer
 * https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/examples/ESP_AsyncFSBrowser/ESP_AsyncFSBrowser.ino
 * 
 * https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/
 * 
*/
#include <Arduino.h>
#include "main.h"
#include "GPIO_PINS.h"

#if !defined(ESP32)
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#include "FS.h"
#include "LITTLEFS.h" //this needs to be first, or it all crashes and burns...
#define FORMAT_LITTLEFS_IF_FAILED true

// Load Wi-Fi libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
IPAddress apIP(192, 168, 4, 1);
AsyncWebServer webServer(80);
AsyncEventSource events("/events");
AsyncWebSocket ws("/");
AsyncWebSocketClient *globalClient = NULL;

// Replace with your network credentials
const char *ssid = "StopWatch";
const char *password = "12345678";

unsigned long dnsPreviousMillis = 0;
unsigned int dnsInterval = 2000;
const byte DNS_PORT = 53;
DNSServer dnsServer;

/*
 * Written by Ahmad Shamshiri
  * with lots of research, this sources was used:
 * https://support.randomsolutions.nl/827069-Best-dBm-Values-for-Wifi 
 * This is approximate percentage calculation of RSSI
 * WiFi Signal Strength Calculation
 * Written Aug 08, 2019 at 21:45 in Ajax, Ontario, Canada
-30 dBm	Max achievable signal strength. The client can only be a few feet from the AP to achieve this.
-67 dBm	Minimum signal strength for applications that require very reliable, timely packet delivery.	VoIP/VoWiFi, streaming video
-70 dBm	Minimum signal strength for reliable packet delivery.	Email, web
-80 dBm	Minimum signal strength for basic connectivity. Packet delivery may be unreliable.	N/A
-90 dBm	Approaching or drowning in the noise floor. Any functionality is highly unlikely.
 
 */

// ----------------------------------------------------------------------------------------
// type     excellent   good    medium  poor    off
// quality  4           3       2       1       0
// perc     100%        80%     50%     30%
// RSSI    -50          -60     -70     -80     -90
int RSSI_dBm2Level(int dBm)
{
    Serial.print("LoRa RSSI = ");
    Serial.print(dBm);
    Serial.print(" db , level = ");

    int quality = 0;
    if (dBm > -50) // excellent
        quality = 4;
    else if (dBm > -60) // good
        quality = 3;
    else if (dBm > -70) // medium
        quality = 2;
    else if (dBm > -80) // poor
        quality = 1;
    else if (dBm > -90) // off
        quality = 0;

    Serial.print(quality);
    Serial.println("/4");
    return quality;
} // end of fuction dBmtoLevel

// ----------------------------------------------------------------------------------------
// type     excellent   good    medium  poor    off
// quality  4           3       2       1       0
// perc     100%        80%     50%     30%
// SNR      7          5      0         -10     -15
// https://lora.readthedocs.io/en/latest/#snr
int SNR_dBm2Level(float SNRdBm)
{
    Serial.printf("LoRa SNR = %2.1f db - level = ", SNRdBm);
    int quality = 0;
    if (SNRdBm > 7) // excellent
        quality = 4;
    else if (SNRdBm > 5) // good
        quality = 3;
    else if (SNRdBm > 0) // medium
        quality = 2;
    else if (SNRdBm > -10) // poor
        quality = 1;
    else if (SNRdBm > -15) // off
        quality = 0;

    Serial.print(quality);
    Serial.println("/4");
    return quality;
} // end of fuction dBmtoLevel

// --------------------------------------------------------------------------
// websocket - send modes for system and stopwtach - see main.h
void wsSendMode(int sys_mode, int sw_mode, int mod_mode)
{
    Serial.printf("wifi::wsSendMode > system mode %d, stopwatch mode %d, type %d\n", sys_mode, sw_mode, mod_mode);
    ws.printfAll_P("sys_mode=%d", sys_mode);
    ws.printfAll_P("sw_mode=%d", sw_mode);
    ws.printfAll_P("mod_mode=%d", mod_mode);
}

// --------------------------------------------------------------------------
// websocket - send timer: sw_timer = current /main timer
void wsSendTimer(unsigned int sw_timer, unsigned int timerUsed, unsigned int timeLaps[])
{
    char buf[100];
    char buf2[20];
    Serial.printf("wifi::wsSendTimer> timer %u, %u laps\n", sw_timer, timerUsed);
    for (int i = 0; i < timerUsed; i++)
    {
        Serial.printf("timer %d: %u\n", i + 1, timeLaps[i]);
    }

    ws.printfAll_P("sw_timer=%u", sw_timer);
    ws.printfAll_P("sw_laps_used=%u", timerUsed);

    sprintf(buf, "timer=%u", timeLaps[0]);
    for (int i = 1; i < TIME_LAPS_MAX; i++)
    {
        sprintf(buf2, ",%u", timeLaps[i]);
        strcat(buf, buf2);
    }
    ws.printfAll_P(buf);
}

// --------------------------------------------------------------------------
// websocket - send countdown
void wsSendCountdown(int count)
{
    Serial.printf("wifi::wsSendMode> countdown %d\n", count);
    ws.printfAll_P("sw_count=%d", count);
}

// --------------------------------------------------------------------------
// websocket - send modes for system and stopwtach - see main.h
void wsSendAdmin(byte localAddress,
                 unsigned int loraConnected, int incomingRSSI, int incomingRSSIlevel, float incomingSNR, unsigned int swRoundtrip,
                 unsigned int lightBarrierActive, unsigned int lightBarrierBeam, unsigned int buzzerActive)
{
    // Serial.printf("wifi::wsSendAdmin> convert ...");
    // int incomingRSSIlevel = RSSI_dBm2Level(incomingRSSI);
    // int incomingSNRlevel = SNR_dBm2Level(incomingSNR);

    Serial.printf("wifi::wsSendAdmin> firmware %s, deviceID %2x, connected %d, RSSI %d, SNR %2.1f, trip %d, gate %d, beam %d, buzzer %d\n",
                  FIRMWARE_VERSION, localAddress,
                  loraConnected, incomingRSSI, incomingSNR, swRoundtrip,
                  lightBarrierActive, lightBarrierBeam,
                  buzzerActive);
    ws.printfAll_P("admin_firmware=%s", FIRMWARE_VERSION);
    ws.printfAll_P("admin_deviceID=%2x", localAddress);
    ws.printfAll_P("admin_connected=%d", loraConnected);
    ws.printfAll_P("admin_RSSI=%d", incomingRSSIlevel);
    ws.printfAll_P("admin_RSSIlevel=%d", incomingRSSIlevel);
    ws.printfAll_P("admin_SNR=%2.1f", incomingSNR);
    ws.printfAll_P("admin_roundtrip=%d", swRoundtrip);
    ws.printfAll_P("admin_lbactive=%d", lightBarrierActive);
    ws.printfAll_P("admin_beam=%d", lightBarrierBeam);
    ws.printfAll_P("admin_buzzer=%d", buzzerActive);
}

// --------------------------------------------------------------------------
// send status to all connected clients = broadcast
// send_SW_Mode() + send_SW_Timer() are defined in main.h calling wsSendMode() + wsSendTimer()
void wsBroadcast()
{
    send_SW_Mode();
    // send_SW_Count();
    send_SW_Timer();
    send_Admin();
} // end of function

// --------------------------------------------------------------------------
// web socket - process incoming messages from client, e.g. smartphone
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    if (type == WS_EVT_CONNECT)
    {

        Serial.println("onWsEvent> Websocket client connection received");
        //client->text("Hello from ESP32 Server");
        wsBroadcast();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("onWsEvent> Client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT)
        {
            data[len] = 0;
            char *command = (char *)data;
            Serial.printf("onWsEvent> command: >%s< len: %d\n", command, strlen(command));

            // example: onWsEvent> command: >sw_mode=1< len: 9
            if (strncmp(command, "admin_buzzer", strlen("admin_buzzer")) == 0)
            {
                uint8_t trgt_buzzer_mode = atoi(command + strlen("admin_buzzer="));
                Serial.printf("onWsEvent> set buzzer %d\n", trgt_buzzer_mode);
                ws_BuzzerMode(trgt_buzzer_mode);
            }

            // example: onWsEvent> command: >sw_mode=1< len: 9
            if (strncmp(command, "mod_mode", strlen("mod_mode")) == 0)
            {
                uint8_t trgt_mod_mode = atoi(command + strlen("mod_mode="));
                Serial.printf("onWsEvent> set mod mode to %d\n", trgt_mod_mode);
                ws_ModMode((module_modes)trgt_mod_mode);
            }

            // example: onWsEvent> command: >sw_mode=1< len: 9
            if (strncmp(command, "sys_mode", strlen("sys_mode")) == 0)
            {
                uint8_t trgt_sys_mode = atoi(command + strlen("sys_mode="));
                Serial.printf("onWsEvent> set sys mode to %d\n", trgt_sys_mode);
                sendMessage(SW_IDLE, (system_modes)trgt_sys_mode, "system");
                ws_SysMode((system_modes)trgt_sys_mode);
            }

            // example: onWsEvent> command: >sw_mode=1< len: 9
            if (strncmp(command, "sw_mode", strlen("sw_mode")) == 0)
            {
                // okay since command is \0 terminated
                uint8_t trgt_mode = atoi(command + strlen("sw_mode="));
                Serial.printf("onWsEvent> set mode to %d\n", trgt_mode);
                if (mySysMode() == SYS_STOPWATCH)
                {
                    if (trgt_mode == SW_COUNTDOWN) // 1
                    {
                        sendMessage(SW_COUNTDOWN, SYS_STOPWATCH, "Start");
                        sw_start();
                    }
                    else if (trgt_mode == SW_STOP) // 5
                    {
                        sendMessage(SW_STOP, SYS_STOPWATCH, "Stop");
                        sw_stop();
                    }
                    else if (trgt_mode == SW_RESET) // 6
                    {
                        sendMessage(SW_RESET, SYS_STOPWATCH, "Reset");
                        sw_reset();
                    }
                    else if (trgt_mode == SW_LAP) // 4
                    {
                        sendMessage(SW_LAP, SYS_STOPWATCH, "Lap");
                        sw_lap();
                    }
                }
                else if (mySysMode() == SYS_STARTLOOP)
                {
                    if (trgt_mode == SW_COUNTDOWN) // 1
                    {
                        sendMessage(SW_COUNTDOWN, SYS_STARTLOOP, "Start");
                        sw_start();
                    }
                    else if (trgt_mode == SW_STOP) // 5
                    {
                        sendMessage(SW_STOP, SYS_STARTLOOP, "Stop");
                        sw_stop();
                    }
                    else if (trgt_mode == SW_RESET) // 6
                    {
                        sendMessage(SW_RESET, SYS_STARTLOOP, "Reset");
                        sw_reset();
                    }
                }
            }
        }
    }
} // end of function

// --------------------------------------------------------------------------
void handleNotFound(AsyncWebServerRequest *request)
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (uint8_t i = 0; i < request->args(); i++)
    {
        message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }

    request->send(404, "text/plain", message);
} // end of function

// --------------------------------------------------------------------------
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        //request->addInterestingHeader("ANY");
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        /*
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
        response->print("<p>This is out captive portal front page.</p>");
        response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
        response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
        response->print("</body></html>");
        request->send(response);
*/
        if (LITTLEFS.exists("/index.html"))
        {
            // Serial.println("index.html exists!");
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            File file = LITTLEFS.open("/index.html");
            while (file.available())
                response->write(file.read());
            request->send(response);
        }
        else
            request->send(404);
    }
};

// ---------------------------------------------------------------------------------------------------------
// register the web server - no call in loop required
void setup_webserver()
{
    Serial.print(F("setup_webServer> Inizializing FS..."));
    if (LITTLEFS.begin())
    {
        Serial.println(F("done."));
    }
    else
    {
        Serial.println(F("fail."));
    }

    webServer.on("/inline", [](AsyncWebServerRequest *request)
                 { request->send(200, "text/plain", "this works as well"); });

    webServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");
    webServer.onNotFound(handleNotFound);

    ws.onEvent(onWsEvent);
    webServer.addHandler(&ws);

    /*
    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("hello!", NULL, millis(), 1000);
    });
    webServer.addHandler(&events);
*/

    webServer.begin();
    Serial.println("HTTP server started");
} // end of function

// ---------------------------------------------------------------------------------------------------------
// start access point using the MAC to set a unique SSID
void setup_WiFiAP(uint32_t curMAC)
{
    char buf[30];

    // unique SSID for each module
    module_modes module_type = myModMode();
    if (module_type == MOD_BASIC) // show module type
        sprintf(buf, "%s-%x", ssid, curMAC);
    else if (module_type == MOD_START) // show module type
        sprintf(buf, "%s-Start-%x", ssid, curMAC);
    else if (module_type == MOD_FINISH) // show module type
        sprintf(buf, "%s-Finish-%x", ssid, curMAC);
    else if (module_type == MOD_LAP) // show module type
        sprintf(buf, "%s-Lap-%x", ssid, curMAC);

    // same SSID for all LoRa modules - so you can use the one which is close to you
    sprintf(buf, "%s", ssid);

    // Connect to Wi-Fi network with SSID and password
    Serial.printf("setup_WiFiAP> SSID: %s\n", buf);
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(buf, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    // dnsServer.start(DNS_PORT, "*", apIP);
    // webServer.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP

    setup_webserver();
} // end of function

// ---------------------------------------------------------------------------------------------------------
// [E][vfs_api.cpp:64] open(): /littlefs/hotspot-detect.html does not exist
void WiFiAP_loop()
{
    static int clientsOld = 0;

    //dnsServer.processNextRequest();
    ws.cleanupClients();

    unsigned long currentMillis = millis();
    int clients = WiFi.softAPgetStationNum();

    if (currentMillis - dnsPreviousMillis >= dnsInterval)
    {
        dnsPreviousMillis = currentMillis;
        if (clients >= 1)
        {
            if (clientsOld != clients)
            {
                Serial.printf("WiFiAP_loop> Stations connected = %d\n", clients);
                clientsOld = clients;
            }
        }
    }
}
