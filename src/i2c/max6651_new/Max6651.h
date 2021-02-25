#pragma once


#include "../TeensyI2CMaster.h"
#include "Max6651Config.h"
#include "Max6651Constants.h"

class Max6651
{
private:
    TeensyI2CMaster *i2c;
    uint8_t address;
    Max6651Config config;
public:
    Max6651(TeensyI2CMaster *i2c, uint8_t address, Max6651Config config);
    Max6651(TeensyI2CMaster *i2c, uint8_t address) : Max6651(i2c, address, Max6651Config()){};

    int begin(bool gpio = false, bool alarm = false);

    int setSpeed(int rpm);
    int actualSpeed(uint8_t index) const;
    uint8_t rawSpeed(uint8_t index) const;

    void minimumSpeed(int minimumRPM);
    int minimumSpeed() const;

    void maximumSpeed(int maximumRPM);
    int maximumSpeed() const;


    int mode(ConfigMode m);
    ConfigMode mode() const;

    int voltage(ConfigFanVoltage v);
    ConfigFanVoltage voltage() const;

    int prescaler(ConfigPrescaler p);
    ConfigPrescaler prescaler() const;

    int prescalerMultiplier() const;
    TachometerCountTime tachometerCountTime() const;
    float tachometerCountSeconds() const;
    float clockFrequencyHertz() const;
private:
    ConfigMode last_mode;

    uint8_t mode_config;
    uint8_t gpio_config;
    uint8_t alarm_config;
    uint8_t tachometer_count_time;
    float f_clk;

    int target_rpm = -1;

    int pulses_per_rev;
    int k_tach_min_speed;
    int k_tach_max_speed;
private:

    int kTach_to_rpm(uint8_t k_tach) const;
    uint8_t rpm_to_kTach(int rpm) const;

    int read_tachometer(uint8_t index) const;

    int set_tach_speed(uint8_t tach) const;

    uint8_t write(Register command, uint8_t data) const;
    uint8_t read(Register command, uint8_t &result) const;
};

