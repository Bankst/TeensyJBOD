#pragma once

#include <set>

#include <i2c_driver_wire.h>

#include "TeensyThreads.h"

#include "i2c/TeensyI2CMaster.h"
#include "i2c/max6651/Max6651.h"
#include "i2c/max6651_new/Max6651.h"

#define I2C_SCAN_DEBUG

#define PSU_UNK0_ADDRESS 0x39
#define PSU_UNK1_ADDRESS 0x3D
#define JBOD_MAX6651_ADDRESS 0x1B
#define BACKPLANE_UNK0_ADDRESS 0x24
#define BACKPLANE_UNK1_ADDRESS 0x2D
#define BACKPLANE_MAX6651_ADDRESS 0x48
#define BACKPLANE_UNK2_ADDRESS 0x49

#define GOOD_FAN_SPEED 85

class I2CManager
{
private:
  TeensyI2CMaster i2c;

  Max6651 backplane_fan_controller{&i2c, BACKPLANE_MAX6651_ADDRESS};
  // Max6651ClosedLoop fan_controller_closedloop{&i2c, BACKPLANE_MAX6651_ADDRESS};

public:
  I2CManager()
  {
    Wire.begin();
  }

  void begin_fan_control();

  void fan_speed_scan_task();
  void i2c_scan_task();

private:
  std::set<uint8_t> detected_i2c_slaves;

  void log_debug(std::string str) { serialout << F("[I2CManager.h - DEBUG] ") << str.c_str() << endl; }
  void log_info(std::string str) { serialout << F("[I2CManager.h - INFO] ") << str.c_str() << endl; }
  void log_warn(std::string str) { serialout << F("[I2CManager.h - WARNING] ") << str.c_str() << endl; }
  void log_error(std::string str) { serialout << F("[I2CManager.h - ERROR] ") << str.c_str() << endl; }
};

void I2CManager::begin_fan_control()
{
  backplane_fan_controller.begin();
  backplane_fan_controller.mode(ConfigMode::MODE_CLOSED_LOOP);
  int set_speed = backplane_fan_controller.setSpeed(4000);
  log_info("Fan control begin!");
  log_info("Set speed to " + to_string(set_speed) + "RPM");
}

void I2CManager::fan_speed_scan_task()
{
  std::string fan_str = "Fan speeds: ";
  for (int i = 0; i < 4; i++)
  {
    fan_str += to_string(backplane_fan_controller.actualSpeed(i));
    if (i != 3) fan_str += ", ";
  }
  log_info(fan_str);
}

void I2CManager::i2c_scan_task()
{
  // scanning
  for (uint8_t address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0)
    {
      detected_i2c_slaves.insert(address);
    }
    else if (error == 2)
    {
      detected_i2c_slaves.erase(address);
    }
  }

#ifdef I2C_SCAN_DEBUG
  // output scan results
  if (detected_i2c_slaves.size() > 0)
  {
    std::string result = "";
    std::set<uint8_t>::iterator iter;
    for (iter = detected_i2c_slaves.begin(); iter != detected_i2c_slaves.end(); ++iter)
    {
      if (iter != detected_i2c_slaves.begin())
      {
        result += ", ";
      }
      char addrString[3];
      itoa(*iter, addrString, 16);
      result += "0x" + std::string(addrString);
    }
    // Serial.println(result.c_str());
    log_info("I2C Scan completed: " + result);
  }
  else
  {
    log_info("I2C Scan completed: no devices");
  }
#endif
}