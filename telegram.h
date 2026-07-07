#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include "secrets.h"
#include "Relay.h"

// Flags
bool stopRiceCookerWhenHome = false;
int riceCookerCookingMin = -1;
unsigned long riceCookerStartTime;

int teaMin = -1;
unsigned long teaStartTime;
String notifyChatID;

int dark_mode = 0;
bool autoWasOnBeforeDark = false;
bool lastLampState[2] = {0, 0};

// Minimum delay between Telegram polls (auto mode's channel dwell makes the
// effective poll rate ~1s anyway)
const unsigned long botRequestDelay = 100;
unsigned long lastTimeBotRan;

// Handle what happens when you receive new messages
void handleNewMessages(UniversalTelegramBot *bot, int numNewMessages, int *auto_mode, Relay *relays)
{
    for (int i = 0; i < numNewMessages; i++)
    {
        // Chat id of the requester
        String chat_id = String(bot->messages[i].chat_id);
        if (chat_id != CHAT_ID)
        {
            bot->sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        // Print the received message
        String text = bot->messages[i].text;
        Serial.println("[Tlgm] Received: \"" + text + "\"");

        String from_name = bot->messages[i].from_name;

        if (text == "/start")
        {
            String welcome = "Welcome, " + from_name + ".\n\n";
            welcome += "/status - relay states\n";
            welcome += "/toggle_auto - presence-based auto mode\n";
            welcome += "/toggle_cozy - warm light\n";
            welcome += "/toggle_daylight - cold light\n";
            welcome += "/toggle_dark - all lights off, restore on second use\n";
            welcome += "/cook - cook rice (120 min)\n";
            welcome += "/warm - warm up rice (30 min)\n";
            welcome += "/tea - warm water for tea (10 min)\n";
            welcome += "/cancel - cancel rice/tea timers\n";
            bot->sendMessage(chat_id, welcome, "");
        }

        if (text == "/toggle_auto")
        {
            if (*auto_mode == 1)
            {
                *auto_mode = 0;
                bot->sendMessage(chat_id, "🟥 Auto mode turned OFF! (. ❛ ᴗ ❛.)", "");
            }
            else
            {
                *auto_mode = 1;
                bot->sendMessage(chat_id, "🟦 Auto mode turned ON! ☆*: .｡. o(≧▽≦)o .｡.:*☆", "");
            }
        }

        if (text == "/status")
        {
            String status = "Status: \n\nDispenser - ";
            status += relays[0].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nWarm light - ";
            status += relays[1].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nCold light - ";
            status += relays[2].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nRice cooker - ";
            status += relays[3].getState() ? "🟦 ON" : "🟥 OFF";
            bot->sendMessage(chat_id, status, "");
        }

        if (text == "/cook")
        {
            stopRiceCookerWhenHome = false;
            riceCookerCookingMin = 120;
            riceCookerStartTime = millis();
            relays[3].turnOn();
            String message = "I will cook the rice for " + String(riceCookerCookingMin) + " minutes!";
            bot->sendMessage(chat_id, message, "");
            notifyChatID = chat_id;
        }

        if (text == "/warm")
        {
            stopRiceCookerWhenHome = false;
            riceCookerCookingMin = 30;
            riceCookerStartTime = millis();
            relays[3].turnOn();
            String message = "I will warm up the rice for " + String(riceCookerCookingMin) + " minutes!";
            bot->sendMessage(chat_id, message, "");
            notifyChatID = chat_id;
        }

        if (text == "/cancel")
        {
            if (riceCookerCookingMin < 0 && teaMin < 0)
            {
                bot->sendMessage(chat_id, "Nothing to cancel!", "");
            }
            else
            {
                if (riceCookerCookingMin >= 0)
                {
                    riceCookerCookingMin = -1;
                    relays[3].turnOff();
                    unsigned long warmUpDurationSec = (millis() - riceCookerStartTime) / 1000;
                    String message = "Cancelled warming up rice after " + String(warmUpDurationSec / 60) + " minutes and " + String(warmUpDurationSec % 60) + " seconds.";
                    bot->sendMessage(chat_id, message, "");
                }
                if (teaMin >= 0)
                {
                    teaMin = -1;
                    relays[0].turnOff();
                    unsigned long warmUpDurationSec = (millis() - teaStartTime) / 1000;
                    String message = "Cancelled making tea after " + String(warmUpDurationSec / 60) + " minutes and " + String(warmUpDurationSec % 60) + " seconds.";
                    bot->sendMessage(chat_id, message, "");
                }
            }
        }

        if (text == "/tea")
        {
            teaMin = 10; // measured
            teaStartTime = millis();
            relays[0].turnOn();
            String message = "I will prepare warm water for tea!";
            bot->sendMessage(chat_id, message, "");
            notifyChatID = chat_id;
        }

        if (text == "/toggle_dark")
        {
            if (dark_mode == 0)
            {
                autoWasOnBeforeDark = (*auto_mode == 1);
                if (autoWasOnBeforeDark)
                {
                    bot->sendMessage(chat_id, "Turning OFF auto mode...", "");
                    *auto_mode = 0;
                }

                // Get current states
                lastLampState[0] = relays[1].getState();
                lastLampState[1] = relays[2].getState();

                relays[1].turnOff();
                relays[2].turnOff();
                dark_mode = 1;
                bot->sendMessage(chat_id, "Time to immerse yourself!", "");
            }
            else
            {
                if (autoWasOnBeforeDark)
                {
                    bot->sendMessage(chat_id, "Turning ON auto mode...", "");
                    *auto_mode = 1;
                }

                // Restore last state
                if (lastLampState[0])
                {
                    relays[1].turnOn();
                }

                if (lastLampState[1])
                {
                    relays[2].turnOn();
                }

                dark_mode = 0;
                bot->sendMessage(chat_id, "Welcome back!", "");
            }
        }

        if (text == "/toggle_cozy")
        {
            String message;
            if (relays[1].getState())
            {
                relays[1].turnOff();
                message = "Turning off warm light!";
            }
            else
            {
                relays[1].turnOn();
                message = "Turning on warm light!!";
            }
            bot->sendMessage(chat_id, message, "");
        }

        if (text == "/toggle_daylight")
        {
            String message;
            if (relays[2].getState())
            {
                relays[2].turnOff();
                message = "Turning off cold light!";
            }
            else
            {
                relays[2].turnOn();
                message = "Turning on cold light!!";
            }
            bot->sendMessage(chat_id, message, "");
        }
    }
}

#endif
