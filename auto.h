#ifndef AUTO_H
#define AUTO_H

// Auto mode: scans Wi-Fi channels for known devices and switches lighting
// profiles based on presence and time of day.

#include <Arduino.h>
#include <UniversalTelegramBot.h>
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

// How long the owner must be absent before switching to "Off"
const unsigned long turnOffDelay = 15000;
unsigned long lastPeoplePresence = 0;

// Dwell on each channel for a bit before hopping; prevents flickering
const unsigned long channelDwellMs = 1000;
unsigned long lastChannelHop = 0;

bool presencePerChannel[maxCh];

bool isTherePeople()
{
    for (int i = 0; i < maxCh; i++)
    {
        if (presencePerChannel[i])
        {
            return true;
        }
    }
    return false;
}

void setupAuto()
{
    setupSniffer();
}

void handleAuto(UniversalTelegramBot *bot, Relay *relays, bool *stopRiceCookerWhenHome)
{
    // Stay on the current channel until the dwell time is up, without
    // blocking the loop (keeps Telegram commands responsive)
    if (millis() - lastChannelHop < channelDwellMs)
    {
        return;
    }

    // Record what the sniffer saw on this channel, then hop to the next one
    snifferTick();
    presencePerChannel[curChannel - 1] = knownDevicePresent();

    curChannel++;
    if (curChannel > maxCh)
    {
        curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    lastChannelHop = millis();

    // Change profile based on owner's presence
    if (isTherePeople())
    {
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("[Auto] Failed to obtain time");
        }
        else
        {
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
    else if (millis() - lastPeoplePresence > turnOffDelay)
    {
        profile = "Off";
    }

    // On profile change
    if (profile != prev_profile)
    {
        Serial.println("[Auto] Profile switched to \"" + profile + "\"");
        if (prev_profile == "Off")
        {
            bot->sendMessage(CHAT_ID, "Okaeri~", "");
            // Rice cooker
            if (*stopRiceCookerWhenHome)
            {
                *stopRiceCookerWhenHome = false;
                relays[3].turnOff();
                bot->sendMessage(CHAT_ID, "Warm rice is ready!", "");
            }
        }
        prev_profile = profile;
        if (profile == "Morning")
        {
            relays[1].turnOn();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Morning ~", "");
        }
        else if (profile == "Day")
        {
            relays[1].turnOff();
            relays[2].turnOn();
            bot->sendMessage(CHAT_ID, "Have a great day ~", "");
        }
        else if (profile == "Evening")
        {
            relays[1].turnOn();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Good evening ~", "");
        }
        else if (profile == "Night")
        {
            relays[1].turnOff();
            relays[2].turnOff();
            bot->sendMessage(CHAT_ID, "Good night ~", "");
        }
        else if (profile == "Off")
        {
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
