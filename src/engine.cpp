#include <Arduino.h>
#include "engine.h"
#include "storage.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#define MILLISEC 1000

Engine::Engine(StatusBar &sb,
        Pzem &pzem, Mqtt &mqtt, Http &http) :
        _statusBar(sb),
        _pzem(pzem),
        _mqtt(mqtt),
        _http(http),
        _sensorTimer(storage.getIntByName(TIK_SENSOR)),
        _infoTimer(storage.getIntByName(TIK_INFO)),
        _energyCorr(storage.getFloatByName(COR_SENSOR))
{
    _mqtt.setCallbackSensor([&](uint16_t timer) {
        if(_sensorTimer != timer && timer != 0) {
            _sensorTimer = timer;
            String value(timer, DEC);
            if(storage.setVariable(TIK_SENSOR, value)) {
                storage.flush();
                printf("[EMG] New value set for sensor update: %u\n", timer);
            }
        }
    });
    _mqtt.setCallbackInfo([&](uint16_t timer) {
        if(_infoTimer != timer && timer != 0) {
            _infoTimer = timer;
            String value(timer, DEC);
            if (storage.setVariable(TIK_INFO, value)) {
                storage.flush();
                printf("[EMG] New value set for info update: %u\n", timer);
            }
        }
    });
    _mqtt.setCallbackCorrection([&](float corr) {
        if(_energyCorr != corr) {
            _energyCorr = corr;
            String value(corr, 1);
            if (storage.setVariable(COR_SENSOR, value)) {
                storage.flush();
                printf("[EMG] New value set for energy correction: %0.1f\n", corr);
            }
        }
    });
    _http.setOnConnectedClient([&](AsyncEventSourceClient *client) {
        String json;
        json = jsonSensor();
        httpSendSensor(client, json);
        json = jsonInfo();
        httpSendInfo(client, json);
    });

    _sensorCount = millis();
    _infoCount = millis();
}

Engine::~Engine()
{
}

String Engine::UptimeToStr()
{
    char buff[20] = { 0 };
    uint32_t time = millis();
    int day, hour, min;
    time /= 60000;
    min = time % 60;
    time /= 60;
    hour = time % 24;
    day = time / 24;
    sprintf(buff, "%d %02d:%02d", day, hour, min);
    return String(buff);
}

String Engine::jsonInfo()
{
    String json = "{\n  \"Version\":\"";
    json += String(VERSION);
    json += "\",\n  \"IpAddr\":\"";
    json += WiFi.localIP().toString();
    json += "\",\n  \"SSID\":\"";
    json += String(WiFi.SSID());
    json += "\",\n  \"RSSI\":";
    json += String(WiFi.RSSI());
    json += ",\n  \"HeapFree\":";
    json += String(ESP.getFreeHeap());
    json += ",\n  \"Uptime\":\"";
    json += UptimeToStr();
    json += "\"\n}";
    return json;
}

String Engine::jsonSensor()
{
    String json = "{\n  \"Voltage\":";
    json += String(_pzem.getVoltage(), 1);
    json += ",\n  \"Current\":";
    json += String(_pzem.getCurrent(), 2);
    json += ",\n  \"Power\":";
    json += String(_pzem.getPower(), 1);
    json += ",\n  \"Energy\":";
    json += String(_pzem.getEnergy() + _energyCorr, 1);
    json += ",\n  \"Frequency\":";
    json += String(_pzem.getFrequency(), 1);
    json += ",\n  \"PF\":";
    json += String(_pzem.getPf(), 2);
    json += "\n}";
    return json;
}

void Engine::httpSendSensor(AsyncEventSourceClient *client, const String &json)
{
    if (_pzem.isCorrect()) {
        _http.sendSensor(client, EVENT_SENSOR, json.c_str());
    }
}

void Engine::httpSendInfo(AsyncEventSourceClient *client, const String &json)
{
    _http.sendInfo(client, EVENT_INFO, json.c_str());
}

void Engine::httpEventSensor(const String &json)
{
    if (_pzem.isCorrect()) {
        _http.eventSensor(json.c_str());
    }
}

void Engine::httpEventInfo(const String &json)
{
    _http.eventInfo(json.c_str());
}

void Engine::mqttPublishSensor(const String &json)
{
    if (_pzem.isCorrect()) {
        _mqtt.publishSensor(json.c_str());
    }
}

void Engine::mqttPublishInfo(const String &json)
{
    _mqtt.publishInfo(json.c_str());
}

void Engine::startInfoTimer()
{
    _infoCount = millis() + (uint32_t)_infoTimer * MILLISEC;
}

bool Engine::checkInfoTimer()
{
    return (millis() > _infoCount);
}

void Engine::startSensorTimer()
{
    _sensorCount = millis() + (uint32_t)_sensorTimer * MILLISEC;
}

bool Engine::checkSensorTimer()
{
    return (millis() > _sensorCount);
}

void Engine::perform()
{
    if (checkSensorTimer()) {
        String json = jsonSensor();
        mqttPublishSensor(json);
        httpEventSensor(json);
        startSensorTimer();
    }

    if (checkInfoTimer()) {
        String json = jsonInfo();
        mqttPublishInfo(json);
        httpEventInfo(json);
        startInfoTimer();
    }

}
