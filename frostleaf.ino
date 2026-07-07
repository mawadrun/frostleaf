#include "telegram.h"
#include "auto.h"
#include "secrets.h"
#include "Relay.h"

Relay relays[4] = {Relay(33, true, false), Relay(25, true, false), Relay(26, true, false), Relay(27, true, false)};
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Flags
int auto_mode = 1;

// Retry Wi-Fi this often after the connection drops
const unsigned long wifiReconnectDelay = 30000;
unsigned long lastWifiReconnectAttempt = 0;

void setup()
{
    Serial.begin(115200);
    // Time config
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // Setup relays
    for (int i = 0; i <= 3; i++)
    {
        relays[i].begin();
    }
    // Setup handleAuto()
    setupAuto();
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

    // Restart if the connection can't be established, instead of hanging forever
    unsigned long wifiConnectStart = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("[Main] Connecting to WiFi..");
        if (millis() - wifiConnectStart > 60000)
        {
            Serial.println("[Main] WiFi connection timed out, restarting...");
            ESP.restart();
        }
    }
    // Print ESP32 Local IP Address
    Serial.print("[Main] ");
    Serial.println(WiFi.localIP());
    bot.sendMessage(CHAT_ID, "Hello, I just woke up (●'◡'●)", "");
}

void loop()
{
    // Recover from Wi-Fi drops so the bot keeps working unattended
    if (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - lastWifiReconnectAttempt > wifiReconnectDelay)
        {
            Serial.println("[Main] WiFi disconnected, reconnecting...");
            WiFi.reconnect();
            lastWifiReconnectAttempt = millis();
        }
    }
    else if (millis() - lastTimeBotRan > botRequestDelay)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages)
        {
            Serial.println("[Main] Got new messages, handling...");
            handleNewMessages(&bot, numNewMessages, &auto_mode, relays);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }

    if (riceCookerCookingMin >= 0 && (millis() - riceCookerStartTime >= (unsigned long)riceCookerCookingMin * 60000UL))
    {
        relays[3].turnOff();
        bot.sendMessage(notifyChatID, "Warm rice is ready!", "");
        riceCookerCookingMin = -1;
    }

    if (teaMin >= 0 && (millis() - teaStartTime >= (unsigned long)teaMin * 60000UL))
    {
        relays[0].turnOff();
        bot.sendMessage(notifyChatID, "Warm water for your tea is ready!", "");
        teaMin = -1;
    }

    if (auto_mode == 1)
    {
        handleAuto(&bot, relays, &stopRiceCookerWhenHome);
    }
}
