#include "update.h"
#include "ESPAsyncWebServer.h"
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "Update.h"
#endif

namespace
{

const uint8_t update_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>power statas</title>
<meta name="viewport"content="width=device-width,initial-scale=1">
<style>html{font-family:Arial;display:inline-block;text-align:center;}p{font-size:1.2rem;}body{margin:0;}
.topnav{overflow:hidden;background-color:#5088b8;color:white;font-size:1rem;}.content{padding:20px;align-content:center;}
.card{max-width:300px;padding:5px 20px;margin: auto;background-color:white;color:#5088b8;box-shadow:2px 2px 12px 1px rgba(140,140,140,.5);}
</style></head><body><div class="topnav"><h1>UPDATE <span id="Percent"></span></h1></div><div class="content"><div class="card">
<form id="form" method="POST" enctype="multipart/form-data"><p><input type="file" accept=".bin" name="firmware">
</p><p><input type="submit" value="Update"></p></form></div></div><script>
if(!!window.EventSource){var source = new EventSource('/events');
source.addEventListener('progress',function(e){document.getElementById("Percent").innerHTML=e.data;},false);}
</script></body></html>
)rawliteral";

}

WebUpdate::WebUpdate(AsyncWebServer *server) :
        _server(server),
        _id(getID())
{
}

WebUpdate::~WebUpdate()
{
}

String WebUpdate::getID()
{
    String id;
#if defined(ESP8266)
    id = String(ESP.getChipId());
#elif defined(ESP32)
    id = String((uint32_t)ESP.getEfuseMac(), HEX);
#endif
    id.toUpperCase();
    return id;
}

void WebUpdate::begin(const char* username, const char* password)
{
    if(strlen(username) > 0) {
        _authRequired = true;
        _username = username;
        _password = password;
    }

    _server->on("/update", HTTP_GET, [&](AsyncWebServerRequest *request) {
        _needProgress = false;
        if(_authRequired){
            if(!request->authenticate(_username.c_str(), _password.c_str())) {
                return request->requestAuthentication();
            }
        }
        request->send(200, "text/html", (char *)update_html);
    });

    _server->on("/update", HTTP_POST, [&](AsyncWebServerRequest *request) {
        _needProgress = false;
        if(_authRequired){
            if(!request->authenticate(_username.c_str(), _password.c_str())) {
                return request->requestAuthentication();
            }
        }
        if (!Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
            "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...");
            response->addHeader("Connection", "close");
            request->send(response);
        }
    }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if(_authRequired){
            if(!request->authenticate(_username.c_str(), _password.c_str())){
                return request->requestAuthentication();
            }
        }
        if (!index) {
            _progress = 0;
            _needProgress = true;
            
            Serial.printf("[UPD] Update Start: %s\n", filename.c_str());
            #if defined(ESP8266)
            Update.runAsync(true);
            if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
                Update.printError(Serial);
            }
            #elif defined(ESP32)
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
            #endif
        }
        if(len) {
            if (Update.write(data, len) != len) {
                return request->send(400, "text/plain", "Could not write chunk");
            }
        }
        if (final) {
            _needProgress = false;
            if(Update.end(true)) {
                Serial.printf("[UPD] Update Success: %uB\n", index + len);
                _needReboot = true;
            } else {
                return request->send(400, "text/plain", "Could not fin OTA");
            }
        }
    });

    Update.onProgress([&](size_t len, size_t total) {
        _progress = (len * 100 / total);
    });

}

bool WebUpdate::isNeedReboot()
{
    return _needReboot;
}

bool WebUpdate::isNeedProgress()
{
    return _needProgress;
}

unsigned WebUpdate::getPercentProgress()
{
    return _progress;
}


