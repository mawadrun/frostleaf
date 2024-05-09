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
int turnOffDelay = 15000;
unsigned long lastPeoplePresence;

void setupAuto()
{
    setupSniffer();
}

void handleAuto(UniversalTelegramBot *bot, Relay *relays)
{
    // From wifi-sniffer.h
    Serial.println("[Auto] Changed channel:" + String(curChannel));
    if (curChannel > maxCh)
    {
        curChannel = 1;
    }
    Serial.println("[Auto] Changing Wi-Fi channel...");
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    Serial.println("[Auto] Wait for 1s..."); // Prevents flickering
    delay(1000);
    Serial.println("[Auto] Updating beacon list...");
    updatetime();
    purge();
    curChannel++;

    // Change profile based on owner's presence
    Serial.println("[Auto] Checking owner's presence...");
    if (showpeople())
    {
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("[Auto] Failed to obtain time");
        }
        else
        {
            Serial.print("[Auto] Current time: ");
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
        lastPeoplePresence = millis();
    }
    // Wait for turnOffDelay before turning off
    else if (millis() > lastPeoplePresence + turnOffDelay)
    {
        profile = "Off";
    }

    // On change profile
    if (profile != prev_profile)
    {
        if (prev_profile == "Off")
        {
            bot->sendMessage(CHAT_ID, "Okaeri~", "");
        }
        prev_profile = profile;
        if (profile == "Morning")
        {
            Serial.println("[Auto] Profile switched to \"Morning\"");
            relays[1].turnOn();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Morning ~", "");
        }
        else if (profile == "Day")
        {
            Serial.println("[Auto] Profile switched to \"Day\"");
            relays[1].turnOff();
            relays[2].turnOn();
            bot->sendMessage(CHAT_ID, "Time to start your day! 🔥", "");
        }
        else if (profile == "Evening")
        {
            Serial.println("[Auto] Profile switched to \"Evening\"");
            relays[1].turnOn();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Good evening ~", "");
        }
        else if (profile == "Night")
        {
            Serial.println("[Auto] Profile switched to \"Night\"");
            relays[1].turnOff();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Good night ~", "");
        }
        else if (profile == "Off")
        {
            Serial.println("[Auto] Profile switched to \"Off\"");
            relays[1].turnOff();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Cya ~", "");
        }
        else
        {
            Serial.println("[Auto] ERROR: invalid profile");
            bot->sendMessage(CHAT_ID, "ERROR: invalid profile", "");
        }
    }
}

#endif