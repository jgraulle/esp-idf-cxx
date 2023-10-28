/* PulseCounter C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <cstdlib>
#include "queue_cxx.hpp"
#include "pulse_counter_cxx.hpp"


extern "C" void app_main(void)
{
    try {
        idf::Queue<int> pulseCounterQueue(10);
        idf::PulseCounter pulseCounter(-10, 10, true);
        pulseCounter.addWatchPoints(-10);
        pulseCounter.addWatchPoints(10);
        idf::PulseCounter::Channel channel(pulseCounter, idf::GPIONum(25), {});
        channel.setEdgeChannelAction(PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
        pulseCounter.registerEventCallbacks([&pulseCounterQueue](const idf::PulseCounter & pulseCounter, const pcnt_watch_event_data_t &){
            bool higherPriorityTaskWoken;
            pulseCounterQueue.sendFromISR(pulseCounter.getCount(), higherPriorityTaskWoken);
            return higherPriorityTaskWoken;
        });
        pulseCounter.setGlitchFilter(1000);
        pulseCounter.enable();
        pulseCounter.start();

        // Main loop
        while(true) {
            auto pulseCounterQueueResponce = pulseCounterQueue.receive(1000 / portTICK_PERIOD_MS);
            if (pulseCounterQueueResponce.has_value()) {
                printf("interruption pulse counter = %i\n", pulseCounterQueueResponce.value());
            }
            printf("pulse counter = %i\n", pulseCounter.getCount());
        }
    } catch (idf::GPIOException &e) {
        printf("GPIO exception occurred: %s\n", esp_err_to_name(e.error));
        printf("stopping.\n");
    }
}
