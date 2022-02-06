#include <Arduino.h>
#include "led.h"

Led::Led(uint8_t pin) : _pin(pin)
{
}

void Led::init()
{
  pinMode(_pin, OUTPUT);
#ifdef ESP8266
  ledOff();
#endif
}

void Led::ledOn()
{
    _state = true;
#ifdef ESP8266
    digitalWrite(_pin, LOW);
#else
    digitalWrite(_pin, HIGH);
#endif
}

void Led::ledOff()
{
    _state = false;
#ifdef ESP8266
    digitalWrite(_pin, HIGH);
#else
    digitalWrite(_pin, LOW);
#endif
}

void Led::ledToggle()
{
    if (_state)
        ledOff();
    else
        ledOn();
}
