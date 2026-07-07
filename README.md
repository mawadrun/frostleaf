# Frostleaf

An ESP32 home-automation companion, controlled through a Telegram bot. It
drives a 4-channel relay board (water dispenser, warm light, cold light, rice
cooker) and passively detects whether the owner is home by sniffing Wi-Fi
frames from known devices — switching lighting profiles automatically by time
of day and turning everything off when nobody is around.

## Features

- **Telegram remote control** — only your chat id is authorized:

  | Command | Action |
  | --- | --- |
  | `/start` | Welcome message + command list |
  | `/status` | Show the state of all relays |
  | `/toggle_auto` | Toggle presence-based auto mode |
  | `/toggle_cozy` | Toggle the warm light |
  | `/toggle_daylight` | Toggle the cold light |
  | `/toggle_dark` | All lights off; run again to restore the previous state |
  | `/cook` | Run the rice cooker for 120 min, then notify |
  | `/warm` | Warm up rice for 30 min, then notify |
  | `/tea` | Heat water for tea (10 min), then notify |
  | `/cancel` | Cancel a running rice/tea timer |

- **Presence detection** — the radio hops across Wi-Fi channels in
  promiscuous mode, watching for frames from the MAC addresses listed in
  `secrets.h`. No app or GPS needed.
- **Auto profiles** — when someone is home, lights follow the time of day
  (Morning / Day / Evening / Night); 15 s after the last known device
  disappears, everything switches off and the bot says goodbye.

## Hardware

- ESP32 dev board
- 4-channel relay board (active-low), wired to GPIO 33, 25, 26, 27:

  | Relay | GPIO | Appliance |
  | --- | --- | --- |
  | 1 | 33 | Water dispenser |
  | 2 | 25 | Warm light |
  | 3 | 26 | Cold light |
  | 4 | 27 | Rice cooker |

## Setup

1. Install the **Arduino IDE** and add the ESP32 board package
   (**version 3.0.7** from the board manager).
   The old Arduino extension for VS Code is discontinued — open
   `frostleaf.ino` in the Arduino IDE and upload from there.
2. Install these libraries via the Library Manager:
   - [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
   - ArduinoJson (dependency of the above)
3. Copy `secrets.h.example` to `secrets.h` and fill in your Wi-Fi
   credentials, bot token, chat id, and known device MACs.
4. Adjust `gmtOffset_sec` in `auto.h` (default UTC+7) and `maxCh` in
   `wifi-sniffer.h` (11 US / 13 EU / 14 JP) if needed.
5. Select your ESP32 board and port, then upload.

## Notes

- Phones randomize their Wi-Fi MAC per network by default. Put the
  per-network MAC your phone actually uses on your Wi-Fi into `KnownMac`,
  or disable randomization for your home network.
- Presence detection only sees a device while it's actively transmitting;
  a sleeping phone can go quiet for a while, hence the per-device TTL and
  the turn-off delay.
