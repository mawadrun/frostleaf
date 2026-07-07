#ifndef RELAY_H
#define RELAY_H

class Relay
{
public:
    Relay();
    Relay(int pin, bool isActiveLow = false, bool isNormallyClosed = false);
    void begin();
    bool getState();
    void turnOn();
    void turnOff();

protected:
    int pin;
    bool activeLow;
    bool normallyClosed;

private:
    void writeState(bool on);
};

#endif
