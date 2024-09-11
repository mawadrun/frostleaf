#ifndef TELEGRAM
#define TELEGRAM

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>          // Initialize Telegram BOT
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialLite.h>
#include "secrets.h"
#include "Relay.h"

#define RELAY_SELECT_MENU "[[\"REL 1 - None 🚫\", \"REL 2 - Warm light 🟨\"],[\"REL 3 - Cold light 🟦\", \"REL 4 - Rice cooker 🌾🔥\"]]"
#define RELAY_OPERATION_MENU "[[\"🟦 ON\", \"🟥 OFF\"]]"

// Flags
bool stopRiceCookerWhenHome = false;
int riceCookerCookingMin = -1;
unsigned long riceCookerStartTime;

int teaMin = -1;
unsigned long teaStartTime;
String notifyChatID;

int dark_mode = 0;
bool lastLampState[2] = {0, 0};

// Checks for new messages every 1 second.
int botRequestDelay = 100;
unsigned long lastTimeBotRan;
int relay_index = 0;

// Handle what happens when you receive new messages
void handleNewMessages(UniversalTelegramBot *bot, int numNewMessages, int *auto_mode, Relay *relays)
{
    // Serial.print("Received: ");
    // Serial.println(String(numNewMessages));

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
        Serial.print("Received: \"");
        Serial.print(text);
        Serial.println("\"");

        String from_name = bot->messages[i].from_name;

        if (text == "/start")
        {
            String welcome = "Welcome, " + from_name + ".\n";
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
                    int warmUpDurationSec = (millis() - riceCookerStartTime) / 1000;
                    String message = "Cancelled warming up rice after " + String(warmUpDurationSec / 60) + " minutes and " + String(warmUpDurationSec % 60) + " seconds.";
                    bot->sendMessage(chat_id, message, "");
                }
                if (teaMin >= 0)
                {
                    teaMin = -1;
                    relays[0].turnOff();
                    int warmUpDurationSec = (millis() - riceCookerStartTime) / 1000;
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
                if (*auto_mode == 1)
                {
                    String message = "Turning OFF auto mode...";
                    bot->sendMessage(chat_id, message, "");
                    *auto_mode = 0;
                }

                // Get current states
                lastLampState[0] = relays[1].getState();
                lastLampState[1] = relays[2].getState();

                relays[1].turnOff();
                relays[2].turnOff();
                dark_mode = 1;
                String message = "Time to immerse yourself!";
                bot->sendMessage(chat_id, message, "");
            }
            else
            {
                if (*auto_mode == 0)
                {
                    String message = "Turning ON auto mode...";
                    bot->sendMessage(chat_id, message, "");
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
                String message = "Welcome back!";
                bot->sendMessage(chat_id, message, "");
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