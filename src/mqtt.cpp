#include "mqtt.h"
#include "storage.h"
#include <PubSubClient.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#include <esp_wifi.h>
#endif

#define TOPIC_SENSOR "sensor"
#define TOPIC_INFO "info"
#define TOPIC_STATUS "status"
#define TOPIC_TIMER "timer"
#define TOPIC_CORR "correction"
#define STATE_ONLINE "Online"
#define STATE_OFFLINE "Offline"
#define LEN_BUFF 40

namespace
{
    char tSensorSensor[LEN_BUFF] = { 0 };
    char tSensorTimer[LEN_BUFF] = { 0 };
    char tSensorCorr[LEN_BUFF] = { 0 };
    char tInfoStatus[LEN_BUFF] = { 0 };
    char tInfoInfo[LEN_BUFF] = { 0 };
    char tInfoTimer[LEN_BUFF] = { 0 };
    char devId[16] = { 0 };

    void initTopicName()
    {
        uint8_t mac[6];
#ifdef ESP8266
        wifi_get_macaddr(STATION_IF, mac);
#else
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
#endif
        sprintf(devId, "pstatus_%02X%02X%02X", mac[3], mac[4], mac[5]);
        sprintf(tSensorSensor, "%s/%s/%s", TOPIC_SENSOR, devId, TOPIC_SENSOR);
        sprintf(tSensorTimer, "%s/%s/%s", TOPIC_SENSOR, devId, TOPIC_TIMER);
        sprintf(tSensorCorr, "%s/%s/%s", TOPIC_SENSOR, devId, TOPIC_CORR);
        sprintf(tInfoStatus, "%s/%s/%s", TOPIC_INFO, devId, TOPIC_STATUS);
        sprintf(tInfoInfo, "%s/%s/%s", TOPIC_INFO, devId, TOPIC_INFO);
        sprintf(tInfoTimer, "%s/%s/%s", TOPIC_INFO, devId, TOPIC_TIMER);
    }

}


Mqtt::Mqtt(StatusBar &sb, Led &led, uint32_t cnt) :
    TimerTask(cnt),
    _statusBar(sb),
    _led(led),
    _addr(storage.getStrByName(MQTT_ADDR)),
    _port(storage.getIntByName(MQTT_PORT)),
    _user(storage.getStrByName(MQTT_USER)),
    _pass(storage.getStrByName(MQTT_PASS))
{
    initTopicName();
    _wifi = new WiFiClient;
    _client = new PubSubClient(*_wifi);
    _client->setServer(_addr.c_str(), _port);
    _client->setCallback([&](char* topic, byte* payload, unsigned int length) {
        char* buff = (char *)malloc(length + 1);
        memcpy(buff, payload, length);
        buff[length] = 0;

        if(strcmp(topic, tSensorTimer) == 0) {
            uint16_t timer = atoi(buff);
            if(_callbackSensor) {
                _callbackSensor(timer);
            }
        }
        else if(strcmp(topic, tInfoTimer) == 0) {
            uint16_t timer = atoi(buff);
            if(_callbackInfo) {
                _callbackInfo(timer);
            }
        }
        if(strcmp(topic, tSensorCorr) == 0) {
            float corr = atof(buff);
            if(_callbackCorrection) {
                _callbackCorrection(corr);
            }
        }
        free(buff);
    });

    startTaskTimer();
}

Mqtt::~Mqtt()
{
    delete _client;
    delete _wifi;
}

bool Mqtt::connect()
{
    bool res = false;

    _led.ledOn();
    if (_client->connect(devId, _user.c_str(), _pass.c_str(),
                          tInfoStatus, 0, 0, STATE_OFFLINE)) {

        _client->publish(tInfoStatus, STATE_ONLINE);

        subscribeSetsorTimer(storage.getIntByName(TIK_SENSOR));
        subscribeInfoTimer(storage.getIntByName(TIK_INFO));
        subscribeCorrection(storage.getFloatByName(COR_SENSOR));

        discoveryVoltage();
        discoveryCurrent();
        discoveryPower();
        discoveryEnergy();
        discoveryFrequency();
        discoveryPF();

        discoveryVersion();
        discoveryIpAddr();
        discoverySSID();
        discoveryRSSI();
        discoveryHeapFree();
        discoveryUptime();

        Serial.printf("[MQTT] Connection to '%s:%d' user='%s' OK\n",
                        _addr.c_str(), _port, _user.c_str());
        res = true;
    }
    else {
        Serial.printf("[MQTT] Connection to '%s' failed\n", _addr.c_str());
    }
    _led.ledOff();

    return res;
}

void Mqtt::discoveryVoltage()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Voltage/config", devId);
    sprintf(config, "{\"device_class\":\"voltage\",\"name\":\"Voltage\","
    "\"unit_of_measurement\":\"V\",\"icon\":\"mdi:speedometer\",\"state_topic\":\"%s\","
    "\"value_template\":\"{{value_json.Voltage}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryCurrent()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Current/config", devId);
    sprintf(config, "{\"device_class\":\"current\",\"name\":\"Current\","
    "\"unit_of_measurement\":\"A\",\"icon\":\"mdi:speedometer\",\"state_topic\":\"%s\","
    "\"value_template\":\"{{value_json.Current}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryPower()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Power/config", devId);
    sprintf(config, "{\"device_class\":\"power\",\"name\":\"Power\","
    "\"unit_of_measurement\":\"W\",\"icon\":\"mdi:speedometer\",\"state_topic\":\"%s\","
    "\"value_template\":\"{{value_json.Power}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryEnergy()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Energy/config", devId);
    sprintf(config, "{\"device_class\":\"energy\",\"name\":\"Energy\","
    "\"unit_of_measurement\":\"kWh\",\"icon\":\"mdi:counter\",\"state_topic\":\"%s\","
    "\"value_template\":\"{{value_json.Energy}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryFrequency()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Frequency/config", devId);
    sprintf(config, "{\"device_class\":\"frequency\",\"name\":\"Frequency\","
    "\"unit_of_measurement\":\"Hz\",\"icon\":\"mdi:lightning-bolt\",\"state_topic\":\"%s\","
    "\"value_template\":\"{{value_json.Frequency}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryPF()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/PF/config", devId);
    sprintf(config, "{\"device_class\":\"power_factor\",\"name\":\"PF\",\"icon\":\"mdi:lightning-bolt\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.PF}}\"}", tSensorSensor);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryVersion()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Version/config", devId);
    sprintf(config, "{\"name\":\"Version\",\"icon\":\"mdi:information-outline\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.Version}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryIpAddr()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/IpAddr/config", devId);
    sprintf(config, "{\"name\":\"IpAddr\",\"icon\":\"mdi:wifi\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.IpAddr}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::discoverySSID()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/SSID/config", devId);
    sprintf(config, "{\"name\":\"SSID\",\"icon\":\"mdi:wifi\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.SSID}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryRSSI()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/RSSI/config", devId);
    sprintf(config, "{\"name\":\"RSSI\",\"icon\":\"mdi:access-point-network\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.RSSI}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryHeapFree()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/HeapFree/config", devId);
    sprintf(config, "{\"name\":\"HeapFree\",\"icon\":\"mdi:memory\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.HeapFree}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::discoveryUptime()
{
    char topic[100] = { 0 };
    char config[200] = { 0 };
    sprintf(topic, "homeassistant/sensor/%s/Uptime/config", devId);
    sprintf(config, "{\"name\":\"Uptime\",\"icon\":\"mdi:clock-time-four-outline\","
    "\"state_topic\":\"%s\",\"value_template\":\"{{value_json.Uptime}}\"}", tInfoInfo);
    _client->publish(topic, config, true);
}

void Mqtt::subscribeSetsorTimer(uint16_t timer)
{
    _client->publish(tSensorTimer, String(timer, DEC).c_str());
    _client->subscribe(tSensorTimer);
}

void Mqtt::subscribeInfoTimer(uint16_t timer)
{
    _client->publish(tInfoTimer, String(timer, DEC).c_str());
    _client->subscribe(tInfoTimer);
}

void Mqtt::subscribeCorrection(float corr)
{
    _client->publish(tSensorCorr, String(corr, 1).c_str());
    _client->subscribe(tSensorCorr);
}

void Mqtt::setCallbackSensor(std::function<void(uint16_t)> callback)
{
    _callbackSensor = callback;
}

void Mqtt::setCallbackInfo(std::function<void(uint16_t)> callback)
{
    _callbackInfo = callback;
}

void Mqtt::setCallbackCorrection(std::function<void(float)> callback)
{
    _callbackCorrection = callback;
}

void Mqtt::publishSensor(const char* json)
{
    if(_statusBar.isMqttConnected()) {
        _led.ledOn();
        _client->publish(tSensorSensor, json);
        _led.ledOff();
    }
}

void Mqtt::publishInfo(const char* json)
{
    if(_statusBar.isMqttConnected()) {
        _led.ledOn();
        _client->publish(tInfoInfo, json);
        _led.ledOff();
    }
}

void Mqtt::perform()
{
    if(_statusBar.isWifiConnected()) {
        if(! _statusBar.isMqttConnected()) {
            if (checkTaskTimer()) {
                if (connect()) {
                    _statusBar.mqttConnected(true);
                }
                restartTaskTimer();
            }
        }
        else {
            if(!_client->loop()) {
                _statusBar.mqttConnected(false);
            }
        }
    }
}
