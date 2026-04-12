#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "I2C.h"
#include "Battery.h"

extern "C" void app_main(void)
{
    std::cout << "Init starts!" << std::endl;

    I2C i2c;
    Battery battery(i2c.GetHandle());

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
