#pragma once

#include <WString.h>

class AsyncWebServer;

class WebUpdate
{
public:
    WebUpdate(AsyncWebServer *server);
    ~WebUpdate();
    void begin(const char* username = "", const char* password = "");
    bool isNeedReboot();
    bool isNeedProgress();
    unsigned getPercentProgress();

private:
    String getID();

    AsyncWebServer *_server;
    const String _id;
    String _username;
    String _password;
    bool _authRequired = false;
    bool _needReboot = false;
    bool _needProgress = false;
    unsigned _progress = 0;

};
