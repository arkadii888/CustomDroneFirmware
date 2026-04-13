#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

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

    spi_device_handle_t bmi270_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &bmi270_handle));

    // PMW3901MB Sleep
    gpio_set_direction(static_cast<gpio_num_t>(12), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(12), 1);

    // Check BMI270 Communication
    spi_transaction_t t = {};
    t.length = 24;
    t.tx_data[0] = 0x80;
    t.tx_data[1] = 0x00;
    t.tx_data[2] = 0x00;
    t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;

    auto err = spi_device_transmit(bmi270_handle, &t);
    if(err == ESP_OK) {
        std::cout << "BMI270 check: 0x" << std::hex << static_cast<int>(t.rx_data[2]) << std::endl;
    }
    else {
        std::cout << "BMI270 check error!" << std::endl;
    }

    std::cout << "Init finished!" << std::endl;

    while (true) {
        std::cout << "Battery voltage: " << battery.GetVoltage() << " V" << std::endl;
        std::cout << "Battery current: " << battery.GetCurrent() << " A" << std::endl;
        vTaskDelay(500/ portTICK_PERIOD_MS);
    }
}
