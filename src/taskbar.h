#pragma once

#include <vector>
#include <stdint.h>

class Task
{
public:
    virtual ~Task() {}
    virtual void perform() = 0;
};

class TimerTask : public Task
{
public:
    TimerTask(uint32_t cnt);
    virtual ~TimerTask() {}
protected:
    void startTaskTimer();
    void restartTaskTimer();
    bool checkTaskTimer();
    uint32_t _beforeTaskTimer;
private:
    const uint32_t _timeTaskTimer;
};

class TaskBar
{
public:
    TaskBar();
    ~TaskBar();
    void add(Task *task);
    void dutyCycle();

private:
    std::vector<Task *> _tasklist;

};