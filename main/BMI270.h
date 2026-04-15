#pragma once

#include "driver/spi_master.h"

struct BMI270Data {
    int16_t acc_x = 0;
    int16_t acc_y = 0;
    int16_t acc_z = 0;
    int16_t gyr_x = 0;
    int16_t gyr_y = 0;
    int16_t gyr_z = 0;
};

class BMI270 {
public:
    BMI270();

    BMI270Data GetData();

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
};
