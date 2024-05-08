#ifndef TELEGRAM
#define TELEGRAM

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>          // Initialize Telegram BOT
#include "secrets.h"
#include "Relay.h"

#define RELAY_SELECT_MENU "[[\"REL 1 - None\", \"REL 2 - Warm light\"],[\"REL 3 - Cold light\", \"REL 4 - Rice cooker\"]]"
#define RELAY_OPERATION_MENU "[[\"ON\", \"OFF\"],[\"Status\"]]"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 100;
unsigned long lastTimeBotRan;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages, int *auto_mode, int *operand_relay, int *relay_index, Relay *relays)
{
    // Serial.print("Received: ");
    // Serial.println(String(numNewMessages));

    for (int i = 0; i < numNewMessages; i++)
    {
        // Chat id of the requester
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID)
        {
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.print("Received: \"");
        Serial.print(text);
        Serial.println("\"");

        String from_name = bot.messages[i].from_name;

        if (text == "/start")
        {
            String welcome = "Welcome, " + from_name + ".\n";
            welcome += "Use /options to start.\n\n";
            bot.sendMessage(chat_id, welcome, "");
        }

        if (text == "/options")
        {
            String keyboardJson = RELAY_SELECT_MENU;
            bot.sendMessageWithReplyKeyboard(chat_id, "Select Relay", "", keyboardJson, true);
        }

        if (text == "/auto")
        {
            if (*auto_mode == 1)
            {
                bot.sendMessage(chat_id, "Auto mode is already on! (. ❛ ᴗ ❛.)", "");
            }
            else
            {
                *auto_mode = 1;
                bot.sendMessage(chat_id, "Auto mode turned on! ☆*: .｡. o(≧▽≦)o .｡.:*☆", "");
            }
        }

        if (text == "/manual")
        {
            if (*auto_mode == 0)
            {
                bot.sendMessage(chat_id, "You're in control! \(￣︶￣*\))", "");
            }
            else
            {
                *auto_mode = 0;
                bot.sendMessage(chat_id, "Turned off auto mode. We're now in manual! (～￣▽￣)～", "");
            }
        }

        if (text.substring(0, 3) == "REL")
        {
            if (*auto_mode == 1)
            {
                bot.sendMessage(chat_id, "Can't do that! Turn off auto mode first using /manual", "");
            }
            else
            {
                Serial.println("received RELAY");
                *relay_index = (int)(text[4]) - 48 - 1;
                Serial.print("Operating on relay ");
                Serial.println(*relay_index + 1);
                String keyboardJson = RELAY_OPERATION_MENU;
                bot.sendMessageWithReplyKeyboard(chat_id, "Select operation", "", keyboardJson, true);
            }
        }

        if (text == "ON")
        {
            bot.sendMessage(chat_id, "Relay state set to ON", "");
            Serial.print("Relay ");
            Serial.print(*relay_index + 1);
            Serial.println(" set to ON.");
            relays[*relay_index].turnOn();
            String keyboardJson = RELAY_SELECT_MENU;
            bot.sendMessageWithReplyKeyboard(chat_id, "Select Relay", "", keyboardJson, true);
        }

        if (text == "OFF")
        {
            bot.sendMessage(chat_id, "Relay state set to OFF", "");
            Serial.print("Relay ");
            Serial.print(*relay_index + 1);
            Serial.println(" set to OFF.");
            relays[*relay_index].turnOff();
            String keyboardJson = RELAY_SELECT_MENU;
            bot.sendMessageWithReplyKeyboard(chat_id, "Select Relay", "", keyboardJson, true);
        }

        if (text == "Status")
        {
            if (!relays[*relay_index].getState())
            {
                bot.sendMessage(chat_id, "Relay is OFF", "");
            }
            else
            {
                bot.sendMessage(chat_id, "Relay is ON", "");
            }
            String keyboardJson = RELAY_SELECT_MENU;
            bot.sendMessageWithReplyKeyboard(chat_id, "Select Relay", "", keyboardJson, true);
        }
    }
}
#endif