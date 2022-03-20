#pragma once

#include <functional>
#include <WString.h>
#include "taskbar.h"
#include "statusbar.h"
#include "led.h"

#define MQTT_TASK_TIMER (45 * 1000)

class WiFiClient;
class PubSubClient;

class Mqtt : public TimerTask
{
public:
    Mqtt(StatusBar &sb, Led &led, uint32_t cnt = MQTT_TASK_TIMER);
    virtual ~Mqtt();
    void publishSensor(const char* json);
    void publishInfo(const char* json);
    void setCallbackSensor(std::function<void(uint16_t)> callback);
    void setCallbackInfo(std::function<void(uint16_t)> callback);
    void setCallbackCorrection(std::function<void(float)> callback);

private:
    void perform();
    bool connect();
    void startReconnectTimer();
    bool checkReconnectTimer();
    void subscribeSetsorTimer(uint16_t timer);
    void subscribeInfoTimer(uint16_t timer);
    void subscribeCorrection(float corr);
    void discoveryVoltage();
    void discoveryCurrent();
    void discoveryPower();
    void discoveryEnergy();
    void discoveryFrequency();
    void discoveryPF();
    void discoveryVersion();
    void discoveryIpAddr();
    void discoverySSID();
    void discoveryRRSI();
    void discoveryHeapFree();
    void discoveryUptime();

    StatusBar &_statusBar;
    Led _led;

    const String _addr;
    const uint16_t _port;
    const String _user;
    const String _pass;

    std::function<void(uint16_t)> _callbackSensor = nullptr;
    std::function<void(uint16_t)> _callbackInfo = nullptr;
    std::function<void(float)> _callbackCorrection = nullptr;

    WiFiClient *_wifi;
    PubSubClient *_client;

};
