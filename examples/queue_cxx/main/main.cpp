/* Queue C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "gpio_cxx.hpp"
#include "gptimer_cxx.hpp"
#include <thread>
#include <cstdlib>
#include "queue_cxx.hpp"


extern "C" void app_main(void)
{
    idf::Queue<bool> queue(10);

    try {
        idf::GpTimer timer(GPTIMER_COUNT_UP, 1000000u); // Set Frequency at 1Mhz => 1 unit is 1us
        timer.setAlarmAction(100000u, 0u); // Set period at 100ms
        idf::GPIO_Output ledGpio(idf::GPIONum(26));
        bool ledState = false;
        timer.registerEventCallbacks([&ledState, &ledGpio, &queue](const idf::GpTimer &, const gptimer_alarm_event_data_t &){
            ledState = !ledState;
            bool higherPriorityTaskWokenByPost = false;
            bool ret = queue.sendFromISR(ledState, higherPriorityTaskWokenByPost);
            assert(ret);
            if (ledState)
                ledGpio.set_high();
            else
                ledGpio.set_low();
            return higherPriorityTaskWokenByPost;
        });
        timer.enable();
        timer.start();

        // Main loop
        while(true) {
            auto responce = queue.receive(portTICK_PERIOD_MS*1000);
            if (responce.has_value()) {
                if (responce.value())
                    printf("LED ON\n");
                else
                    printf("LED OFF\n");
            }
        }
    } catch (idf::GPIOException &e) {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
}
