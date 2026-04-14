#pragma once

#include "driver/spi_master.h"

class BMI270 {
public:
    BMI270();

private:
    void CheckCommunication();
    void DisablePowersave();
    void PrepareForConfigUpload();
    void UploadConfig();
    void UnpackConfig();
    void CheckConfigUploadStatus();

private:
    spi_device_handle_t handle;
};
