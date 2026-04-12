#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "Battery.h"

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

    // Battery(Current / Voltage Monitor) Init
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = 0x40;
    dev_cfg.scl_speed_hz = 400000;

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    Battery battery(dev_handle);

    // SPI Init
    spi_bus_config_t buscfg = {};
    buscfg.miso_io_num = 43;
    buscfg.mosi_io_num = 14;
    buscfg.sclk_io_num = 44;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // BMI270(Accelerometer + Gyroscope) Init
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 1000000;
    devcfg.mode = 0;
    devcfg.spics_io_num = 46;
    devcfg.queue_size = 7;

    spi_device_handle_t spi_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));

    std::cout << "Init finished!" << std::endl;

    while (true) {
        std::cout << "Battery voltage: " << battery.GetVoltage() << " V" << std::endl;
        std::cout << "Battery current: " << battery.GetCurrent() << " A" << std::endl;
        vTaskDelay(500/ portTICK_PERIOD_MS);
    }
}
