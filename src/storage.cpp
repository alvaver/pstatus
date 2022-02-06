#include <Arduino.h>
#include <EEPROM.h>
#include "storage.h"
#ifdef EXT_ITEMS
#include "addr.h"
#endif
#define LEN_USER 10
#define LEN_ADDR 20
#define LEN_MAC LEN_MACADDR

Storage storage;

namespace
{
    #pragma pack(push,1)

    const char magic[4] = { 'E', 'p', 'r', 0 };

    struct
    {
        uint8_t magic[4];

        uint8_t wifiEn: 1;
        uint8_t mqttEn: 1;

        int8_t ntpZone;
        int16_t mqttPort;

        uint8_t wifiSsid1[LEN_USER];
        uint8_t wifiPass1[LEN_USER];
        uint8_t wifiSsid2[LEN_USER];
        uint8_t wifiPass2[LEN_USER];
        uint8_t wifiSsid3[LEN_USER];
        uint8_t wifiPass3[LEN_USER];

        uint8_t ntpAddr1[LEN_ADDR];
        uint8_t ntpAddr2[LEN_ADDR];
        uint8_t ntpAddr3[LEN_ADDR];

        uint8_t mqttAddr[LEN_ADDR];
        uint8_t mqttUser[LEN_USER];
        uint8_t mqttPass[LEN_USER];

        uint8_t httpUser[LEN_USER];
        uint8_t httpPass[LEN_USER];

        int16_t sensorTime;
        int16_t infoTime;
        float EnergyCorr;

    } eeprom;

    #pragma pack(pop)

    enum item_type : uint8_t
    {
        ITEM_BOOLEAN,
        ITEM_INTEGER,
        ITEM_FLOAT,
        ITEM_STRING,
        ITEM_MACADDR,
    };

    struct item_t
    {
        const char *name;
        const char *defValue;
        void *eepromPtr;
        uint8_t len;
        item_type type;

        String toStr() const
        {
            if (type == ITEM_INTEGER) {
                if (len == sizeof(int8_t))
                    return String(*(int8_t *)eepromPtr, DEC);
                else if (len == sizeof(int16_t))
                    return String(*(int16_t *)eepromPtr, DEC);
                else if (len == sizeof(int32_t))
                    return String(*(int32_t *)eepromPtr, DEC);
                else if (len == sizeof(int32_t))
                    return String(*(int32_t *)eepromPtr, DEC);
                else
                    return "";
            }
            else if (type == ITEM_FLOAT) {
                    float value = *(float *)eepromPtr;
                    if (isnan(value))
                        value = 0;
                    return String(value, 1);
            }
            else if (type == ITEM_STRING) {
                String Str;
                char *str = (char *)eepromPtr;
                for (unsigned i = 0; i < len; i++) {
                    if (str[i])
                        Str += str[i];
                    else
                        break;
                }
                return Str;
            }
            else if (type == ITEM_BOOLEAN) {
                return *(bool *)eepromPtr ? "y" : "n";
            }
#ifdef EXT_ITEMS
            else if (type == ITEM_MACADDR) {
                MacAddr mac((uint8_t *)eepromPtr);
                return mac.AsStr();
            }
#endif
            else
                return "";
        }

        int toInt() const
        {
            if (type == ITEM_INTEGER) {
                if (len == sizeof(int8_t))
                    return *(int8_t *)eepromPtr;
                else if (len == sizeof(int16_t))
                    return *(int16_t *)eepromPtr;
                else if (len == sizeof(int32_t))
                    return *(int32_t *)eepromPtr;
                else if (len == sizeof(int32_t))
                    return *(int32_t *)eepromPtr;
                else if (len == sizeof(int64_t))
                    return *(int64_t *)eepromPtr;
                else
                    return 0;
            }
            else if (type == ITEM_STRING) {
                String Str;
                char *str = (char *)eepromPtr;
                for (unsigned i = 0; i < len; i++) {
                    if (str[i])
                        Str[i] = str[i];
                    else
                        break;
                }
                return Str.toInt();
            }
            else if (type == ITEM_BOOLEAN) {
                return *(bool *)eepromPtr ? true : false;
            }
            return 0;
        }

        float toFloat() const
        {
            if (type == ITEM_FLOAT) {
                float value = *(float *)eepromPtr;
                return (isnan(value)) ? 0 : value;
            }
            return 0;
        }
#ifdef EXT_ITEMS
        uint8_t *toMac() const
        {
            if (type == ITEM_MACADDR) {
                return (uint8_t *)eepromPtr;
            }
            return 0;
        }
#endif
        bool setValue(const String &value)
        {
            if (type == ITEM_INTEGER) {
                if (len == sizeof(int8_t)) {
                    *(int8_t *)eepromPtr = value.toInt();
                    return true;
                }
                else if (len == sizeof(int16_t)) {
                    *(int16_t *)eepromPtr = value.toInt();
                    return true;
                }
                else if (len == sizeof(int32_t)) {
                    *(int32_t *)eepromPtr = value.toInt();
                    return true;
                }
                else if (len == sizeof(int32_t)) {
                    *(int32_t *)eepromPtr = value.toInt();
                    return true;
                }
                else if (len == sizeof(int64_t)) {
                    *(int64_t *)eepromPtr = value.toInt();
                    return true;
                }
                else
                    return false;
            }
            else if (type == ITEM_FLOAT) {
                *(float *)eepromPtr = value.toFloat();
                return true;
            }
            else if (type == ITEM_STRING) {
                char *str = (char *)eepromPtr;
                memset(str, 0, len);
                for (unsigned i = 0; i < value.length() && i < len; i++) {
                    str[i] = value[i];
                }
                return true;
            }
            else if (type == ITEM_BOOLEAN) {
                *(bool *)eepromPtr = (value == "true" || value == "yes" || value == "enabled");
                return true;
            }
#ifdef EXT_ITEMS
            else if (type == ITEM_MACADDR) {
                MacAddr mac(value);
                if (mac.empty()) {
                    return false;
                }
                memcpy((uint8_t *)eepromPtr, mac.AsBin(), LEN_MAC);
                return true;
            }
#endif
            return false;
        }
    };

    const item_t itemArray[] = {
    { WIFI_SSID_1, "home", &eeprom.wifiSsid1, sizeof(eeprom.wifiSsid1), ITEM_STRING },
    { WIFI_PASS_1, "", &eeprom.wifiPass1, sizeof(eeprom.wifiPass1), ITEM_STRING },
    { WIFI_SSID_2, "", &eeprom.wifiSsid2, sizeof(eeprom.wifiSsid2), ITEM_STRING },
    { WIFI_PASS_2, "", &eeprom.wifiPass2, sizeof(eeprom.wifiPass2), ITEM_STRING },
    { WIFI_SSID_3, "", &eeprom.wifiSsid3, sizeof(eeprom.wifiSsid3), ITEM_STRING },
    { WIFI_PASS_3, "", &eeprom.wifiPass3, sizeof(eeprom.wifiPass3), ITEM_STRING },

    { NTP_ADDR_1, "europe.pool.ntp.org", &eeprom.ntpAddr1, sizeof(eeprom.ntpAddr1), ITEM_STRING },
    { NTP_ADDR_2, "north-america.pool.ntp.org", &eeprom.ntpAddr2, sizeof(eeprom.ntpAddr2), ITEM_STRING },
    { NTP_ADDR_3, "pool.ntp.org", &eeprom.ntpAddr3, sizeof(eeprom.ntpAddr3), ITEM_STRING },
    { NTP_ZONE, "0", &eeprom.ntpZone, sizeof(eeprom.ntpZone), ITEM_INTEGER },

    { MQTT_ADDR, "", &eeprom.mqttAddr, sizeof(eeprom.mqttAddr), ITEM_STRING },
    { MQTT_PORT, "1883", &eeprom.mqttPort, sizeof(eeprom.mqttPort), ITEM_INTEGER },
    { MQTT_USER, "", &eeprom.mqttUser, sizeof(eeprom.mqttUser), ITEM_STRING },
    { MQTT_PASS, "", &eeprom.mqttPass, sizeof(eeprom.mqttPass), ITEM_STRING },

    { HTTP_USER, "", &eeprom.httpUser, sizeof(eeprom.httpUser), ITEM_STRING },
    { HTTP_PASS, "", &eeprom.httpPass, sizeof(eeprom.httpPass), ITEM_STRING },

    { TIK_SENSOR, "5", &eeprom.sensorTime, sizeof(eeprom.sensorTime), ITEM_INTEGER },
    { TIK_INFO, "30", &eeprom.infoTime, sizeof(eeprom.infoTime), ITEM_INTEGER },
    { COR_SENSOR, "0.0", &eeprom.EnergyCorr, sizeof(eeprom.EnergyCorr), ITEM_FLOAT },

    };
    const unsigned itemCount = sizeof(itemArray) / sizeof(item_t);

//#define DEBUG
#ifdef DEBUG
#define __SHOW_DUMP__ showDump(__FUNCTION__)
    void showDump(const char* note)
    {
        char *buff = (char *)&eeprom;
        Serial.printf("[%s]", note);
        for (unsigned i = 0; i < sizeof(eeprom); i++) {
            if(i%32)
                Serial.printf("%02x ", buff[i]);
            else
                Serial.printf("\n%02x ", buff[i]);
        }
        Serial.println();
    }
#else
#define __SHOW_DUMP__
#endif /* DEBUG */

    bool isMagicValid()
    {
        for (uint8_t i = 0; i < 3; i++) {
            if (eeprom.magic[i] != magic[i])
                return false;
        }
        return true;
    }

    void setMagic()
    {
        for (uint8_t i = 0; i < 3; i++)
            eeprom.magic[i] = magic[i];
    }

    void setAllAsDefault()
    {
        for (unsigned i = 0; i < itemCount; i++) {
            item_t *item = (item_t *)&itemArray[i];
            item->setValue(item->defValue);
        }
    }

    void writeToEeprom()
    {
        setMagic();
        __SHOW_DUMP__;
        EEPROM.begin(sizeof(eeprom));
        EEPROM.put(0, eeprom);
        EEPROM.commit();
        EEPROM.end();
    }

    void readFromEeprom()
    {
        EEPROM.begin(sizeof(eeprom));
        EEPROM.get(0, eeprom);
        EEPROM.end();
        __SHOW_DUMP__;
    }
}

void Storage::init()
{
    readFromEeprom();
    if (!isMagicValid()) {
        setAllAsDefault();
        writeToEeprom();
    }
}

void Storage::clear()
{
    setAllAsDefault();
    _modified = true;
}

void Storage::flush()
{
    if (_modified) {
        writeToEeprom();
        _modified = false;
    }
}

bool Storage::setVariable(const String &name, const String &value)
{
    for (unsigned i = 0; i < itemCount; i++) {
        item_t *item = (item_t *)&itemArray[i];
        if (name == (String)item->name) {
            if (item->setValue(value)) {
                _modified = true;
                return true;
            }
            else return false;
        }
    }
    return false;
}

int Storage::getIntByName(const String &name)
{
    for (unsigned i = 0; i < itemCount; i++) {
        const item_t *item = &itemArray[i];
        if (name == (String)item->name) {
            return item->toInt();
        }
    }
    return 0;
}

float Storage::getFloatByName(const String &name)
{
    for (unsigned i = 0; i < itemCount; i++) {
        const item_t *item = &itemArray[i];
        if (name == (String)item->name) {
            return item->toFloat();
        }
    }
    return 0;
}

String Storage::getStrByName(const String &name)
{
    for (unsigned i = 0; i < itemCount; i++) {
        const item_t *item = &itemArray[i];
        if (name == (String)item->name) {
            return item->toStr();
        }
    }
    return "";
}

uint8_t *Storage::getMacByName(const String &name)
{
#ifdef EXT_ITEMS
    for (unsigned i = 0; i < itemCount; i++) {
        const item_t *item = &itemArray[i];
        if (name == (String)item->name) {
            return item->toMac();
        }
    }
#endif
    return nullptr;
}

String Storage::allVariables()
{
    String list;
    for (unsigned i = 0; i < itemCount; i++) {
        const item_t *item = &itemArray[i];
        list += item->name;
        list += "=";
        list += item->toStr();
        list += " (";
        if(item->type == ITEM_INTEGER) {
            list += "number";
        }
        else if(item->type == ITEM_FLOAT) {
            list += "float";
        }
        else if(item->type == ITEM_STRING) {
            String tmp = String("string[") + String(item->len) + String("]");
            list += tmp;
        }
        else if(item->type == ITEM_BOOLEAN) {
            list += "true/false";
        }
#ifdef EXT_ITEMS
        else if(item->type == ITEM_MACADDR) {
            list += "mac address";
        }
#endif
        else {
            list += "unknown";
        }
        list += ")\n";
    }
    return list;
}
