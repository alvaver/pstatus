#include "pzem.h"

#define REG_VOLTAGE     0x0000
#define REG_CURRENT_L   0x0001
#define REG_CURRENT_H   0X0002
#define REG_POWER_L     0x0003
#define REG_POWER_H     0x0004
#define REG_ENERGY_L    0x0005
#define REG_ENERGY_H    0x0006
#define REG_FREQUENCY   0x0007
#define REG_PF          0x0008
#define REG_ALARM       0x0009

#define CMD_RHR         0x03
#define CMD_RIR         0X04
#define CMD_WSR         0x06
#define CMD_CAL         0x41
#define CMD_REST        0x42

#define WREG_ALARM_THR  0x0001
#define WREG_ADDR       0x0002
#define READ_TIMEOUT    100
#define INVALID_ADDRESS 0x00
#define PZEM_BAUD_RATE  9600

//#define DEBUG
#ifdef DEBUG
#define __SHOW_DUMP__ showDump(this)
    void showDump(Pzem *p)
    {
        Serial.printf("[PZEM] Voltage: %0.1f(V)\n", p->getVoltage());
        Serial.printf("[PZEM] Current: %0.3f(A)\n", p->getCurrent());
        Serial.printf("[PZEM] Power: %0.1f(W)\n", p->getPower());
        Serial.printf("[PZEM] Energy: %0.0f(Wh)\n", p->getEnergy());
        Serial.printf("[PZEM] Frequency: %0.1f(Hz)\n", p->getFrequency());
        Serial.printf("[PZEM] PF: %0.2f\n", p->getPf());
    }
#else
#define __SHOW_DUMP__
#endif /* DEBUG */

#ifdef PZEM004_SOFTSERIAL
Pzem::Pzem(uint8_t rxdPin, uint8_t txdPin,
         uint8_t addr, uint32_t cnt) :
    TimerTask(cnt)
{
    localSWserial = new SoftwareSerial(rxdPin, txdPin);
    localSWserial->begin(PZEM_BAUD_RATE);
    init((Stream *)localSWserial, addr);
    startTaskTimer();
}

Pzem::Pzem(SoftwareSerial& port, uint8_t addr, uint32_t cnt) :
    TimerTask(cnt)
{
    port.begin(PZEM_BAUD_RATE);
    init((Stream *)&port, addr);
    startTaskTimer();
}

Pzem::~Pzem()
{
    if(localSWserial != nullptr){
        delete localSWserial;
    }
}

#else
Pzem::Pzem(HardwareSerial& port, uint8_t addr, uint32_t cnt) :
    TimerTask(cnt)
{
    port.begin(PZEM_BAUD_RATE);
    init((Stream *)&port, addr);
    startTaskTimer();
}

Pzem::~Pzem()
{
}
#endif /* PZEM004_SOFTSERIAL */

void Pzem::clearValues()
{
    _voltage = 0;
    _current = 0;
    _power = 0;
    _energy = 0;
    _frequency = 0;
    _pf = 0;
    _alarms = 0;
}

void Pzem::init(Stream* port, uint8_t addr){
    if(addr < 0x01 || addr > 0xF8)
        addr = PZEM_DEFAULT_ADDR;
    _addr = addr;

    this->_serial = port;
    _isConnected = false;
    _isCorecct = false;
}

bool Pzem::resetEnergy(){
    uint8_t buffer[] = {0x00, CMD_REST, 0x00, 0x00};
    uint8_t reply[5];
    buffer[0] = _addr;

    setCRC(buffer, 4);
    _serial->write(buffer, 4);

    uint16_t length = receive(reply, 5);

    if(length == 0 || length == 5){
        return false;
    }

    return true;
}

bool Pzem::setAddress(uint8_t addr)
{
    uint8_t response[8];

    sendCmd8(CMD_WSR, WREG_ADDR, addr);
    if(receive(response, 8) != 8)
        return false;

    _addr = addr;
    return true;
}

uint8_t Pzem::readAddress(bool update)
{
    uint8_t response[7];
    uint8_t addr = 0;

    sendCmd8(CMD_RHR, WREG_ADDR, 0x01);
    if(receive(response, 7) != 7)
        return INVALID_ADDRESS;

    addr = ((uint32_t)response[3] << 8 |
                              (uint32_t)response[4]);

    if(update){
        _addr = addr;
    }
    return addr;
}

uint8_t Pzem::getAddress()
{
    return _addr;
}

bool Pzem::setPowerAlarm(uint16_t watts)
{
    uint8_t response[8];
    if (watts > 25000){
        watts = 25000;
    }

    sendCmd8(CMD_WSR, WREG_ALARM_THR, watts);
    if(receive(response, 8) != 8)
        return false;

    return true;
}

bool Pzem::getPowerAlarm()
{
    if(!updateValues())
        return false;

    return _alarms != 0x0000;
}

bool Pzem::updateValues()
{
    uint8_t response[25];

    sendCmd8(CMD_RIR, 0x00, 0x0A);
    if(receive(response, 25) != 25){
        return false;
    }

    _voltage = ((uint32_t)response[3] << 8 | // Raw voltage in 0.1V
                 (uint32_t)response[4])/10.0;

    _current = ((uint32_t)response[5] << 8 | // Raw current in 0.001A
                 (uint32_t)response[6] |
                 (uint32_t)response[7] << 24 |
                 (uint32_t)response[8] << 16) / 1000.0;

    _power =   ((uint32_t)response[9] << 8 | // Raw power in 0.1W
                 (uint32_t)response[10] |
                 (uint32_t)response[11] << 24 |
                 (uint32_t)response[12] << 16) / 10.0;

    _energy =  ((uint32_t)response[13] << 8 | // Raw Energy in 1Wh
                 (uint32_t)response[14] |
                 (uint32_t)response[15] << 24 |
                 (uint32_t)response[16] << 16) / 1000.0;

    _frequency=((uint32_t)response[17] << 8 | // Raw Frequency in 0.1Hz
                 (uint32_t)response[18]) / 10.0;

    _pf =      ((uint32_t)response[19] << 8 | // Raw pf in 0.01
                 (uint32_t)response[20])/100.0;

    _alarms =  ((uint32_t)response[21] << 8 | // Raw alarm value
                 (uint32_t)response[22]);
    return true;
}

void Pzem::sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val)
{
    uint8_t sendBuffer[8];

    sendBuffer[0] = _addr;
    sendBuffer[1] = cmd;
    sendBuffer[2] = (rAddr >> 8) & 0xFF;
    sendBuffer[3] = (rAddr) & 0xFF;
    sendBuffer[4] = (val >> 8) & 0xFF;
    sendBuffer[5] = (val) & 0xFF;
    setCRC(sendBuffer, 8);

    _serial->write(sendBuffer, 8);
}

uint16_t Pzem::receive(uint8_t *resp, uint16_t len)
{
    #ifdef PZEM004_SOFTSERIAL
    ((SoftwareSerial *)_serial)->listen();
    #endif
    unsigned long startTime = millis();
    uint8_t bytes = 0;
    while((bytes < len) && (millis() - startTime < READ_TIMEOUT))
    {
        if(_serial->available() > 0)
        {
            uint8_t c = (uint8_t)_serial->read();
            resp[bytes++] = c;
        }
        yield();
    }
    if(!checkCRC(resp, bytes)) {
        _isConnected = false;
        return 0;
    }
    _isConnected = true;
    return bytes;
}

static const uint16_t crcTable[] PROGMEM = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

uint16_t Pzem::CRC16(const uint8_t *data, uint16_t len)
{
    uint8_t nTemp;
    uint16_t crc = 0xFFFF;

    while (len--)
    {
        nTemp = *data++ ^ crc;
        crc >>= 8;
        crc ^= (uint16_t)pgm_read_word(&crcTable[nTemp]);
    }
    return crc;
}

bool Pzem::checkCRC(const uint8_t *buf, uint16_t len)
{
    if(len <= 2)
        return false;

    uint16_t crc = CRC16(buf, len - 2);
    return ((uint16_t)buf[len-2] | (uint16_t)buf[len-1] << 8) == crc;
}

void Pzem::setCRC(uint8_t *buf, uint16_t len)
{
    if(len <= 2)
        return;

    uint16_t crc = CRC16(buf, len - 2);
    buf[len - 2] = crc & 0xFF;
    buf[len - 1] = (crc >> 8) & 0xFF;
}

void Pzem::perform()
{
    if(checkTaskTimer()) {
        if(!_isConnected) {
            clearValues();
            setAddress(_addr);
        }
        else {
            _isCorecct = updateValues();
            __SHOW_DUMP__;
        }
        restartTaskTimer();
    }
}
