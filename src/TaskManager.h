#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

class TaskManager
{
public:
    class TimedTask
    {
    private:
        const uint32_t task_period_ms;
        const std::function<void()> task_function;
        uint32_t last_execute_ms = 0;

    public:
        TimedTask(uint32_t task_period_ms, std::function<void()> task_function) : task_period_ms(task_period_ms), task_function(task_function){};

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