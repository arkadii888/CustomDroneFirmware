#pragma once

#include "driver/ledc.h"

class Motor {
public:
    Motor(gpio_num_t pin, ledc_channel_t chnl);

    void SetThrottle(float percentage);

private:
    ledc_channel_t channel;
};
