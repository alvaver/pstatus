#pragma once

#include <HardwareSerial.h>
#include "taskbar.h"

#if defined(ESP8266) && !defined(ESP32)
/* Software serial is only available for ESP8266 */
#define PZEM004_SOFTSERIAL
#endif

#ifdef PZEM004_SOFTSERIAL
#include <SoftwareSerial.h>
#endif

#define PZEM_DEFAULT_ADDR   0xF8
#define PZEM_TASK_TIMER     (2 * 1000)


class Pzem : public TimerTask
{
public:

#ifdef PZEM004_SOFTSERIAL
    Pzem(uint8_t rxdPin = PZEM_RX_PIN,
         uint8_t txdPin = PZEM_TX_PIN,
         uint8_t addr = PZEM_DEFAULT_ADDR,
         uint32_t cnt = PZEM_TASK_TIMER);
    Pzem(SoftwareSerial& port,
         uint8_t addr = PZEM_DEFAULT_ADDR,
         uint32_t cnt = PZEM_TASK_TIMER);
#else
    Pzem(HardwareSerial& port = Serial2,
         uint8_t addr = PZEM_DEFAULT_ADDR,
         uint32_t cnt = PZEM_TASK_TIMER);
#endif
    ~Pzem();

    bool isCorrect()     { return _isCorecct; }
    float getVoltage()   { return _voltage; }
    float getCurrent()   { return _current; }
    float getPower()     { return _power; }
    float getEnergy()    { return _energy; }
    float getFrequency() { return _frequency; }
    float getPf()        { return _pf; }

    bool setAddress(uint8_t addr);
    uint8_t getAddress();
    uint8_t readAddress(bool update = false);
    bool setPowerAlarm(uint16_t watts);
    bool getPowerAlarm();
    bool resetEnergy();

private:
    void clearValues();
    void init(Stream* port, uint8_t addr); // Init common to all constructors
    bool updateValues();    // Get most up to date values from device registers and cache them
    uint16_t receive(uint8_t *resp, uint16_t len); // Receive len bytes into a buffer
    void sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val); // Send 8 byte command
    void setCRC(uint8_t *buf, uint16_t len);           // Set the CRC for a buffer
    bool checkCRC(const uint8_t *buf, uint16_t len);   // Check CRC of buffer
    uint16_t CRC16(const uint8_t *data, uint16_t len); // Calculate CRC of buffer

    void DbugMessage();
    void perform();

    Stream* _serial;
    uint8_t _addr;
    bool _isConnected;
    bool _isCorecct;
#ifdef PZEM004_SOFTSERIAL
    SoftwareSerial* localSWserial = nullptr;
#endif
    float _voltage;
    float _current;
    float _power;
    float _energy;
    float _frequency;
    float _pf;
    uint16_t _alarms;
};
