#include "Motor.h"

Motor::Motor(gpio_num_t pin, ledc_channel_t chnl) : channel(chnl) {
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.freq_hz = 20000;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timerConfig);

    ledc_channel_config_t channelConfig = {};
    channelConfig.gpio_num = pin;
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = channel;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ledc_channel_config(&channelConfig);
}

void Motor::SetThrottle(float percentage) {
    if(percentage < 0.0f) {
        percentage = 0.0f;
    }
    if(percentage > 100.0f) {
        percentage = 100.0f;
    }

    uint32_t duty = static_cast<uint32_t>((percentage / 100.0f) * 1023.0f);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}
