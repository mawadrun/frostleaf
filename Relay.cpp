#include <Arduino.h>
#include "Relay.h"

Relay::Relay() {}
Relay::Relay(int pin, bool isActiveLow, bool isNormallyClosed)
{
    this->pin = pin;
    this->activeLow = isActiveLow;
    this->normallyClosed = isNormallyClosed;
}

// Translates the logical "on" state to the pin level, accounting for
// active-low driver boards and normally-closed wiring.
void Relay::writeState(bool on)
{
    bool level = on;
    if (normallyClosed)
        level = !level;
    if (activeLow)
        level = !level;
    digitalWrite(pin, level);
}

bool Relay::getState()
{
    bool on = digitalRead(pin);
    if (normallyClosed)
        on = !on;
    if (activeLow)
        on = !on;
    return on;
}

void Relay::turnOn()
{
    writeState(true);
}

void Relay::turnOff()
{
    writeState(false);
}

void Relay::begin()
{
    pinMode(pin, OUTPUT);
    writeState(false);
}
