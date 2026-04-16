#include <iostream>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "I2C.h"
#include "VoltageMonitor.h"
#include "SPI.h"
#include "IMU.h"
#include "Wifi.h"

void Communication(void *pvParameters) {
    while(true) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    std::cout << "Init Started!" << std::endl;

    I2C i2c;
    VoltageMonitor voltageMonitor(i2c.GetHandle());

    SPI spi;

    // PMW3901MB Sleep
    gpio_set_direction(static_cast<gpio_num_t>(12), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(12), 1);

    IMU imu;

    Wifi wifi;

    std::cout << "Init Finished!" << std::endl;

    // Communication Thread
    xTaskCreatePinnedToCore(Communication, "Communication", 4096, nullptr, 5, nullptr, 0);

    // Flight Thread
    while (true) {
        //IMUData imuData = imu.GetData();
        //std::cout << "Accelerometer Data:" << std::endl;
        //std::cout << "Acc_X: " << imuData.acc_x << std::endl;
        //std::cout << "Acc_Y: " << imuData.acc_y << std::endl;
        //std::cout << "Acc_Z: " << imuData.acc_z << std::endl;

        //std::cout << "Gyroscope Data:" << std::endl;
        //std::cout << "Gyr_X: " << imuData.gyr_x << std::endl;
        //std::cout << "Gyr_Y: " << imuData.gyr_y << std::endl;
        //std::cout << "Gyr_Z: " << imuData.gyr_z << std::endl;

        //std::cout << "VoltageMonitor Voltage: " << voltageMonitor.GetVoltage() << " V" << std::endl;
        //std::cout << "VoltageMonitor Current: " << voltageMonitor.GetCurrent() << " A" << std::endl;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
