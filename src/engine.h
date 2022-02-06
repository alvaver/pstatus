#pragma once

#include "taskbar.h"
#include "statusbar.h"
#include "pzem.h"
#include "mqtt.h"
#include "http.h"

#define MOVER_TASK_TIMER (5 * 1000)

class AsyncEventSourceClient;

class Engine : public Task
{
public:
    Engine(StatusBar &sb, Pzem &pzem, Mqtt &mqtt, Http &http);
    virtual ~Engine();

private:
    void perform();

    String UptimeToStr();
    String jsonSensor();
    String jsonInfo();
    void mqttPublishSensor(const String &json);
    void mqttPublishInfo(const String &json);
    void httpEventSensor(const String &json);
    void httpEventInfo(const String &json);
    void httpSendSensor(AsyncEventSourceClient *client, const String &json);
    void httpSendInfo(AsyncEventSourceClient *client, const String &json);

    void startSensorTimer();
    bool checkSensorTimer();
    void startInfoTimer();
    bool checkInfoTimer();

    StatusBar &_statusBar;
    Pzem &_pzem;
    Mqtt &_mqtt;
    Http &_http;
    uint32_t _sensorCount;
    uint32_t _infoCount;
    int16_t _sensorTimer;
    int16_t _infoTimer;
    float _energyCorr = 0;

};


