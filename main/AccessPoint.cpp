#include "AccessPoint.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <cstring>

AccessPoint::AccessPoint() {
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
