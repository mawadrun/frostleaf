#ifndef AUTO
#define AUTO

#include <Arduino.h>
#include <ESP32Ping.h>
#include <UniversalTelegramBot.h>
#include <string>
#include <time.h>
#include "wifi-sniffer.h"
#include "Relay.h"

// LAN scanner
#define PING_LIMIT 5
#define LED_PIN 23

bool prev_showpeople = false;

// Time
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;
struct tm timeinfo;

// Flags
int auto_mode = 1;
int light_mode = 0;
int prev_light_mode = -1;

void setupAuto()
{
    setupSniffer();
}

void handleAuto(UniversalTelegramBot *bot, Relay *relays)
{
    // From wifi-sniffer.h
    Serial.println("Changed channel:" + String(curChannel));
    if (curChannel > maxCh)
    {
        curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(1000);
    updatetime();
    purge();
    curChannel++;
    if (showpeople() != prev_showpeople && showpeople())
    // My own things
    {
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
        }
        else
        {
            Serial.print("Current time: ");
            Serial.print(timeinfo.tm_hour);
            Serial.print(":");
            Serial.print(timeinfo.tm_min);
            Serial.print(":");
            Serial.println(timeinfo.tm_sec);
            if (timeinfo.tm_hour >= 5 && timeinfo.tm_hour < 6)
            {
                Serial.println("Mode 1");
                relays[1].turnOn();
                relays[2].turnOff();
            }
            else if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 19)
            {
                Serial.println("Mode 2");
                relays[1].turnOff();
                relays[2].turnOn();
            }
            else if (timeinfo.tm_hour >= 19 && timeinfo.tm_hour < 23)
            {
                Serial.println("Mode 3");
                relays[1].turnOn();
                relays[2].turnOff();
            }
            else
            {
                Serial.println("Mode 4");
                relays[1].turnOff();
                relays[2].turnOff();
            }
        }
    }
    else
    {
        Serial.println("Mode 0");
        relays[1].turnOff();
        relays[2].turnOff();
    }
}

#endif