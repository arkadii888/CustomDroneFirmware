#pragma once

#include "driver/i2c_master.h"

class I2C {
public:
    I2C();

    i2c_master_bus_handle_t GetHandle();

private:
    i2c_master_bus_handle_t handle;
};
