#pragma once

#include <Arduino.h>
#include "taskbar.h"
#include "statusbar.h"

#define _MAX_COMMAND_LEN 128

class Terminal : public Task
{
public:
    Terminal();
    virtual ~Terminal();
private:
    void usage();
    void printInvite();
    void printListVariables();
    void printStoreVariable(const char *name);
    void setStoreVariable(const char *name, const char *value);
    void parseCommand();
    void perform();

    uint8_t _cnt;
    char _command[_MAX_COMMAND_LEN];
};
