#include <Arduino.h>
#include "Relay.h"

Relay::Relay() {}
Relay::Relay(int pin, bool isActiveLow = false, bool isNormallyClosed = false)
{
    this->pin = pin;
    this->activeLow = isActiveLow;
    this->normallyClosed = isNormallyClosed;
}
bool Relay::getState()
{
    bool val;
    bool pinState = digitalRead(pin);
    if (activeLow)
    {
        val = normallyClosed ? pinState : !pinState;
    }
    else
    {
        val = normallyClosed ? !pinState : pinState;
    }
    return val;
}
void Relay::turnOn()
{
    bool val;
    if (activeLow)
    {
        val = normallyClosed ? 1 : 0;
    }
    else
    {
        val = normallyClosed ? 0 : 1;
    }
    digitalWrite(pin, val);
}
void Relay::turnOff()
{
    bool val;
    if (activeLow)
    {
        val = normallyClosed ? 0 : 1;
    }
    else
    {
        val = normallyClosed ? 1 : 0;
    }
    digitalWrite(pin, val);
}
void Relay::begin()
{
    bool val;
    pinMode(pin, OUTPUT);
    if (activeLow)
    {
        val = normallyClosed ? 0 : 1;
    }
    else
    {
        val = normallyClosed ? 1 : 0;
    }
    digitalWrite(pin, val);
}