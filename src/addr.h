#pragma once

#include <WString.h>

#define LEN_MACADDR 6
#define LEN_IPADDR 4

class MacAddr
{
public:
    MacAddr();
    MacAddr(const uint8_t *mac);
    MacAddr(const String &mac);
    String AsStr() const;
    const uint8_t *AsBin() const;
    bool empty();
private:
    uint8_t _mac[LEN_MACADDR];
};

class IpAddr
{
public:
    IpAddr();
    IpAddr(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
    IpAddr(const uint8_t *ip);
    IpAddr(const String &ip);
    String AsStr() const;
    const uint8_t *AsBin() const;
    bool empty();
private:
    uint8_t _ip[LEN_IPADDR];
};
