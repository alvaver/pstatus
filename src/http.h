#pragma once

#include <functional>
#include "taskbar.h"
#include "statusbar.h"

#define HTTP_TASK_TIMER     (1 * 1000)
#define EVENT_SENSOR "sensor"
#define EVENT_INFO "info"
#define EVENT_PROGRESS "progress"


class AsyncEventSourceClient;
class WebUpdate;

class Http : public TimerTask
{
public:
    Http(StatusBar &sb, uint32_t cnt = HTTP_TASK_TIMER);
    virtual ~Http();
    void setOnConnectedClient(std::function<void(AsyncEventSourceClient *)> callback);
    void sendSensor(AsyncEventSourceClient *client, const char* event, const char* json);
    void sendInfo(AsyncEventSourceClient *client, const char* event, const char* json);
    void eventSensor(const char* json);
    void eventInfo(const char* json);
    void eventProgress(unsigned percent);

private:
    void perform();
    bool init();
    void reboot();

    WebUpdate *_update;
    StatusBar &_statusBar;
    std::function<void(AsyncEventSourceClient *)> _onConnectedClient = nullptr;
    bool _connected = false;
    unsigned _last_percent = 0;

};


