#include "telegram.h"
#include "auto.h"
#include "secrets.h"
#include "Relay.h"
// #include "RelayModule.h"

int operand_relay = 0; // Pin number from relay[]
int relay_index = 0;   // Index of relay in relay[]

Relay relays[4] = {Relay(33, true, false), Relay(25, true, false), Relay(26, true, false), Relay(27, true, false)};

// Relay relaytest(25, true, false);

void setup()
{
    Serial.begin(115200);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    for (int i = 0; i <= 3; i++)
    {
        relays[i].begin();
    }
    // pinMode(25, OUTPUT);
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
}

void loop()
{
    if (millis() > lastTimeBotRan + botRequestDelay)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        while (numNewMessages)
        {
            handleNewMessages(numNewMessages, &auto_mode, &operand_relay, &relay_index, relays);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
    if (auto_mode == 1)
    {
        handleAuto(relays);
    }

    // relaytest.turnOn();
    // Serial.println("ON");
    // delay(1000);
    // relaytest.turnOff();
    // Serial.println("OFF");
    // delay(1000);
}