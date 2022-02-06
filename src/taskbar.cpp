#include <Arduino.h>
#include "taskbar.h"


TimerTask::TimerTask(uint32_t cnt)
    : _timeTaskTimer(cnt)
{
}

void TimerTask::startTaskTimer()
{
    _beforeTaskTimer = millis();
}

void TimerTask::restartTaskTimer()
{
    _beforeTaskTimer = millis() + _timeTaskTimer;
}

bool TimerTask::checkTaskTimer()
{
    return (millis() > _beforeTaskTimer);
}


TaskBar::TaskBar()
{
}

TaskBar::~TaskBar()
{
    for (unsigned char i = 0; i < _tasklist.size(); i++) {
        delete(_tasklist[i]);
    }
}

void TaskBar::add(Task *task)
{
    _tasklist.push_back(task);
}

void TaskBar::dutyCycle()
{
    for (unsigned char i = 0; i < _tasklist.size(); i++) {
        _tasklist[i]->perform();
    }
}