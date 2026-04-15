#include "BMI270.h"

#include "bmi270_config.h"
#include "esp_heap_caps.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "bmi270_config.h"

BMI270::BMI270() {
    spi_device_interface_config_t config = {};
    config.clock_speed_hz = 1000000;
    config.mode = 0;
    config.spics_io_num = 46;
    config.queue_size = 7;

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &config, &handle));

    // Upload Config
    CheckCommunication();
    DisablePowersave(0x00);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    PrepareForConfigUpload();
    UploadConfig();
    UnpackConfig();
    vTaskDelay(150 / portTICK_PERIOD_MS);
    CheckConfigUploadStatus();

    // Setup Accelerometer and Gyroscope
    DisablePowersave(0x00);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    EnableSensors();
    ConfigAccelerometer();
    ConfigGyroscope();
    DisablePowersave(0x02);
    vTaskDelay(150 / portTICK_PERIOD_MS);
}

void BMI270::CheckCommunication() {
    spi_transaction_t t = {};
    t.length = 24;
    t.tx_data[0] = 0x80;
    t.tx_data[1] = 0x00;
    t.tx_data[2] = 0x00;
    t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
    std::cout << "BMI270::CheckCommunication: 0x" << std::hex << static_cast<int>(t.rx_data[2]) << std::endl;
}

void BMI270::DisablePowersave(const uint8_t value) {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x7C & 0x7F;
    t.tx_data[1] = value;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::PrepareForConfigUpload() {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x59 & 0x7F;
    t.tx_data[1] = 0x00;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::UploadConfig() {
    uint8_t* buffer = static_cast<uint8_t*>(heap_caps_malloc(8193, MALLOC_CAP_DMA));
    memcpy(buffer, bmi270_config_file, 8193);

    spi_transaction_t t = {};
    t.length = 8193 * 8;
    t.tx_buffer = buffer;

    auto err = spi_device_transmit(handle, &t);
    free(buffer);
    if(err != ESP_OK) {
        std::cout << "BMI270::UploadConfig Error: " << esp_err_to_name(err) << std::endl;
        abort();
    }
}

void BMI270::UnpackConfig() {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x59 & 0x7F;
    t.tx_data[1] = 0x01;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::CheckConfigUploadStatus() {
    spi_transaction_t t = {};
    t.length = 24;
    t.tx_data[0] = 0x21 | 0x80;
    t.tx_data[1] = 0x00;
    t.tx_data[2] = 0x00;
    t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));

    std::cout << "BMI270::CheckConfigUploadStatus: 0x" << std::hex << static_cast<int>(t.rx_data[2]) << std::dec << std::endl;

    if(t.rx_data[2] == 0x01) {
        std::cout << "BMI270::CheckConfigUploadStatus Success!" << std::endl;
    }
}

void BMI270::EnableSensors() {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x7D & 0x7F;
    t.tx_data[1] = 0x0E;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::ConfigAccelerometer() {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x40 & 0x7F;
    t.tx_data[1] = 0xA8;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::ConfigGyroscope() {
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_data[0] = 0x42 & 0x7F;
    t.tx_data[1] = 0xA9;
    t.flags = SPI_TRANS_USE_TXDATA;

    ESP_ERROR_CHECK(spi_device_transmit(handle, &t));
}

void BMI270::PrintData() {
    uint8_t* tx_buffer = static_cast<uint8_t*>(heap_caps_calloc(14, 1, MALLOC_CAP_DMA));
    uint8_t* rx_buffer = static_cast<uint8_t*>(heap_caps_calloc(14, 1, MALLOC_CAP_DMA));

    tx_buffer[0] = 0x0C | 0x80;

    spi_transaction_t t = {};
    t.length = 14 * 8;
    t.tx_buffer = tx_buffer;
    t.rx_buffer = rx_buffer;

    auto err = spi_device_transmit(handle, &t);
    if(err != ESP_OK) {
        free(tx_buffer);
        free(rx_buffer);
        std::cout << "BMI270::UploadConfig Error: " << esp_err_to_name(err) << std::endl;
        abort();
    }

    int16_t acc_x = (rx_buffer[3] << 8) | rx_buffer[2];
    int16_t acc_y = (rx_buffer[5] << 8) | rx_buffer[4];
    int16_t acc_z = (rx_buffer[7] << 8) | rx_buffer[6];

    int16_t gyr_x = (rx_buffer[9] << 8) | rx_buffer[8];
    int16_t gyr_y = (rx_buffer[11] << 8) | rx_buffer[10];
    int16_t gyr_z = (rx_buffer[13] << 8) | rx_buffer[12];

    free(tx_buffer);
    free(rx_buffer);

    std::cout << "BMI270::PrintData: Accelerometer Data" << std::endl;
    std::cout << "Acc_X: " << acc_x << std::endl;
    std::cout << "Acc_Y: " << acc_y << std::endl;
    std::cout << "Acc_Z: " << acc_z << std::endl;

    std::cout << "BMI270::PrintData: Gyroscope Data" << std::endl;
    std::cout << "Gyr_X: " << gyr_x << std::endl;
    std::cout << "Gyr_Y: " << gyr_y << std::endl;
    std::cout << "Gyr_Z: " << gyr_z << std::endl;
}
