// #pragma once

// #include <stdint.h>
// #include <Arduino.h>

// #include "../TeensyI2CMaster.h"
// #include "Max6651Constants.h"

// struct ConfigureMax6651
// {
//     ConfigureMax6651(TeensyI2CMaster &i2c, uint8_t address);

//     ConfigureMax6651 &voltage(ConfigFanVoltage v);
//     ConfigureMax6651 &v5();
//     ConfigureMax6651 &v12();

//     ConfigureMax6651 &preScaler(ConfigPrescaler p);
//     ConfigureMax6651 &clockFrequency(float hertz);
//     ConfigureMax6651 &tachometerCountTime(TachometerCountTime time);

//     ConfigureMax6651 &gpioAsOutput(uint8_t index, bool high);
//     ConfigureMax6651 &gpioAsInput(uint8_t index);

//     ConfigureMax6651 &gpio2AsClockOutput();
//     ConfigureMax6651 &gpio2AsClockInput();

//     ConfigureMax6651 &gpio1AsFullOnInput();

//     ConfigureMax6651 &gpio0AsAlertOutput();

//     ConfigureMax6651 &enableAlarms(AlarmEnable a);
//     ConfigureMax6651 &maxAlarm(bool enable = true);
//     ConfigureMax6651 &minAlarm(bool enable = true);
//     ConfigureMax6651 &tachometerAlarm(bool enable = true);
//     ConfigureMax6651 &gpio1Alarm(bool state = true);
//     ConfigureMax6651 &gpio2Alarm(bool state = true);

//     ConfigureMax6651 &logger(Print *l);

//     TeensyI2CMaster *i2c_;
//     uint8_t address_;
//     uint8_t mode_config_;
//     uint8_t alarm_config_;
//     uint8_t gpio_config_;
//     uint8_t tachometer_count_time_;
//     uint8_t pulses_per_revolution_;
//     float f_clk_;
//     Print *logger_;
// };