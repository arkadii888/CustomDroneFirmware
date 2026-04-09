#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

class Battery {
public:
    Battery(i2c_master_dev_handle_t handle) : _handle(handle) {}

    float GetVoltage() {
        uint8_t reg_addr = 0x04;
        uint8_t raw_data[2] = {0};

        auto err = i2c_master_transmit_receive(_handle, &reg_addr, 1, raw_data, 2, -1);
        if(err == ESP_OK) {
            uint16_t combined_raw = (raw_data[0] << 8) | raw_data[1];
            combined_raw = combined_raw >> 3;
            float voltage = combined_raw * 0.008f;
            return voltage;
        }

        return 0.0f;
    }

private:
    i2c_master_dev_handle_t _handle;
};

extern "C" void app_main(void)
{
    std::cout << "Init starts!" << std::endl;

    // IC2 Init
    i2c_master_bus_config_t i2c_mst_config = {};
    i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_mst_config.i2c_port = -1;
    i2c_mst_config.scl_io_num = static_cast<gpio_num_t>(4);
    i2c_mst_config.sda_io_num = static_cast<gpio_num_t>(3);
    i2c_mst_config.glitch_ignore_cnt = 7;
    i2c_mst_config.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = 0x40;
    dev_cfg.scl_speed_hz = 400000;

    // Battery Init
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    Battery battery(dev_handle);

    std::cout << "Init finished!" << std::endl;

    while (true) {
        std::cout << "Battery voltage: " << battery.GetVoltage() << " V" << std::endl;
        vTaskDelay(500/ portTICK_PERIOD_MS);
    }
}
