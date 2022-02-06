#include <Arduino.h>
#include "addr.h"

MacAddr::MacAddr()
{
    memset(_mac, 0, LEN_MACADDR);
}

MacAddr::MacAddr(const uint8_t *mac)
{
    if (mac)
        memcpy(_mac, mac, LEN_MACADDR);
    else
        memset(_mac, 0, LEN_MACADDR);
}

MacAddr::MacAddr(const String &str)
{
    uint8_t buff[LEN_MACADDR];
    uint8_t off, n;
    for (uint8_t i = 0; i < LEN_MACADDR; i++) {
        off = i * 3;
        n = strtoul(str.substring(off, off + 2).c_str(), 0, 16);
        if ((off < LEN_MACADDR - 3) && (str[off + 2] != ':')) {
            memset(_mac, 0, LEN_MACADDR);
            return;
        }
        else {
            buff[i] = n;
        }
    }
    memcpy(_mac, buff, LEN_MACADDR);
}

String MacAddr::AsStr() const
{
    char buff[24] = { 0 };
    sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
        _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
    return String(buff);
}

const uint8_t *MacAddr::AsBin() const
{
    return _mac;
}

bool MacAddr::empty()
{
    for (uint8_t i = 0; i < LEN_MACADDR; i++) {
        if (_mac != 0)
            return false;
    }
    return true;
}


IpAddr::IpAddr()
{
    memset(_ip, 0, LEN_IPADDR);
}

IpAddr::IpAddr(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4)
{
    _ip[0] = a1;
    _ip[1] = a2;
    _ip[2] = a3;
    _ip[3] = a4;
}

IpAddr::IpAddr(const uint8_t *ip)
{
    if (ip)
        memcpy(_ip, ip, LEN_IPADDR);
    else
        memset(_ip, 0, LEN_IPADDR);
}

IpAddr::IpAddr(const String &str)
{
    const char *addr = str.c_str();
    uint8_t buff[LEN_IPADDR];
    uint8_t dots = 0;
    uint16_t acc = 0;

    while (*addr)
    {
        char c = *addr++;
        if (c >= '0' && c <= '9')
        {
            acc = acc * 10 + (c - '0');
            if (acc > 255) {
                break;
            }
        }
        else if (c == '.')
        {
            if (dots == 3) {
                break;
            }
            buff[dots++] = acc;
            acc = 0;
        }
        else
            break;
    }

    if (dots != 3)
        return;

    buff[3] = acc;
    memcpy(_ip, buff, LEN_IPADDR);
}

String IpAddr::AsStr() const
{
    char buff[24] = { 0 };
    sprintf(buff, "%d.%d.%d.%d", _ip[0], _ip[1], _ip[2], _ip[3]);
    return String(buff);
}

const uint8_t *IpAddr::AsBin() const
{
    return _ip;
}

bool IpAddr::empty()
{
    for (uint8_t i = 0; i < LEN_IPADDR; i++) {
        if (_ip != 0)
            return false;
    }
    return true;
}

