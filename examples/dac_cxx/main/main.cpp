/* DAC C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "esp_exception.hpp"
#include "dac_cxx.hpp"
#include <thread>


const static char * TAG = "DAC_Example";

extern "C" void app_main(void)
{
    try {
        ESP_LOGI(TAG, "Configure DAC");
        idf::DacOneshot dac(DAC_CHAN_1);
        ESP_LOGI(TAG, "Set voltage to 1.65V");
        dac.setVoltage(128);
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    } catch (idf::ESPException & e) {
        ESP_LOGE(TAG, "Exception occurred: %s", esp_err_to_name(e.error));
        ESP_LOGE(TAG, "stopping.");
    }
}
