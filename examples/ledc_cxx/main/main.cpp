/* LED controleur C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "esp_exception.hpp"
#include "ledc_cxx.hpp"
#include "gpio_cxx.hpp"
#include <thread>
#include <iostream>


using namespace std::chrono_literals;
const static char * TAG = "Ledc_Example";

extern "C" void app_main(void)
{
    try
    {
        ESP_LOGI(TAG, "Configure LED controleur");
        idf::LedcTimer ledcTimer0(LEDC_TIMER_0, 440, LEDC_LOW_SPEED_MODE, LEDC_TIMER_13_BIT);
        idf::LedcChannel ledcChannel0(LEDC_CHANNEL_0, ledcTimer0, idf::GPIONum(16), 1<<12);

        // play 440 Hz for 1s
        ESP_LOGI(TAG, "Play a 440Hz sound for 1s");
        std::this_thread::sleep_for(1s);

        // stop for 1s
        ESP_LOGI(TAG, "Pause for 1s");
        ledcTimer0.pause();
        std::this_thread::sleep_for(1s);

        // play 880 Hz for 1s
        ESP_LOGI(TAG, "Play a 880Hz sound for 1s");
        ledcTimer0.setFreq(880); // Set frequency for 880Hz
        ledcTimer0.resume();
        std::this_thread::sleep_for(1s);

        // stop for 1s
        ESP_LOGI(TAG, "Pause");

        while (true)
        {
            std::this_thread::sleep_for(1s);
        }
    }
    catch (idf::ESPException & e)
    {
        std::cout << "Exception occurred: " << esp_err_to_name(e.error) << std::endl
                 << "stopping." << std::endl;
    }
}
