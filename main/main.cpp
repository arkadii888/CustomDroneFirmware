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

    PIDController pitch(1.5f, 0.0f, 0.02f);
    PIDController roll(1.5f, 0.0f, 0.02f);
    PIDController yaw(2.5f, 0.0f, 0.0f);

    float currentPitch = 0.0f;
    float currentRoll = 0.0f;
    float currentYaw = 0.0f;
    float baseThrottle = 30.0f;
    float desiredPitch = 0.0f;
    float desiredRoll = 0.0f;
    float desiredYaw = 0.0f;
    float time = 0.01;

    xTaskCreatePinnedToCore(Communication, "Communication", 4096, &context, 5, nullptr, 0);

    while(!context.arm) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (context.arm) {
        IMUData imuData = imu.GetData();

        float accPitch = atan2(-imuData.acc_x, sqrt(imuData.acc_y * imuData.acc_y + imuData.acc_z * imuData.acc_z)) * (180.0f / M_PI);
        float accRoll = atan2(imuData.acc_y, imuData.acc_z) * (180.0f / M_PI);

        float gyroRateX = imuData.gyr_x / 131.0f;
        float gyroRateY = imuData.gyr_y / 131.0f;
        float gyroRateZ = imuData.gyr_z / 131.0f;

        currentPitch = 0.98f * (currentPitch + gyroRateY *  time) + 0.02f * accPitch;
        currentRoll = 0.98f * (currentRoll + gyroRateX * time) + 0.02f * accRoll;
        currentYaw += gyroRateZ * time;

        float pPidOutput = pitch.Update(desiredPitch, currentPitch, time);
        float rPidOutput = roll.Update(desiredRoll, currentRoll, time);
        float yPidOutput = yaw.Update(desiredYaw, currentYaw, time);

        float pwrLU = baseThrottle + pPidOutput + rPidOutput - yPidOutput;
        float pwrLD = baseThrottle - pPidOutput + rPidOutput + yPidOutput;
        float pwrRU = baseThrottle + pPidOutput - rPidOutput + yPidOutput;
        float pwrRD = baseThrottle - pPidOutput - rPidOutput - yPidOutput;

        motorLU.SetThrottle(std::clamp(pwrLU, 0.0f, 100.0f));
        motorLD.SetThrottle(std::clamp(pwrLD, 0.0f, 100.0f));
        motorRU.SetThrottle(std::clamp(pwrRU, 0.0f, 100.0f));
        motorRD.SetThrottle(std::clamp(pwrRD, 0.0f, 100.0f));

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    motorLU.SetThrottle(0.0f);
    motorLD.SetThrottle(0.0f);
    motorRU.SetThrottle(0.0f);
    motorRD.SetThrottle(0.0f);
}
