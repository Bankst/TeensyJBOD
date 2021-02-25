#include "Max6651Config.h"

namespace
{
  uint8_t setState(uint8_t value, bool state, uint8_t mask)
  {
    return state
               ? value | mask
               : value & ~mask;
  }
}

Max6651Config::Max6651Config() : mode_config_(MODE_FULL_ON | DEFAULT_FAN_VOLTAGE | DEFAULT_PRESCALER),
                                 alarm_config_(DEFAULT_ALARM_ENABLE),
                                 gpio_config_(DEFAULT_GPIO_DEFINITION),
                                 tachometer_count_time_(DEFAULT_TACHOMETER_COUNT_TIME),
                                 pulses_per_revolution_(DEFAULT_PULSES_PER_REVOLUTION),
                                 f_clk_(DEFAULT_CLOCK_FREQUENCY)
{
}

Max6651Config &Max6651Config::voltage(ConfigFanVoltage v)
{
  mode_config_ &= ~VOLTAGE_MASK;
  mode_config_ |= v;
  return *this;
}

Max6651Config &Max6651Config::tachometerCountTime(TachometerCountTime time)
{
  tachometer_count_time_ = time;
  return *this;
}

Max6651Config &Max6651Config::gpioAsOutput(uint8_t index, bool high)
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[index];
  gpio_config_ |= high ? GPIO_DEFINE_HIGH[index] : GPIO_DEFINE_LOW[index];
  return *this;
}

Max6651Config &Max6651Config::gpioAsInput(uint8_t index)
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[index];
  gpio_config_ |= GPIO_DEFINE_INPUT[index];
  return *this;
}

Max6651Config &Max6651Config::gpio2AsClockOutput()
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[2];
  gpio_config_ |= GPIO_DEFINE_2_CLOCK_OUTPUT;
  return *this;
}

Max6651Config &Max6651Config::gpio2AsClockInput()
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[2];
  gpio_config_ |= GPIO_DEFINE_2_CLOCK_INPUT;
  return *this;
}

Max6651Config &Max6651Config::gpio1AsFullOnInput()
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[1];
  gpio_config_ |= GPIO_DEFINE_1_FULL_ON_INPUT;
  return *this;
}

Max6651Config &Max6651Config::gpio0AsAlertOutput()
{
  gpio_config_ &= ~GPIO_DEFINE_MASK[0];
  gpio_config_ |= GPIO_DEFINE_0_ALERT_OUTPUT;
  return *this;
}

Max6651Config &Max6651Config::preScaler(ConfigPrescaler p)
{
  mode_config_ &= ~PRESCALER_MASK;
  mode_config_ |= p;
  return *this;
}

Max6651Config &Max6651Config::clockFrequency(float hertz)
{
  f_clk_ = hertz;
  return *this;
}

Max6651Config &Max6651Config::maxAlarm(bool state)
{
  alarm_config_ = setState(alarm_config_, state, ALARM_ENABLE_MAX_OUTPUT);
  return *this;
}

Max6651Config &Max6651Config::minAlarm(bool state)
{
  alarm_config_ = setState(alarm_config_, state, ALARM_ENABLE_MIN_OUTPUT);
  return *this;
}

Max6651Config &Max6651Config::tachometerAlarm(bool state)
{
  alarm_config_ = setState(alarm_config_, state, ALARM_ENABLE_TACH_OVERFLOW);
  return *this;
}

Max6651Config &Max6651Config::gpio1Alarm(bool state)
{
  alarm_config_ = setState(alarm_config_, state, ALARM_ENABLE_GPIO_1);
  return *this;
}

Max6651Config &Max6651Config::gpio2Alarm(bool state)
{
  alarm_config_ = setState(alarm_config_, state, ALARM_ENABLE_GPIO_2);
  return *this;
}