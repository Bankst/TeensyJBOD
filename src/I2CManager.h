#pragma once

#include <set>

#include <i2c_driver_wire.h>

#include "TeensyThreads.h"

#include "i2c/TeensyI2CMaster.h"
#include "i2c/max6651/Max6651.h"

#define I2C_SCAN_DEBUG

// fault values
#define RESERVED_FAULT_0 0x01
#define PRIMARY_OTP 0x02
#define SECONDARY_OTP 0x03
#define RESERVED_FAULT_1 0x04
#define SCP_FAULT 0x05
#define OCP_FAULT_220V 0x06
#define OCP_FAULT_110V 0x07
#define OVP_FAULT 0x08
#define DC12V_OVP_FAULT 0x09
#define DC12V_UVP_FAULT 0x0A
#define AC_LOSS 0x0B
#define FAN2_FAULT 0x0C
#define FAN1_FAULT 0x0D
#define OPP_FAULT 0x0E
#define OTHER_FAULT 0x0F

// read-only registers
#define READ_TEMPERATURE 0x09
#define READ_FAN_1_SPEED 0x0A
#define READ_FAN_2_SPEED 0x0B
#define READ_POWER_STATUS 0x0C   // hex 01 = DC_GOOD, byte 00 = No DC Output
#define READ_AC_RMS_CURRENT 0x14 // this byte divided by 16 is the AC (RMS) input current

#define TEMP_HIGH_LIMIT 0x0D
#define FAN_1_LOW_LIMIT 0x0E
#define FAN_2_LOW_LIMIT 0x0F
#define RESERVED_2 0x10
#define RESERVED_3 0x11
#define RESERVED_4 0x12
#define RESERVED_5 0x13
#define RESERVED_6 0x15
#define FIRMWARE_VERSION 0x16 // i.e. 2.0 encoded as 0x20
#define FRU_FILE_REV 0x17
#define AC_CURRENT_LIMIT 0xF0    // this byte divided by 16 is the AC (RMS) input current limit
#define DC12V_CURRENT_LIMIT 0xF1 // this byte divided by 2 is the 12V DC output current
#define PSU_WATTAGE_LOWER 0xF2   // output power in watts (lower byte)
#define PSU_WATTAGE_UPPER 0xF3   // output power in watts (upper byte)
#define INPUT_VOLTAGE 0xF4       // 100-240VAC input voltage reading
#define INPUT_POWER_LOWER 0xF5   // input power in watts (lower byte) (0x0864=360W)
#define INPUT_POWER_UPPER 0xF6   // input power in watts (upper byte)
#define WRITE_PROTECTION 0xFF    // 0x88 = writable, otherwise read-only

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
  Max6651ClosedLoop fan_controller_closedloop{&i2c, BACKPLANE_MAX6651_ADDRESS};

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
  int begin_retcode = fan_controller_closedloop.begin();
  Serial.print("fan_begin: ");
  Serial.println(begin_retcode);

  int command_retcode = i2c.write(BACKPLANE_MAX6651_ADDRESS, COMMAND_CONFIG, uint8_t(B00111010));
  Serial.print("fan_command: ");
  Serial.println(command_retcode);

  int speed_retcode = i2c.write(BACKPLANE_MAX6651_ADDRESS, COMMAND_DAC, uint8_t(GOOD_FAN_SPEED));
  Serial.print("fan_speed: ");
  Serial.println(speed_retcode);

  // threads.delay(1500);

  // int max_retcode = fan_controller_closedloop.targetSpeed(1);
  // Serial.print("fan_max: ");
  // Serial.println(max_retcode);

  // threads.delay(1500);

  // int min_retcode = fan_controller_closedloop.setSpeed(100);
  // Serial.print("fan_min: ");
  // Serial.println(min_retcode);

  // int run_retcode = fan_controller_closedloop.run();
  // Serial.print("fan_run: ");
  // Serial.println(run_retcode);

  // i2c.write(MAX6651_ADDRESS, uint8_t(B00000000), 0);

  // int bump_rpm = 6700 / 12;

  // for (int i = 1; i < 13; i++)
  // {
  //   int speed = i * bump_rpm;
  //   Serial.print("fan_targetspeed: ");
  //   Serial.print(speed);
  //   int set_retcode = fan_controller_closedloop.targetSpeed(speed);
  //   Serial.print(", retcode: ");
  //   Serial.println(set_retcode);
  //   threads.delay(1000);
  // }
}

void I2CManager::fan_speed_scan_task()
{
  Serial.print("Fan speeds: ");
  for (int i = 0; i < 4; i++)
  {
    int speed = fan_controller_closedloop.actualSpeed(i);
    if (speed > 5500) {
      i2c.write(BACKPLANE_MAX6651_ADDRESS, COMMAND_DAC, uint8_t(GOOD_FAN_SPEED));
    }
    Serial.print(speed);
    if (i != 3) Serial.print(", ");
  }
  Serial.println();
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