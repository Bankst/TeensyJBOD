#pragma once

#include <stdint.h>

#include "i2c_driver_wire.h"

class TeensyI2CMaster
{
public:
    TeensyI2CMaster(){};

    uint8_t write(uint8_t address, uint8_t register_address, uint8_t value)
    {
        Wire.beginTransmission(address);
        Wire.write(register_address);
        Wire.write(value);
        return Wire.endTransmission();
    }

    uint8_t read(uint8_t address, uint8_t register_address, uint8_t &result)
    {
        Wire.beginTransmission(address);
        Wire.write(register_address);
        Wire.endTransmission();
        Wire.requestFrom(address, uint8_t(1));
        if (Wire.available() >= 1)
        {
            result = Wire.read();
            return 0;
        }
        return -1;
    }
};
