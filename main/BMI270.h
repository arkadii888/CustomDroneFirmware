#pragma once

#include "driver/spi_master.h"

class BMI270 {
public:
    BMI270();

    void PrintData();

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
