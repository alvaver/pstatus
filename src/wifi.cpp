#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include "wifi.h"
#include "ntp.h"
#include "storage.h"

#define COUNT_RECONNECT 3

Wifi::Wifi(StatusBar &sb, Led &led, uint32_t cnt) :
    TimerTask(cnt),
    _statusBar(sb),
    _led(led),
    _cred1(wifiCredential(storage.getStrByName(WIFI_SSID_1),
                           storage.getStrByName(WIFI_PASS_1))),
    _cred2(wifiCredential(storage.getStrByName(WIFI_SSID_2),
                           storage.getStrByName(WIFI_PASS_2))),
    _cred3(wifiCredential(storage.getStrByName(WIFI_SSID_3),
                           storage.getStrByName(WIFI_PASS_3)))
{
    startTaskTimer();
}

Wifi::~Wifi()
{
}

bool Wifi::isConnected()
{
    return (WL_CONNECTED == WiFi.status());
}

bool Wifi::connect(const char* ssid, const char* pass)
{
    Serial.printf("[WIFI] Connect to '%s'\n", ssid);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    for (uint8_t n = 0; n < 60; n++) {
        if (isConnected()) {
            return true;
        }
        _led.ledOn();
        delay(1);
        _led.ledOff();
        delay(250);
    }
    return false;
}

wifiCredential *Wifi::nextCredential()
{
    if (_curCred == &_cred1)
        return (wifiCredential *)&_cred2;
    else if (_curCred == &_cred2)
        return (wifiCredential *)&_cred3;
    else 
        return (wifiCredential *)&_cred1;
}

void Wifi::perform()
{
    bool connected = isConnected();

    if (!connected) {
        _statusBar.mqttConnected(false);
        _curCred = nullptr;
    }

    if (!connected && checkTaskTimer())
    {
        if (_curCred != nullptr && !_curCred->empty()) {
            connected = connect(_curCred->getSsid(), _curCred->getPass());
            _curCred = nextCredential();
        }
        else {
            for (uint8_t i = 0; i < COUNT_RECONNECT; i++) {
                _curCred = nextCredential();
                if(_curCred != nullptr && !_curCred->empty()) {
                    connected = connect(_curCred->getSsid(), _curCred->getPass());
                    if (connected)
                        break;
                }
            }

            if (!connected) {
                Serial.printf("[WIFI] All connection failed\n");
                restartTaskTimer();
            }
        }
    }

    if (connected &&  !_statusBar.isWifiConnected()) {
        Serial.printf("[WIFI] IP address '%s'\n", WiFi.localIP().toString().c_str());
        Serial.printf("[WIFI] Run NTP sync\n");
        ntpSync();
    }

    _statusBar.wifiConnected(connected);
}
