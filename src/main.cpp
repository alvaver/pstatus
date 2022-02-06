#include <Arduino.h>
#include "taskbar.h"
#include "statusbar.h"
#include "terminal.h"
#include "wifi.h"
#include "mqtt.h"
#include "http.h"
#include "pzem.h"
#include "led.h"
#include "storage.h"
#include "engine.h"

TaskBar *taskBar;
StatusBar *statusBar;
Terminal *terminal;
Engine *engine;
Wifi *wifi;
Mqtt *mqtt;
Http *http;
Pzem *pzem;
Led statLed(STAT_LED_PIN);

void setup()
{
    Serial.begin(115200);
    statLed.init();
    storage.init();

    taskBar = new TaskBar;
    statusBar = new StatusBar;

    pzem = new Pzem();
    taskBar->add(pzem);

    wifi = new Wifi(*statusBar, statLed);
    taskBar->add(wifi);

    mqtt = new Mqtt(*statusBar, statLed);
    taskBar->add(mqtt);

    http = new Http(*statusBar);
    taskBar->add(http);

    engine = new Engine(*statusBar, *pzem, *mqtt, *http);
    taskBar->add(engine);

    terminal = new Terminal;
    taskBar->add(terminal);
}

void loop()
{
    taskBar->dutyCycle();

    // Power saving delay
    delay(10);
}