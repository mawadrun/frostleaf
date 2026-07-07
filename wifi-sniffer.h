#ifndef WIFI_SNIFFER_H
#define WIFI_SNIFFER_H

// Passive presence detection: puts the radio in promiscuous mode and watches
// for frames whose transmitter address matches a known device (see KnownMac
// in secrets.h). auto.h hops through the channels one per second.

#include <WiFi.h>
#include "esp_wifi.h"
#include "secrets.h"

#define maxCh 13 // Highest Wi-Fi channel to scan: US = 11, EU = 13, Japan = 14

// Seconds a device stays "online" after its last sniffed frame
const int deviceTTL = 30;

const int knownMacCount = sizeof(KnownMac) / sizeof(KnownMac[0]);
uint8_t knownMacBytes[knownMacCount][6];

struct SniffedDevice
{
    uint8_t mac[6];
    volatile int ttl;    // refreshed by the sniffer callback, decremented each tick
    int secondsOnline;   // approx. time the device has been seen online
    bool online;
};

const int maxDevices = 64;
SniffedDevice devices[maxDevices];
int deviceCount = 0;

int curChannel = 1;

const wifi_promiscuous_filter_t snifferFilter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA};

// Generic 802.11 header layout, enough to reach the transmitter address (sa)
typedef struct
{
    int16_t fctl;
    int16_t duration;
    uint8_t da[6];
    uint8_t sa[6];
    uint8_t bssid[6];
    int16_t seqctl;
    unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

String macToString(const uint8_t *mac)
{
    char buf[13];
    snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

// Parses "A1B2C3D4E5F6" (colons/dashes/spaces allowed) into 6 bytes
void parseMac(const String &str, uint8_t *out)
{
    int nibble = 0;
    for (unsigned int i = 0; i < str.length() && nibble < 12; i++)
    {
        char c = str[i];
        int val;
        if (c >= '0' && c <= '9')
            val = c - '0';
        else if (c >= 'A' && c <= 'F')
            val = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            val = c - 'a' + 10;
        else
            continue; // skip separators
        if (nibble % 2 == 0)
            out[nibble / 2] = val << 4;
        else
            out[nibble / 2] |= val;
        nibble++;
    }
}

// Runs in the Wi-Fi task for every sniffed frame — must be fast and must not
// allocate, so devices are tracked in a fixed array of raw MAC bytes.
void sniffer(void *buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t *)buf;
    if (p->rx_ctrl.sig_len < (int)sizeof(WifiMgmtHdr))
        return;
    WifiMgmtHdr *wh = (WifiMgmtHdr *)p->payload;

    for (int i = 0; i < deviceCount; i++)
    {
        if (memcmp(devices[i].mac, wh->sa, 6) == 0)
        {
            devices[i].ttl = deviceTTL;
            devices[i].online = true;
            return;
        }
    }

    if (deviceCount < maxDevices)
    {
        memcpy(devices[deviceCount].mac, wh->sa, 6);
        devices[deviceCount].ttl = deviceTTL;
        devices[deviceCount].secondsOnline = 0;
        devices[deviceCount].online = true;
        deviceCount++;
    }
}

void setupSniffer()
{
    for (int i = 0; i < knownMacCount; i++)
    {
        parseMac(KnownMac[i][1], knownMacBytes[i]);
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&snifferFilter);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);

    Serial.println("[Snfr] Sniffer started");
}

// Called roughly once per second: ages every device and marks the stale ones
// offline once their TTL runs out.
void snifferTick()
{
    for (int i = 0; i < deviceCount; i++)
    {
        if (!devices[i].online)
            continue;
        devices[i].secondsOnline++;
        devices[i].ttl--;
        if (devices[i].ttl <= 0)
        {
            devices[i].online = false;
            devices[i].secondsOnline = 0;
        }
    }
}

// True if any device from KnownMac is currently online; logs matches.
bool knownDevicePresent()
{
    bool present = false;
    for (int i = 0; i < deviceCount; i++)
    {
        if (!devices[i].online)
            continue;
        for (int j = 0; j < knownMacCount; j++)
        {
            if (memcmp(devices[i].mac, knownMacBytes[j], 6) == 0)
            {
                present = true;
                Serial.println("[Snfr] " + KnownMac[j][0] + " : " + macToString(devices[i].mac) + " : " + String(devices[i].secondsOnline) + "s");
            }
        }
    }
    return present;
}

#endif
