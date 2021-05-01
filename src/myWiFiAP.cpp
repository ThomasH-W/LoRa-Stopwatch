/*
 * file   : myWiFi.cpp
 * date   : 2021-05-01
 * 
 * Buttons
 * - Start/Stop/Reset
 * - Mode/Lap
 * Timer
 *   Main
 *   Lap 1...10
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
AsyncWebServer webServer(80);
AsyncWebSocket ws("/");
AsyncWebSocketClient *globalClient = NULL;

// Replace with your network credentials
const char *ssid = "ESP32-StopWatch";
const char *password = "12345678";

// Assign output variables to GPIO pins
const int output26 = 36;
const int output27 = 37;

// --------------------------------------------------------------------------
// websocket - send modes for system and stopwtach - see main.h
void wsSendMode(int sys_mode, int sw_mode)
{
    Serial.printf("wifi::wsSendMode> system mode %d, stopwatch mode %d\n", sys_mode, sw_mode);
    ws.printfAll_P("sys_mode=%d", sys_mode);
    ws.printfAll_P("sw_mode=%d", sw_mode);
}

// --------------------------------------------------------------------------
// websocket - send timer: sw_timer = current /main timer
void wsSendTimer(unsigned int sw_timer, unsigned int timerUsed, unsigned int timeLaps[])
{
    // Serial.printf("wifi::wsSendTimer> timer %u, timer 1 %u\n", sw_timer, timeLaps[0]);

    ws.printfAll_P("sw_timer=%u", sw_timer);
    ws.printfAll_P("sw_timer_used=%u", timerUsed);
    ws.printfAll_P("timer=%u", timeLaps[0]);
    for (int i = 1; i < TIME_LAPS_MAX; i++)
        ws.printfAll_P(",%u", timeLaps[i]);
}

// --------------------------------------------------------------------------
// send status to all connected clients = broadcast
// send_SW_Mode() + send_SW_Timer() are defined in main.h calling wsSendMode() + wsSendTimer()
void wsBroadcast()
{
    send_SW_Mode();
    send_SW_Timer();
} // end of function

// --------------------------------------------------------------------------
// web socket - process incoming messages from client, e.g. smartphone
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    if (type == WS_EVT_CONNECT)
    {

        Serial.println("onWsEvent> Websocket client connection received");
        client->text("Hello from ESP32 Server");
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

            if (strncmp(command, "volume", strlen("volume")) == 0)
            {
                // okay since command is \0 terminated
                uint8_t volume = atoi(command + strlen("volume="));
                Serial.printf("onWsEvent> set volume to %d\n", volume);
                // audio_mode(AUDIO_VOLUME, volume);
            }
            if (strncmp(command, "station_select", strlen("station_select")) == 0)
            {
                // okay since command is \0 terminated
                uint8_t presetNo = atoi(command + strlen("station_select="));
                int stationID = (int)presetNo - 1;
                Serial.printf("onWsEvent> tune to station %d [%d]\n", presetNo, stationID);
                // tation_select(stationID); // tune to new station; index 0...9
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

    webServer.on("/inline", [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "this works as well");
    });

    webServer.serveStatic("/", LITTLEFS, "/").setDefaultFile("index.html");
    webServer.onNotFound(handleNotFound);

    ws.onEvent(onWsEvent);
    webServer.addHandler(&ws);

    webServer.begin();
    Serial.println("HTTP server started");
} // end of function

// ---------------------------------------------------------------------------------------------------------
// start access point using the MAC to set a unique SSID
void setup_WiFiAP(uint32_t curMAC)
{
    char buf[30];

    sprintf(buf, "%s-%x", ssid, curMAC);
    // Connect to Wi-Fi network with SSID and password
    Serial.printf("setup_WiFiAP> SSID: %s\n", buf);
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(buf, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    setup_webserver();
} // end of function

// ---------------------------------------------------------------------------------------------------------
void WiFiAP_loop()
{
}
