#pragma once

#include "driver/i2c_master.h"

class Battery {
public:
    Battery(i2c_master_dev_handle_t handle);

    float GetVoltage() const;
    float GetCurrent() const;

private:
    i2c_master_dev_handle_t _handle;
};
