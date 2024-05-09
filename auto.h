#ifndef AUTO
#define AUTO

#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <string>
#include <time.h>
#include "wifi-sniffer.h"
#include "Relay.h"

// Time
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;
struct tm timeinfo;

String profile = "";
String prev_profile = profile;

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

    // Change profile based on owner's presence
    if (showpeople())
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
                profile = "Morning";
            }
            else if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 19)
            {
                profile = "Day";
            }
            else if (timeinfo.tm_hour >= 19 && timeinfo.tm_hour < 23)
            {
                profile = "Evening";
            }
            else
            {
                profile = "Night";
            }
        }
    }
    else
    {
        profile = "Off";
    }

    // On change profile
    if (profile != prev_profile)
    {
        prev_profile = profile;
        if (profile == "Morning")
        {
            Serial.println("Profile switched to \"Morning\"");
            bot->sendMessage(CHAT_ID, "Morning ~", "");
            relays[1].turnOn();
            relays[2].turnOff();
        }
        else if (profile == "Day")
        {
            Serial.println("Profile switched to \"Day\"");
            bot->sendMessage(CHAT_ID, "Time to start your day! 🔥", "");
            relays[1].turnOff();
            relays[2].turnOn();
        }
        else if (profile == "Evening")
        {
            Serial.println("Profile switched to \"Evening\"");
            bot->sendMessage(CHAT_ID, "Good evening ~", "");
            relays[1].turnOn();
            relays[2].turnOff();
        }
        else if (profile == "Night")
        {
            Serial.println("Profile switched to \"Night\"");
            bot->sendMessage(CHAT_ID, "Good night ~", "");
            relays[1].turnOff();
            relays[2].turnOff();
        }
        else if (profile == "Off")
        {
            Serial.println("Profile switched to \"Off\"");
            bot->sendMessage(CHAT_ID, "Cya ~", "");
            relays[1].turnOff();
            relays[2].turnOff();
        }
        else
        {
            Serial.println("ERROR: invalid profile");
            bot->sendMessage(CHAT_ID, "ERROR: invalid profile", "");
        }
    }
}

#endif