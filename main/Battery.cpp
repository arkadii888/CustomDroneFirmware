#include "Battery.h"

Battery::Battery(i2c_master_bus_handle_t i2c) {
    i2c_device_config_t config = {};
    config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    config.device_address = 0x40;
    config.scl_speed_hz = 400000;

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c, &config, &handle));
}

float Battery::GetVoltage() const {
    uint8_t reg_addr = 0x04;
    uint8_t raw_data[2] = {0};

    auto err = i2c_master_transmit_receive(handle, &reg_addr, 1, raw_data, 2, -1);
    if(err == ESP_OK) {
        uint16_t bus_voltage_raw = (raw_data[0] << 8) | raw_data[1];
        bus_voltage_raw = bus_voltage_raw >> 3;
        return bus_voltage_raw * 0.008f;
    }

    return 0.0f;
}

float Battery::GetCurrent() const {
    uint8_t reg_addr = 0x03;
    uint8_t raw_data[2] = {0};

    auto err = i2c_master_transmit_receive(handle, &reg_addr, 1, raw_data, 2, -1);
    if(err == ESP_OK) {
        int16_t shunt_voltage_raw = (raw_data[0] << 8) | raw_data[1];
        shunt_voltage_raw = shunt_voltage_raw >> 3;
        float shunt_voltage = shunt_voltage_raw * 0.00004f;
        return shunt_voltage / 0.01f;
    }

    return 0.0f;
}
