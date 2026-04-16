#include <iostream>
#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <cstring>

#include "I2C.h"
#include "Battery.h"
#include "SPI.h"
#include "BMI270.h"

void WifiInit() {
    auto err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    wifi_config_t wifiConfig = {};
    strcpy(reinterpret_cast<char*>(wifiConfig.ap.ssid), "Drone");
    strcpy(reinterpret_cast<char*>(wifiConfig.ap.password), "12345678");
    wifiConfig.ap.ssid_len = 5;
    wifiConfig.ap.channel = 1;
    wifiConfig.ap.max_connection = 4;
    wifiConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());
}

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

    WifiInit();

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
