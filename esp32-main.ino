#include "telegram.h"
#include "auto.h"
#include "secrets.h"
#include "Relay.h"

Relay relays[4] = {Relay(33, true, false), Relay(25, true, false), Relay(26, true, false), Relay(27, true, false)};
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Flags
int auto_mode = 1;

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

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
    bot.sendMessage(CHAT_ID, "Hello, I just woke up (●'◡'●)", "");
}

void loop()
{
    if (millis() > lastTimeBotRan + botRequestDelay)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages)
        {
            handleNewMessages(&bot, numNewMessages, &auto_mode, relays);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
    if (auto_mode == 1)
    {
        handleAuto(&bot, relays);
    }
}