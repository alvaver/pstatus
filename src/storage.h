#pragma once

#include <WString.h>

#define WIFI_SSID_1 "wssid1"
#define WIFI_PASS_1 "wpass1"
#define WIFI_SSID_2 "wssid2"
#define WIFI_PASS_2 "wpass2"
#define WIFI_SSID_3 "wssid3"
#define WIFI_PASS_3 "wpass3"
#define NTP_ADDR_1 "naddr1"
#define NTP_ADDR_2 "naddr2"
#define NTP_ADDR_3 "naddr3"
#define NTP_ZONE "ntzone"
#define MQTT_ADDR "maddr"
#define MQTT_PORT "mport"
#define MQTT_USER "muser"
#define MQTT_PASS "mpass"
#define TIK_INFO "infotik"
#define TIK_SENSOR "sensortik"
#define COR_SENSOR "sensorcor"
#define HTTP_USER "huser"
#define HTTP_PASS "hpass"

class Storage
{
public:
    Storage() : _modified(false) {}
    void init();
    void clear();
    void flush();
    bool setVariable(const String &name, const String &value);
    String getStrByName(const String &name);
    int getIntByName(const String &name);
    float getFloatByName(const String &name);
    uint8_t *getMacByName(const String &name);
    String allVariables();
private:
    bool _modified;
};

extern Storage storage;
