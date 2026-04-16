#include <iostream>
#include <sys/types.h>
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

struct CommunicationContext {
    VoltageMonitor* voltageMonitor = nullptr;
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

    Motor motorLU(GPIO_NUM_5, LEDC_CHANNEL_0);
    Motor motorLD(GPIO_NUM_10, LEDC_CHANNEL_1);
    Motor motorRU(GPIO_NUM_42, LEDC_CHANNEL_2);
    Motor motorRD(GPIO_NUM_41, LEDC_CHANNEL_3);

    // Communication Thread
    xTaskCreatePinnedToCore(Communication, "Communication", 4096, &context, 5, nullptr, 0);

    // Flight Thread
    while (true) {
        //IMUData imuData = imu.GetData();

        //motorLU.SetThrottle(10.0f);
        //motorLD.SetThrottle(10.0f);
        //motorRU.SetThrottle(10.0f);
        //motorRD.SetThrottle(10.0f);

        //std::cout << "VoltageMonitor Voltage: " << voltageMonitor.GetVoltage() << " V" << std::endl;
        //std::cout << "VoltageMonitor Current: " << voltageMonitor.GetCurrent() << " A" << std::endl;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
