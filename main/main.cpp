#include <iostream>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "I2C.h"
#include "Battery.h"
#include "SPI.h"
#include "BMI270.h"
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
    Battery battery(i2c.GetHandle());

    SPI spi;

    // PMW3901MB Sleep
    gpio_set_direction(static_cast<gpio_num_t>(12), GPIO_MODE_OUTPUT);
    gpio_set_level(static_cast<gpio_num_t>(12), 1);

    BMI270 bmi270;

    Wifi wifi;

    std::cout << "Init Finished!" << std::endl;

    // Communication Thread
    xTaskCreatePinnedToCore(Communication, "Communication", 4096, nullptr, 5, nullptr, 0);

    // Flight Thread
    while (true) {
        //BMI270Data bmi270Data = bmi270.GetData();
        //std::cout << "Accelerometer Data:" << std::endl;
        //std::cout << "Acc_X: " << bmi270Data.acc_x << std::endl;
        //std::cout << "Acc_Y: " << bmi270Data.acc_y << std::endl;
        //std::cout << "Acc_Z: " << bmi270Data.acc_z << std::endl;

        //std::cout << "Gyroscope Data:" << std::endl;
        //std::cout << "Gyr_X: " << bmi270Data.gyr_x << std::endl;
        //std::cout << "Gyr_Y: " << bmi270Data.gyr_y << std::endl;
        //std::cout << "Gyr_Z: " << bmi270Data.gyr_z << std::endl;

        //std::cout << "Battery Voltage: " << battery.GetVoltage() << " V" << std::endl;
        //std::cout << "Battery Current: " << battery.GetCurrent() << " A" << std::endl;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
