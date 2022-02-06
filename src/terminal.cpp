#include "terminal.h"
#include "TimeLib.h"
#include "storage.h"

#define CMD_DATE    "date"
#define CMD_LIST    "list"
#define CMD_CLEAR   "clear"
#define CMD_SAVE    "save"
#define CMD_GET     "get"
#define CMD_SET     "set"
#define CMD_REBOOT  "reboot"


Terminal::Terminal()
{
    printInvite();
}

Terminal::~Terminal()
{
}

void Terminal::printInvite()
{
    _cnt = 0;
    memset (_command, 0, _MAX_COMMAND_LEN);
    Serial.print("> ");
}

void Terminal::usage()
{
    Serial.printf("Usage: [COMMAND] [ARGS]\n");
    Serial.printf("    %s : Display the current time\n", CMD_DATE);
    Serial.printf("    %s <name> : Display variable\n", CMD_GET);
    Serial.printf("    %s <name> <value> : Set variable\n", CMD_SET);
    Serial.printf("    %s : Display all variables\n", CMD_LIST);
    Serial.printf("    %s : Clear variables in eeprom\n", CMD_CLEAR);
    Serial.printf("    %s : Store variables to eeprom\n", CMD_SAVE);
    Serial.printf("    %s : Reboot device.\n", CMD_REBOOT);
}

void Terminal::printStoreVariable(const char *name)
{
    String value = storage.getStrByName(name);
    Serial.printf("Variable '%s' is '%s'\n", name, value.c_str());
}

void Terminal::setStoreVariable(const char *name, const char *value)
{
    bool res = storage.setVariable(String(name), String(value));
    Serial.printf("Written '%s'='%s' %s\n", name, value, (res) ? "correctly" : "with an error");
}

void Terminal::printListVariables()
{
    String list = storage.allVariables();
    Serial.printf("List variables:\n=================\n%s", list.c_str());
}

void Terminal::parseCommand()
{
    char separators[] = " ";
    char __attribute__((unused)) *arg1 = nullptr;
    char __attribute__((unused)) *arg2 = nullptr;
    char __attribute__((unused)) *arg3 = nullptr;

    if (_cnt == 0) {
        return;
    }

    arg1 = strtok(_command, separators);
    if(arg1 != nullptr) {
        arg2 = strtok(nullptr, separators);
        if(arg2 != nullptr) {
            arg3 = strtok(nullptr, separators);
        }
    }

    if (strcmp(arg1, CMD_DATE) == 0) {
        time_t t = now();
        Serial.printf("Mow %02d.%02d.%04d %02d:%02d:%02d\n",
        day(t), month(t), year(t), hour(t), minute(t), second(t));
    }
    else if (strcmp(arg1, CMD_REBOOT) == 0) {
        ESP.restart();
        delay(500);
    }
    else if (strcmp(arg1, CMD_LIST) == 0) {
        printListVariables();
    }
    else if (strcmp(arg1, CMD_CLEAR) == 0) {
        storage.clear();
    }
    else if (strcmp(arg1, CMD_SAVE) == 0) {
        storage.flush();
    }
    else if (strcmp(arg1, CMD_GET) == 0) {
        if(arg1 != nullptr) {
            printStoreVariable(arg2);
        }
    }
    else if (strcmp(arg1, CMD_SET) == 0) {
        if(arg2 != nullptr) {
            if(arg3 != nullptr)
                setStoreVariable(arg2, arg3);
            else
                setStoreVariable(arg2, "");
        }
    }
    else {
        usage();
    }
}

void Terminal::perform()
{
    while (Serial.available() > 0) {

        char inChar = Serial.read();

        if(inChar == '\r') {
        }
        else if (inChar == '\n') {
            Serial.println();
            parseCommand();
            printInvite();
        }
        else if ((inChar >= ' ') && (inChar < 0x7f)) {
            Serial.write(inChar);
            _command[_cnt++] = inChar;
        }
    }
}
