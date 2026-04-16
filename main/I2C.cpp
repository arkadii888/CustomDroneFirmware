#include "I2C.h"

I2C::I2C() {
    i2c_master_bus_config_t config = {};
    config.clk_source = I2C_CLK_SRC_DEFAULT;
    config.i2c_port = -1;
    config.scl_io_num = GPIO_NUM_4;
    config.sda_io_num = GPIO_NUM_3;
    config.glitch_ignore_cnt = 7;
    config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &handle));
}

i2c_master_bus_handle_t I2C::GetHandle() {
    return handle;
}
