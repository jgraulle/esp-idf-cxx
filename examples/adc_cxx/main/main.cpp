/* DAC C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "esp_exception.hpp"
#include "dac_cxx.hpp"
#include "adc_cxx.hpp"
#include <thread>

const static char * TAG = "DAC-ADC_Example";


extern "C" void app_main(void)
{
    try {
        ESP_LOGI(TAG, "Configure DAC");
        idf::DacOneshot dac(DAC_CHAN_1);

        ESP_LOGI(TAG, "Configure ADC");
        auto [unit, channel] = idf::AdcOneshot::ioToChannel(34);
        idf::AdcOneshot adcUnit(unit);
        adcUnit.configure(channel, ADC_ATTEN_DB_11, ADC_BITWIDTH_10);
        uint8_t dacRaw = 5;
        while (true) {
            ESP_LOGI(TAG, "Set voltage to %i => %imV", static_cast<int>(dacRaw), static_cast<int>(dacRaw)*3300/256);
            dac.setVoltage(dacRaw);
            dacRaw+=10;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            int valueRaw = adcUnit.read(channel);
            int valuemV = valueRaw * 3300 / ((1<<10)-1);
            ESP_LOGI(TAG, "Read raw value: %i => %imV", valueRaw, valuemV);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }
    } catch (idf::ESPException & e) {
        ESP_LOGE(TAG, "Exception occurred: %s", esp_err_to_name(e.error));
        ESP_LOGE(TAG, "stopping.");
    }
}
