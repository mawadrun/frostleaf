#include "telegram.h"
#include "auto.h"
#include "secrets.h"
// #include "RelayModule.h"

int operand_relay = 0; // Pin number from relay[]
int relay_index = 0;   // Index of relay in relay[]
bool relay_state[4] = {HIGH, HIGH, HIGH, HIGH};

const int relay[4] = {33, 25, 26, 27};
// Credentials

void setup()
{
    Serial.begin(115200);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    for (int i = 0; i <= 3; i++)
    {
        pinMode(relay[i], OUTPUT);
        digitalWrite(relay[i], HIGH);
    }
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
            handleNewMessages(numNewMessages, &auto_mode, &operand_relay, &relay_index, relay_state, relay);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
    if (auto_mode == 1)
    {
        handleAuto(relay);
    }
}