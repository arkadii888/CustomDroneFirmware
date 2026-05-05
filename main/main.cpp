#include <cmath>
#include <iostream>
#include <sys/types.h>
#include <math.h>
#include <algorithm>
#include <atomic>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/ledc_types.h"
#include "lwip/sockets.h"

#include "I2C.h"
#include "VoltageMonitor.h"
#include "SPI.h"
#include "IMU.h"
#include "AccessPoint.h"
#include "Motor.h"
#include "PIDContorller.h"

struct CommunicationContext {
    VoltageMonitor* voltageMonitor = nullptr;
    std::atomic<bool> arm = false;
};

void Communication(void *pvParameters) {
    auto context = static_cast<CommunicationContext*>(pvParameters);

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(s, reinterpret_cast<sockaddr*>(&server), sizeof(server));

    int recivedCode = 0;

    while(true) {
        sockaddr_in client;
        socklen_t socklen = sizeof(client);

        recvfrom(s, &recivedCode, sizeof(recivedCode), 0, reinterpret_cast<sockaddr*>(&client), &socklen);
        std::cout << "Communication: Code Recieved: " << recivedCode << std::endl;

        if(recivedCode == 1) {
            const char* reply = "Alive";
            sendto(s, reply, strlen(reply), 0, reinterpret_cast<sockaddr*>(&client), socklen);
        }
        if(recivedCode == 2) {
            if(context == nullptr || context->voltageMonitor == nullptr) {
                std::cout << "Communication: Code 2 Error" << std::endl;
                continue;
            }
            float reply[2] = {context->voltageMonitor->GetVoltage(), context->voltageMonitor->GetCurrent()};
            sendto(s, reply, sizeof(reply), 0, reinterpret_cast<sockaddr*>(&client), socklen);
        }
        if(recivedCode == 3) {
            if(context == nullptr) {
                std::cout << "Communication: Code 3 Error" << std::endl;
                continue;
            }
            context->arm = !context->arm;
            const char* reply = context->arm ? "Armed" : "Disarmed";
            sendto(s, reply, strlen(reply), 0, reinterpret_cast<sockaddr*>(&client), socklen);
        }
    }
}

extern "C" void app_main(void)
{
    std::cout << "Good Morning!" << std::endl;

    I2C i2c;
    VoltageMonitor voltageMonitor(i2c.GetHandle());

    // PMW3901MB Sleep
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_12, 1);

    SPI spi;
    IMU imu;

    AccessPoint accessPoint;

    CommunicationContext context;
    context.voltageMonitor = &voltageMonitor;
    context.arm = false;

    Motor motorLU(GPIO_NUM_5, LEDC_CHANNEL_0);
    Motor motorLD(GPIO_NUM_10, LEDC_CHANNEL_1);
    Motor motorRU(GPIO_NUM_42, LEDC_CHANNEL_2);
    Motor motorRD(GPIO_NUM_41, LEDC_CHANNEL_3);

    PIDController pitch(0.4f, 0.1f, 0.04f);
    PIDController roll(0.4f, 0.1f, 0.04f);

    float baseThrottle = 65.0f;
    float currentPitch = 0.0f;
    float currentRoll = 0.0f;
    const float time = 0.004f;

    xTaskCreatePinnedToCore(Communication, "Communication", 4096, &context, 5, nullptr, 0);

    while(!context.arm) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (context.arm) {
        IMURawData imuRawData = imu.GetRawData();

        float accPitch = std::atan2(imuRawData.acc_y, imuRawData.acc_z) * (180 / M_PI);
        float accRoll = std::atan2(imuRawData.acc_x, imuRawData.acc_z) * (180 / M_PI);

        float gyrPitch = currentPitch + imuRawData.gyr_x / 16.384f * time;
        float gyrRoll = currentRoll + (-imuRawData.gyr_y / 16.384f) * time;

        currentPitch = 0.98f * gyrPitch + 0.02f * accPitch;
        currentRoll = 0.98f * gyrRoll + 0.02f * accRoll;

        float pitchPidOutput = pitch.Update(0.0f, currentPitch, time);
        float rollPidOutput = roll.Update(0.0f, currentRoll, time);

        float pwrLU = baseThrottle + pitchPidOutput - rollPidOutput;
        float pwrLD = baseThrottle - pitchPidOutput - rollPidOutput;
        float pwrRU = baseThrottle + pitchPidOutput + rollPidOutput;
        float pwrRD = baseThrottle - pitchPidOutput + rollPidOutput;

        motorLU.SetThrottle(pwrLU);
        motorLD.SetThrottle(pwrLD);
        motorRU.SetThrottle(pwrRU);
        motorRD.SetThrottle(pwrRD);

        vTaskDelay(4 / portTICK_PERIOD_MS);
    }

    motorLU.SetThrottle(0.0f);
    motorLD.SetThrottle(0.0f);
    motorRU.SetThrottle(0.0f);
    motorRD.SetThrottle(0.0f);
}
