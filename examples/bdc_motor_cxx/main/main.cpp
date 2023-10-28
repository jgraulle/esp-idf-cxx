/* Brushed DC motor C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "bdc_motor_cxx.hpp"
#include "esp_exception.hpp"
#include <thread>

#define TAG "BdcMotorTest"

extern "C" void app_main(void)
{
    try {
        idf::BdcMotor motor(idf::GPIONum(25), idf::GPIONum(26), 25000, 0, 10000000);
        motor.enable();
        motor.setPower(100);

        ESP_LOGI(TAG, "Forward");
        motor.forward();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ESP_LOGI(TAG, "Reverse");
        motor.reverse();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ESP_LOGI(TAG, "Coast");
        motor.coast();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ESP_LOGI(TAG, "Break");
        motor.brake();
        std::this_thread::sleep_for(std::chrono::seconds(2));

        ESP_LOGI(TAG, "Forward");
        motor.reverse();
        uint32_t power = 0;
        bool direction = true;
        while (true) {
            if (direction)
            {
                power+=10;
                if (power == 100)
                    direction = false;
            }
            else
            {
                power-=10;
                if (power == 0)
                    direction = true;
            }
            motor.setPower(power);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (idf::ESPException & e) {
        ESP_LOGE(TAG, "GPIO exception occurred: %s", esp_err_to_name(e.error));
        ESP_LOGE(TAG, "stopping.");
    }
}
