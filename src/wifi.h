#pragma once

#include "taskbar.h"
#include "statusbar.h"
#include "led.h"

#define WIFI_TASK_TIMER (60 * 1000)

class wifiCredential
{
public:
    wifiCredential(const String &ssid,
                   const String &pass) :
                   _ssid(ssid),
                   _pass(pass)
                   {
                   }
    const char *getSsid() { return _ssid.c_str(); }
    const char *getPass() { return _pass.c_str(); }
    bool empty() { return _ssid.isEmpty(); }
private:
    const String _ssid;
    const String _pass;
};


class Wifi : public TimerTask
{
public:
    Wifi(StatusBar &sb, Led &led, uint32_t cnt = WIFI_TASK_TIMER);
    virtual ~Wifi();
private:
    void perform();
    bool isConnected();
    bool connect(const char* ssid, const char* pass);
    wifiCredential *nextCredential();

    StatusBar &_statusBar;
    Led _led;

    const wifiCredential _cred1;
    const wifiCredential _cred2;
    const wifiCredential _cred3;
    wifiCredential *_curCred = nullptr;

};
