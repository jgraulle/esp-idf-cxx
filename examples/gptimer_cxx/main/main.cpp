/* gptimer C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "gpio_cxx.hpp"
#include "gptimer_cxx.hpp"
#include <thread>
#include <cstdlib>


extern "C" void app_main(void)
{
    try {
        idf::GpTimer timer(GPTIMER_COUNT_UP, 1000000u); // Set Frequency at 1Mhz => 1 unit is 1us
        timer.setAlarmAction(100000u, 0u); // Set period at 100ms
        idf::GPIO_Output ledGpio(idf::GPIONum(26));
        bool ledState = false;
        timer.registerEventCallbacks([&ledState, &ledGpio](const idf::GpTimer &, const gptimer_alarm_event_data_t &){
            ledState = !ledState;
            if (ledState)
                ledGpio.set_high();
            else
                ledGpio.set_low();
            return false;
        });
        timer.enable();
        timer.start();

        // Main loop
        while(true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (idf::GPIOException &e) {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
}
