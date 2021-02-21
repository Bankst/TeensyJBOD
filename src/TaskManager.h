#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

typedef void (*callback_function)(void);


class TaskManager
{
public:

    class TimedTask
    {
    private:
        const std::function<void()> task_function;
        const uint32_t task_period_ms;
        int32_t last_execute_ms = 0 - millis(); // enable instant execution upon scheduling

    public:
        TimedTask(std::function<void()> task_function, uint32_t task_period_ms) : task_function(task_function), task_period_ms(task_period_ms){};

        void execute()
        {
            if (millis() - last_execute_ms >= task_period_ms)
            {
                task_function();
                last_execute_ms = millis();
            }
        }
    };

public:
    TaskManager(){};

    void service();
    void add_task(TimedTask new_task);

private:
    std::vector<TimedTask> timed_tasks;
};

void TaskManager::service()
{
    for (auto & task : timed_tasks)
    {
        task.execute();
    }
}

void TaskManager::add_task(TimedTask new_task)
{
    timed_tasks.push_back(new_task);
}