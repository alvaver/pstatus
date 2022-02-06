#include <Arduino.h>
#include "http.h"
#if defined(ESP32)
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "update.h"
#include "storage.h"

namespace
{

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>power statas</title>
<meta name="viewport"content="width=device-width,initial-scale=1">
<style>html{font-family:Arial;display:inline-block;text-align:center;}
p{font-size:1.0rem;}body{margin:0;}a.btn{background-color:#5088b8;color:white;padding:5px 20px;text-decoration:none;}
.topnav{overflow:hidden;background-color:#5088b8;color:white;font-size:1rem;}.content{padding:20px;}
.card{background-color:white;color:#5088b8;box-shadow:2px 2px 12px 1px rgba(140,140,140,.5);}
.cards{max-width:800px;margin:0 auto;display:grid;grid-gap:2rem;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));}
.reading{font-size:1.4rem;}</style></head><body>
<div class="topnav"><h1>POWER STATUS</h1></div><div class="content"><div class="cards">
<div class="card"><p>Voltage: <span class="reading"><span id="Voltage"></span></span> V</p>
<p>Current: <span class="reading"><span id="Current"></span></span> A</p></div>
<div class="card"><p>Power: <span class="reading"><span id="Power"></span></span> W</p>
<p>Energy: <span class="reading"><span id="Energy"></span></span> kWh</p></div>
<div class="card"><p>Frequency: <span class="reading"><span id="Frequency"></span></span> Hz</p>
<p>PF: <span class="reading"><span id="PF"></span></span></p></div>
<div class="card"><p>SSID: <span class="reading"><span id="SSID"></span></span></p>
<p>RSSI: <span class="reading"><span id="RSSI"></span></span></p></div>
<div class="card"><p>Uptime: <span class="reading"><span id="Uptime"></span></span></p>
<p><a href="/update" class="btn">Version: <span id="Version"></span></a></p></div>
</div></div></div><script>
if(!!window.EventSource){
var source = new EventSource('/events');
source.addEventListener('sensor',function(e){
var sensor=JSON.parse(e.data);
document.getElementById("Voltage").innerHTML=sensor.Voltage;
document.getElementById("Current").innerHTML=sensor.Current;
document.getElementById("Power").innerHTML=sensor.Power;
document.getElementById("Energy").innerHTML=sensor.Energy;
document.getElementById("Frequency").innerHTML=sensor.Frequency;
document.getElementById("PF").innerHTML=sensor.PF;
},false);
source.addEventListener('info',function(e){
var info=JSON.parse(e.data);
document.getElementById("Version").innerHTML=info.Version;
document.getElementById("SSID").innerHTML=info.SSID;
document.getElementById("RSSI").innerHTML=info.RSSI;
document.getElementById("Uptime").innerHTML=info.Uptime;
},false);
}</script></body></html>
)rawliteral";

    AsyncWebServer server(80);
    AsyncEventSource events("/events");

}


Http::Http(StatusBar &sb, uint32_t cnt) :
    TimerTask(cnt),
    _statusBar(sb)
{
    _update = new WebUpdate(&server);

    startTaskTimer();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", index_html);
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });

    events.onConnect([&](AsyncEventSourceClient *client) {
        if(_onConnectedClient && client->lastId() == 0) {
            _onConnectedClient(client);
            Serial.printf("[HTTP] Client connected\n");
        }
    });
}

Http::~Http()
{
    delete _update;
}

bool Http::init()
{
    server.addHandler(&events);
    server.begin();
    _update->begin(storage.getStrByName(HTTP_USER).c_str(),
                   storage.getStrByName(HTTP_PASS).c_str());

    Serial.println("[HTTP] server started");
    return true;
}

void Http::setOnConnectedClient(std::function<void(AsyncEventSourceClient *)> callback)
{
    _onConnectedClient = callback;
}

void Http::sendSensor(AsyncEventSourceClient *client, const char* event, const char* json)
{
    if(_connected) {
        client->send(json, event, millis(), 10000);
    }
}

void Http::eventSensor(const char* json)
{
    if(_connected) {
        events.send(json, EVENT_SENSOR, millis());
    }
}

void Http::sendInfo(AsyncEventSourceClient *client, const char* event, const char* json)
{
    if(_connected) {
        client->send(json, event, millis(), 10000);
    }
}

void Http::eventInfo(const char* json)
{
    if(_connected) {
        events.send(json, EVENT_INFO, millis());
    }
}

void Http::eventProgress(unsigned percent)
{
    if(_connected) {
        if ((percent == 0) || (percent > (_last_percent + 8)))
        {
            char buff[8] = { 0 };
            sprintf(buff, "%u%%", percent);
            events.send(buff, EVENT_PROGRESS, millis());
            _last_percent = percent;
        }
    }
}

void Http::reboot()
{
    Serial.println("[HTTP] Rebooting...");
    yield();
    delay(1000);
    yield();
    ESP.restart();
}

void Http::perform()
{
    if(_statusBar.isWifiConnected()) {
        if(! _connected) {
            if (checkTaskTimer()) {
                _connected = init();
                restartTaskTimer();
            }
        }
        else {
            if(_update->isNeedReboot())
                reboot();

            if(_update->isNeedProgress())
                eventProgress(_update->getPercentProgress());
            else
                _last_percent = 0;

        }
    }
}
