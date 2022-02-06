#pragma once

#include <stdint.h>

class Led
{
public:
    Led(uint8_t pin);
    ~Led() {}
    void init();
    void ledOn();
    void ledOff();
    void ledToggle();
private:
    const uint8_t _pin;
    bool _state;
};
