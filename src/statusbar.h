#pragma once

class StatusBar
{
public:
    StatusBar() {
        _wifiConnected = false;
        _mqttConnected = false;
    }

    bool isWifiConnected() { return _wifiConnected; }
    bool isMqttConnected() { return _mqttConnected; }
    void wifiConnected(bool stat) { _wifiConnected = stat; }
    void mqttConnected(bool stat) { _mqttConnected = stat; }

private:
    bool _wifiConnected;
    bool _mqttConnected;
};
