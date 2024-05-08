#ifndef AUTO
#define AUTO

#include <Arduino.h>
#include <ESP32Ping.h>
#include <string>
#include <time.h>
#include "wifi-sniffer.h"

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

void handleAuto(const int relay[4])
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
            digitalWrite(relay[1], LOW);
            digitalWrite(relay[2], HIGH);
        }
        else
        {
            if (timeinfo.tm_hour >= 5 && timeinfo.tm_hour < 6)
            {
                digitalWrite(relay[1], LOW);
                digitalWrite(relay[2], HIGH);
            }
            else if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 19)
            {
                digitalWrite(relay[1], HIGH);
                digitalWrite(relay[2], LOW);
            }
            else if (timeinfo.tm_hour >= 19 && timeinfo.tm_hour < 23)
            {
                digitalWrite(relay[1], LOW);
                digitalWrite(relay[2], HIGH);
            }
            else
            {
                digitalWrite(relay[1], HIGH);
                digitalWrite(relay[2], HIGH);
            }
        }
    }
    else
    {
        for (int i = 0; i <= 3; i++)
        {
            digitalWrite(relay[i], HIGH);
        }
    }
}

#endif