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
String notifyChatID;

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
            welcome += "Use /options to start.\n\n";
            bot->sendMessage(chat_id, welcome, "");
        }

        if (text == "/options")
        {
            String keyboardJson = RELAY_SELECT_MENU;
            bot->sendMessageWithReplyKeyboard(chat_id, "Select Relay", "", keyboardJson, true);
        }

        if (text == "/auto")
        {
            if (*auto_mode == 1)
            {
                bot->sendMessage(chat_id, "Auto mode is already on! (. ❛ ᴗ ❛.)", "");
            }
            else
            {
                *auto_mode = 1;
                bot->sendMessage(chat_id, "Auto mode turned on! ☆*: .｡. o(≧▽≦)o .｡.:*☆", "");
            }
        }

        if (text == "/manual")
        {
            if (*auto_mode == 0)
            {
                bot->sendMessage(chat_id, "You're in control! \(￣︶￣*\))", "");
            }
            else
            {
                *auto_mode = 0;
                bot->sendMessage(chat_id, "Turned off auto mode. We're now in manual! (～￣▽￣)～", "");
            }
        }

        if (text == "/status")
        {
            String status = "Relay statuses: \n\nREL 1 - ";
            status += relays[0].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nREL 2 - ";
            status += relays[1].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nREL 3 - ";
            status += relays[2].getState() ? "🟦 ON" : "🟥 OFF";
            status += "\nREL 4 - ";
            status += relays[3].getState() ? "🟦 ON" : "🟥 OFF";
            bot->sendMessage(chat_id, status, "");
        }

        if (text == "/warm")
        {
            bot->sendMessage(chat_id, "Usage: /warm <cancel (optional)> <turn off when I'm home (y/n)> <duration in minutes (if n)>", "");
        }

        if (text.substring(0, 6) == "/warm ")
        {
            if (text[6] == 'y')
            {
                stopRiceCookerWhenHome = true;
                relays[3].turnOn();
                bot->sendMessage(chat_id, "I'm warming up the rice! Will stop once you're home ~", "");
            }
            else if (text[6] == 'n')
            {
                stopRiceCookerWhenHome = false;
                riceCookerCookingMin = text.substring(8, 11).toInt();
                riceCookerStartTime = millis();
                relays[3].turnOn();
                String message = "I will warm up the rice for " + String(riceCookerCookingMin) + " minutes!";
                bot->sendMessage(chat_id, message, "");
                notifyChatID = chat_id;
            }
            else if (text.substring(6, 12) == "cancel")
            {
                if (riceCookerCookingMin < 0)
                {
                    bot->sendMessage(chat_id, "Nothing to cancel!", "");
                }
                else
                {
                    riceCookerCookingMin = -1;
                    relays[3].turnOff();
                    int warmUpDurationSec = (millis() - riceCookerStartTime) / 1000;
                    String message = "Cancelled warming up rice after " + String(warmUpDurationSec / 60) + " minutes and " + String(warmUpDurationSec % 60) + " seconds.";
                    bot->sendMessage(chat_id, message, "");
                }
            }
        }

        if (text.substring(0, 3) == "REL")
        {
            Serial.println("received RELAY");
            relay_index = (int)(text[4]) - 48 - 1;
            Serial.print("Operating on relay ");
            Serial.println(relay_index + 1);
            String keyboardJson = RELAY_OPERATION_MENU;
            bot->sendMessageWithReplyKeyboard(chat_id, "Select operation", "", keyboardJson, true);
        }

        if (text == "🟦 ON")
        {
            bot->sendMessage(chat_id, "Relay state set to ON", "");
            Serial.print("Relay ");
            Serial.print(relay_index + 1);
            Serial.println(" set to ON.");
            relays[relay_index].turnOn();
            bot->sendMessageWithReplyKeyboard(chat_id, "", "", "", true); // hides reply keyboard
        }

        if (text == "🟥 OFF")
        {
            bot->sendMessage(chat_id, "Relay state set to OFF", "");
            Serial.print("Relay ");
            Serial.print(relay_index + 1);
            Serial.println(" set to OFF.");
            relays[relay_index].turnOff();
            bot->sendMessageWithReplyKeyboard(chat_id, "", "", "", true); // hides reply keyboard
        }
    }
}
#endif