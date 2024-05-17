#include "telegram.h"
#include "auto.h"
#include "secrets.h"
#include "Relay.h"

Relay relays[4] = {Relay(33, true, false), Relay(25, true, false), Relay(26, true, false), Relay(27, true, false)};
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
AsyncWebServer server(80);

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
        Serial.println("[Main] Connecting to WiFi..");
    }
    // Print ESP32 Local IP Address
    Serial.print("[Main] ");
    Serial.println(WiFi.localIP());
    bot.sendMessage(CHAT_ID, "Hello, I just woke up (●'◡'●)", "");

    // Serial.begin(&server);
    server.begin();
}

void loop()
{
    if (millis() > lastTimeBotRan + botRequestDelay)
    {
        Serial.println("[Main] Bot ready. Getting messages...");
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Serial.print("[Main] Got ");
        Serial.print(numNewMessages);
        Serial.println(" messages.");

        while (numNewMessages)
        {
            Serial.println("[Main] Got new messages, handling...");
            handleNewMessages(&bot, numNewMessages, &auto_mode, relays);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        Serial.println("[Main] Waiting before next bot run...");
        lastTimeBotRan = millis();
    }

    if (riceCookerCookingMin >= 0 && (millis() > riceCookerStartTime + riceCookerCookingMin * 60 * 1000))
    {
        relays[3].turnOff();
        bot.sendMessage(notifyChatID, "Warm rice is ready!", "");
        riceCookerCookingMin = -1;
    }

    if (auto_mode == 1)
    {
        handleAuto(&bot, relays, &stopRiceCookerWhenHome);
    }
}