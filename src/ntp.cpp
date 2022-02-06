#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "TimeLib.h"
#include <WiFiUdp.h>
#include <Udp.h>
#include "ntp.h"
#include "storage.h"


namespace
{
    #define SEVENTY_YEARS 2208988800UL
    #define DEFAULT_NTP_PORT 123
    #define NTP_PACKET_SIZE 48

    byte ntpBuffer[NTP_PACKET_SIZE];
    WiFiUDP udpNtp;

    int ntpTimeZone = +2;

    void ntpSendRequest(IPAddress &address)
    {
        memset(ntpBuffer, 0, NTP_PACKET_SIZE);
        ntpBuffer[0] = 0b11100011;
        ntpBuffer[1] = 0;
        ntpBuffer[2] = 6;
        ntpBuffer[3] = 0xEC;
        ntpBuffer[12] = 49;
        ntpBuffer[13] = 0x4E;
        ntpBuffer[14] = 49;
        ntpBuffer[15] = 52;
        udpNtp.beginPacket(address, DEFAULT_NTP_PORT);
        udpNtp.write(ntpBuffer, NTP_PACKET_SIZE);
        udpNtp.endPacket();
    }

    time_t ntpSyncTimeServer(const String &ntpServerName)
    {
        IPAddress ntpServerIP;

        udpNtp.flush();
        WiFi.hostByName(ntpServerName.c_str(), ntpServerIP);

        Serial.printf("[NTP] Transmit request to '%s'\n", ntpServerName.c_str());
        ntpSendRequest(ntpServerIP);

        uint32_t beginWait = millis();
        while (millis() - beginWait < 1500) {
            int size = udpNtp.parsePacket();
            if (size >= NTP_PACKET_SIZE) {
                udpNtp.read(ntpBuffer, NTP_PACKET_SIZE);
                udpNtp.flush(); 

                unsigned long secsSince1900;
                secsSince1900 =  (unsigned long)ntpBuffer[40] << 24;
                secsSince1900 |= (unsigned long)ntpBuffer[41] << 16;
                secsSince1900 |= (unsigned long)ntpBuffer[42] << 8;
                secsSince1900 |= (unsigned long)ntpBuffer[43];
                return secsSince1900 - SEVENTY_YEARS + ntpTimeZone * SECS_PER_HOUR;
            }
        }

        Serial.printf("[NTP] no response\n");
        return 0; 
    }

    time_t ntpSyncTime()
    {
        time_t res;
        udpNtp.begin(8888);
        ntpTimeZone = storage.getIntByName(NTP_ZONE);

        res = ntpSyncTimeServer(storage.getStrByName(NTP_ADDR_1));
        if (res)
            return res;

        res = ntpSyncTimeServer(storage.getStrByName(NTP_ADDR_2));
        if (res)
            return res;

        res = ntpSyncTimeServer(storage.getStrByName(NTP_ADDR_3));

        return res;
    }
}

void ntpSync()
{
    setSyncProvider(ntpSyncTime);
    setSyncInterval(4 * 60 * 60);
}
