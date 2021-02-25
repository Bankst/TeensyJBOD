#include "Max6651.h"

static const float SECONDS_PER_MINUTE = 60.0;

Max6651::Max6651(TeensyI2CMaster *i2c, uint8_t address, Max6651Config config) : i2c(i2c), address(address), config(config)
{
    mode_config = config.mode_config_;
    gpio_config = config.gpio_config_;
    alarm_config = config.alarm_config_;
    tachometer_count_time = config.tachometer_count_time_;
    f_clk = config.f_clk_;
    pulses_per_rev = config.pulses_per_revolution_;
    k_tach_max_speed = K_TACH_LOWER_LIMIT;
    k_tach_min_speed = K_TACH_UPPER_LIMIT;
}

int Max6651::begin(bool gpio, bool alarm)
{
    if (!i2c)
        return -1;

    int error = write(COMMAND_CONFIG, mode_config);
    if (gpio)
        write(COMMAND_GPIO_DEFINE, gpio_config);
    if (alarm)
        write(COMMAND_ALARM_ENABLE, alarm_config);
    write(COMMAND_TACH_COUNT_TIME, tachometer_count_time);
    return error;
}

int Max6651::setSpeed(int rpm)
{
    if (rpm < 0)
    {
        return -1;
    }

    if (rpm == 0)
    {
        mode(MODE_FULL_OFF);
    }
    else
    {
        if (mode() == MODE_FULL_OFF)
        {
            mode(MODE_CLOSED_LOOP);
        }
    }

    // auto minSpeed = minimumSpeed();
    // auto maxSpeed = maximumSpeed();
    // TODO: fix min/maximumSpeed methods!!!
    // int minSpeed = 1000;
    // int maxSpeed = 8000;
    // if (rpm < minSpeed) rpm = minSpeed;
    // if (rpm > maxSpeed) rpm = maxSpeed;

    uint8_t k_tach = rpm_to_kTach(rpm);
    int result = set_tach_speed(k_tach);
    if (result < 0) return result;
    else return kTach_to_rpm(k_tach);
}

int Max6651::actualSpeed(uint8_t index) const
{    
    uint8_t tach = read_tachometer(index);
    if (tach < 0) return tach; // return negative error code

    float pulse_per_sec = tach / tachometerCountSeconds();
    float rps = pulse_per_sec / pulses_per_rev;
    float rpm = rps * SECONDS_PER_MINUTE;
    return int(rpm);
}

uint8_t Max6651::rawSpeed(uint8_t index) const
{
    return read_tachometer(index);
}

void Max6651::minimumSpeed(int minimumRPM)
{
    k_tach_min_speed = rpm_to_kTach(minimumRPM);
}

int Max6651::minimumSpeed() const
{
    return kTach_to_rpm(k_tach_min_speed);
}

void Max6651::maximumSpeed(int maximumRPM)
{
    k_tach_max_speed = rpm_to_kTach(maximumRPM);
}

int Max6651::maximumSpeed() const
{
    return kTach_to_rpm(k_tach_max_speed);
}

int Max6651::mode(ConfigMode m)
{
    last_mode = mode();
    mode_config = m | (mode_config & ~MODE_MASK);
    return write(COMMAND_CONFIG, mode_config);    
}

ConfigMode Max6651::mode() const
{
    uint8_t config;
    read(COMMAND_CONFIG, config);
    return ConfigMode(config & MODE_MASK);
}

int Max6651::voltage(ConfigFanVoltage v)
{
    mode_config = v | (mode_config & ~VOLTAGE_MASK);
    return write(COMMAND_CONFIG, mode_config);
}

ConfigFanVoltage Max6651::voltage() const
{
    uint8_t config;
    read(COMMAND_CONFIG, config);
    return ConfigFanVoltage(config & VOLTAGE_MASK);
}

int Max6651::prescaler(ConfigPrescaler p)
{
    mode_config = p | (mode_config & ~PRESCALER_MASK);
    return write(COMMAND_CONFIG, mode_config);
}

ConfigPrescaler Max6651::prescaler() const
{
    uint8_t config;
    read(COMMAND_CONFIG, config);
    return ConfigPrescaler(config & PRESCALER_MASK);
}

int Max6651::prescalerMultiplier() const
{
    // 0 --> 1
    // 1 --> 2
    // 2 --> 4
    // 3 --> 8
    // 4 --> 16
    // n --> 2^n
    int n = int(prescaler());
    //return (n == 0) ? 0 : 1 << n;
    return (1 << (n + 1)) >> 1;
}

TachometerCountTime Max6651::tachometerCountTime() const
{
    return TachometerCountTime(tachometer_count_time);
}

float Max6651::tachometerCountSeconds() const
{
    return (1 << tachometer_count_time) / 4.0f;
}

float Max6651::clockFrequencyHertz() const
{
    return f_clk;
}

int Max6651::kTach_to_rpm(uint8_t k_tach) const
{
    int prescaler_multiplier = prescalerMultiplier();
    float clock_frequency_hertz = clockFrequencyHertz();
    int k_tach_plus_one = int(k_tach) + 1;
    float numerator = prescaler_multiplier * clock_frequency_hertz;
    float denominator = (128.0f * pulses_per_rev * k_tach_plus_one);
    float rps = numerator / denominator;
    int rpm = int(rps * SECONDS_PER_MINUTE);
    return rpm;
}

uint8_t Max6651::rpm_to_kTach(int rpm) const
{
    float rps = float(rpm) / SECONDS_PER_MINUTE;
    int k_tach = int((clockFrequencyHertz() * prescalerMultiplier()) / (128.0f * pulses_per_rev * rps)) - 1;

    if (k_tach < 0) k_tach = 0;
    if (k_tach > 255) k_tach = 255;

    return uint8_t(k_tach);
}

int Max6651::read_tachometer(uint8_t index) const
{
    if (index < 0 || index > 3) return -1;

    uint8_t result;
    uint8_t error = read(TACH_COMMAND[index], result);
    if (error) return -error;

    // Round a result of one down to zero. A stationary fan can
    // return 1, which is annoying.
    return (result <= 1) ? 0 : result;
}

int Max6651::set_tach_speed(uint8_t k_tach) const
{
    return write(COMMAND_SPEED, k_tach);
}

/**
 *  Write to the I2C device.
 *
 *  Returns: An error code.
 */
uint8_t Max6651::write(Register command, uint8_t data) const
{
    return i2c->write(address, uint8_t(command), data);
}

/**
 *  Read the specified register. If an error
 *  occurs, return error.
 *
 *  Returns a error code.
 */
uint8_t Max6651::read(Register command, uint8_t &result) const
{
    return i2c->read(address, uint8_t(command), result);
}