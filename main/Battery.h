#pragma once

#include "driver/i2c_master.h"

class Battery {
public:
    Battery(i2c_master_bus_handle_t i2c);

    float GetVoltage() const;
    float GetCurrent() const;

private:
    i2c_master_dev_handle_t handle;
};
