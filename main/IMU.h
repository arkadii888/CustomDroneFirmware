#pragma once

#include "driver/spi_master.h"

struct IMUData {
    int16_t acc_x = 0;
    int16_t acc_y = 0;
    int16_t acc_z = 0;
    int16_t gyr_x = 0;
    int16_t gyr_y = 0;
    int16_t gyr_z = 0;
};

// BMI270
class IMU {
public:
    IMU();
    ~IMU();

    IMUData GetData();

private:
    void CheckCommunication();
    void DisablePowersave(const uint8_t value);
    void PrepareForConfigUpload();
    void UploadConfig();
    void UnpackConfig();
    void CheckConfigUploadStatus();
    void EnableSensors();
    void ConfigAccelerometer();
    void ConfigGyroscope();

private:
    spi_device_handle_t handle;
    uint8_t* tx_buffer = nullptr;
    uint8_t* rx_buffer = nullptr;
};
