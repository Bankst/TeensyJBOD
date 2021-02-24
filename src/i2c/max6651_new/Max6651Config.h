#pragma once

#include "Max6651Constants.h"

struct Max6651Config
{
    Max6651Config();

    Max6651Config &voltage(ConfigFanVoltage v);
    Max6651Config &preScaler(ConfigPrescaler p);
    Max6651Config &clockFrequency(float hertz);
    Max6651Config &tachometerCountTime(TachometerCountTime time);
    
    Max6651Config &gpioAsOutput(uint8_t index, bool high);
    Max6651Config &gpioAsInput(uint8_t index);
    Max6651Config &gpio2AsClockOutput();
    Max6651Config &gpio2AsClockInput();
    Max6651Config &gpio1AsFullOnInput();
    Max6651Config &gpio0AsAlertOutput();
    
    Max6651Config &enableAlarms(AlarmEnable a);
    Max6651Config &maxAlarm(bool enable = true);
    Max6651Config &minAlarm(bool enable = true);
    Max6651Config &tachometerAlarm(bool enable = true);
    Max6651Config &gpio1Alarm(bool state = true);
    Max6651Config &gpio2Alarm(bool state = true);

    uint8_t mode_config_;
    uint8_t alarm_config_;
    uint8_t gpio_config_;
    uint8_t tachometer_count_time_;
    uint8_t pulses_per_revolution_;
    float f_clk_;
};