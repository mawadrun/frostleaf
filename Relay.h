#ifndef RELAY_H
#define RELAY_H

class Relay
{
protected:
    int pin;
    bool activeLow;
    bool normallyClosed;

public:
    Relay();
    Relay(int pin, bool isActiveLow, bool isNormallyClosed);
    bool getState();
    void turnOn();
    void turnOff();
};

#endif