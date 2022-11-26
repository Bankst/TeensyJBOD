#pragma once

#include <set>
#include <map>

#include <i2c_driver_wire.h>

#include "TeensyThreads.h"

#include "i2c/TeensyI2CMaster.h"
#include "i2c/max6651/Max6651.h"
#include "i2c/supermicro_pmbus/SuperPMBus.h"

#define PSU_ADDRESS 0x39
#define PSU_UNK1_ADDRESS 0x3D
#define PTJBOD_MAX6651_ADDRESS 0x1B
#define BACKPLANE_UNK0_ADDRESS 0x24
#define BACKPLANE_UNK1_ADDRESS 0x2D
#define BACKPLANE_MAX6651_ADDRESS 0x48
#define BACKPLANE_UNK2_ADDRESS 0x49

#define FAN_COUNT 3

#define FAN_SET_SPEED 4100
#define FAN_MIN_SPEED 3800
#define FAN_MAX_SPEED 4600

class I2CManager
{
private:
  TeensyI2CMaster i2c;

  Max6651 backplane_fan_controller{&i2c, BACKPLANE_MAX6651_ADDRESS};
  // Max6651 ptjbodcb2_fan_controller{&i2c, PTBOD_MAX6651_ADDRESS}
  SuperPMBus psu{&i2c, PSU_ADDRESS};
  // SuperPMBus psu2{&i2c, PSU_UNK1_ADDRESS};

public:
  I2CManager()
  {
    Wire.begin();
  }

  void init();

  void begin_fan_control();
  void begin_psu_readings();

  void fan_speed_scan_task();
  void i2c_scan_task();

private:
  bool fan_begun = false;

  void dump_device(uint8_t address);

  std::set<uint8_t> detected_i2c_slaves;

  void log_debug(std::string str) {};
  void log_info(std::string str) {};
  void log_warn(std::string str) {};
  void log_error(std::string str) {};

  // void log_debug(std::string str) { serialout << F("[I2CManager.h - DEBUG] ") << str.c_str() << endl; }
  // void log_info(std::string str) { serialout << F("[I2CManager.h - INFO] ") << str.c_str() << endl; }
  // void log_warn(std::string str) { serialout << F("[I2CManager.h - WARNING] ") << str.c_str() << endl; }
  // void log_error(std::string str) { serialout << F("[I2CManager.h - ERROR] ") << str.c_str() << endl; }
};

void I2CManager::init()
{
  i2c_scan_task();
}

void I2CManager::begin_fan_control()
{
  log_info("Fan control begin!");
  std::string fan_str = "Fan speeds: ";
  for (int i = 0; i < FAN_COUNT; i++)
  {
    fan_str += StringUtil::to_string(backplane_fan_controller.actualSpeed(i));
    if (i != 3)
      fan_str += ", ";
  }
  log_info(fan_str);

  int fan_speed = backplane_fan_controller.actualSpeed(0);
  if (fan_speed > FAN_MIN_SPEED && fan_speed < FAN_MAX_SPEED)
  {
    log_info("Fan speed OK!");
  }
  else
  {
    backplane_fan_controller.begin();
    backplane_fan_controller.mode(ConfigMode::MODE_CLOSED_LOOP);
    fan_begun = true;

    int set_speed = backplane_fan_controller.setSpeed(FAN_SET_SPEED);
    log_info("Fan speed out of range! Set to " + StringUtil::to_string(set_speed) + "RPM");
  }
}

void I2CManager::begin_psu_readings()
{
  log_info("psu0 init");
  psu.dump();
}

void I2CManager::dump_device(uint8_t address)
{
  for (uint8_t reg = 0; reg < 0xFF; reg++)
  {
    uint8_t value = -2;
    uint8_t result = i2c.read(address, reg, value);

    if (result == 0)
    {
      char opcode_hexbuf[33];
      sprintf(opcode_hexbuf, "%02X", reg);

      char data_hexbuf[33];
      sprintf(data_hexbuf, "%02X", (int)value);

      log_info("addr " + StringUtil::to_string((int)address) + " - read 0x" + std::string(opcode_hexbuf) + ": " + std::string(data_hexbuf));
    }
  }
}

void I2CManager::fan_speed_scan_task()
{
#ifdef FAN_DEBUG_SCAN
  std::string fan_str = "Fan speeds: ";
  for (int i = 0; i < FAN_COUNT; i++)
  {
    fan_str += to_string(backplane_fan_controller.actualSpeed(i));
    if (i != 3)
      fan_str += ", ";
  }
  log_info(fan_str);
#else
  for (int i = 0; i < FAN_COUNT; i++)
  {
    int fan_speed = backplane_fan_controller.actualSpeed(i);
    if (fan_speed < FAN_MIN_SPEED || fan_speed > FAN_MAX_SPEED)
    {
      if (!fan_begun)
      {
        backplane_fan_controller.begin();
        backplane_fan_controller.mode(ConfigMode::MODE_CLOSED_LOOP);
        int set_speed = backplane_fan_controller.setSpeed(FAN_SET_SPEED);
        log_info("Fan " + StringUtil::to_string(i) + " speed out of range! (" + StringUtil::to_string(fan_speed) + "RPM) Set to " + StringUtil::to_string(set_speed) + "RPM");
      }
      else
      {
        log_info("Fan " + StringUtil::to_string(i) + " speed out of range! (" + StringUtil::to_string(fan_speed) + "RPM)");
      }
    }
  }
#endif
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

#define I2C_SCAN_DEBUG
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
