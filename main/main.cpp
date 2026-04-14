#include <cstdint>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "bmi270_config.h"

#include "I2C.h"
#include "Battery.h"
#include "SPI.h"

extern "C" void app_main(void)
{
    std::cout << "Init starts!" << std::endl;

    I2C i2c;
    Battery battery(i2c.GetHandle());

    SPI spi;

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

    // Disable BMI270 Powersave
    spi_transaction_t t2 = {};
    t2.length = 16;
    t2.tx_data[0] = 0x7C & 0x7F;
    t2.tx_data[1] = 0x00;
    t2.flags = SPI_TRANS_USE_TXDATA;

    auto err2 = spi_device_transmit(bmi270_handle, &t2);
    std::cout << "Disable BMI270 Powersave: " << esp_err_to_name(err2) << std::endl;
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // BMI270 Prepare For Config
    spi_transaction_t t3 = {};
    t3.length = 16;
    t3.tx_data[0] = 0x59 & 0x7F;
    t3.tx_data[1] = 0x00;
    t3.flags = SPI_TRANS_USE_TXDATA;

    auto err3 = spi_device_transmit(bmi270_handle, &t3);
    std::cout << "BMI270 Prepare For Config: " << esp_err_to_name(err3) << std::endl;

    // BMI270 Load Config
    uint8_t* buffer = static_cast<uint8_t*>(heap_caps_malloc(8193, MALLOC_CAP_DMA));
    memcpy(buffer, bmi270_config_file, 8193);

    spi_transaction_t t4 = {};
    t4.length = 8193 * 8;
    t4.tx_buffer = buffer;

    auto err4 = spi_device_transmit(bmi270_handle, &t4);
    free(buffer);

    std::cout << "BMI270 Load Config: " << esp_err_to_name(err4) << std::endl;

    // BMI270 Unpack Config
    spi_transaction_t t5 = {};
    t5.length = 16;
    t5.tx_data[0] = 0x59 & 0x7F;
    t5.tx_data[1] = 0x01;
    t5.flags = SPI_TRANS_USE_TXDATA;

    auto err5 = spi_device_transmit(bmi270_handle, &t5);
    std::cout << "BMI270 Unpack Config: " << esp_err_to_name(err5) << std::endl;
    vTaskDelay(150 / portTICK_PERIOD_MS);

    // Check Config Load Status
    spi_transaction_t t6 = {};
    t6.length = 24;
    t6.tx_data[0] = 0x21 | 0x80;
    t6.tx_data[1] = 0x00;
    t6.tx_data[2] = 0x00;
    t6.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;

    auto err6 = spi_device_transmit(bmi270_handle, &t6);
    std::cout << "Check Config Load Status: " << esp_err_to_name(err6) << std::endl;

    std::cout << "BMI270 Config Load Answer: 0x" << std::hex << static_cast<int>(t6.rx_data[2]) << std::dec << std::endl;

    if(t6.rx_data[2] == 0x01) {
        std::cout << "BMI270 Config Load Success!" << std::endl;
    }

    std::cout << "Init finished!" << std::endl;

    while (true) {
        std::cout << "Battery voltage: " << battery.GetVoltage() << " V" << std::endl;
        std::cout << "Battery current: " << battery.GetCurrent() << " A" << std::endl;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
