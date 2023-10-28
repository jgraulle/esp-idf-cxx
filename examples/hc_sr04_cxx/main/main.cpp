/* HC_SR04 C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"
#include "esp_exception.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_private/esp_clk.h"
#include "gpio_cxx.hpp"
#include "mcpwm_cxx.hpp"
#include "queue_cxx.hpp"
#include <thread>
#include <utility>

const static char * TAG = "hcSr04Example";

#define HC_SR04_TRIG_GPIO 33u
#define HC_SR04_ECHO_GPIO 32u

class HcSr04
{
public:
    HcSr04(const idf::GPIONum & trigGpio, const idf::GPIONum & echoGpio)
        : _trigGpio(idf::GPIONum(trigGpio))
        , _capTimer(0)
        , _capChannel{_capTimer, echoGpio, 1, true, true, false, false, false, false, false}
        , _rspQueue(10u)
        , _lastCapValue(0u, 0u)
    {
        _capChannel.registerEventCallbacks([this](const idf::mcpwm::CaptureChannel &, const mcpwm_capture_event_data_t & eventData){
            return this->callback(eventData);
        });
    }

    ~HcSr04() {
        _capTimer.stop();
        _capTimer.disable();
        _capChannel.disable();
        _trigGpio.set_low();
    }

    void enable()
    {
        // drive low by default
        _trigGpio.set_low();

        _capChannel.enable();
        _capTimer.enable();
    }

    void start()
    {
        _capTimer.start();
    }

    /**
     * @brief generate single pulse on Trig pin to start a new sample
     */
    void readRequest()
    {
        _trigGpio.set_high();
        std::this_thread::sleep_for(10us);
        _trigGpio.set_low();
    }

    std::optional<float> receive(TickType_t ticksToWait)
    {
        auto rsp = _rspQueue.receive(ticksToWait);
        if (rsp.has_value()) {
            float pulseWidthUs = rsp.value() * (1000000.0 / _capTimer.getResolution());
            // if not out of range
            if (pulseWidthUs <= 35000) {
                // convert the pulse width into measure distance
                return pulseWidthUs / 58.0;
            }
        }
        return {};
    }
private:
    bool callback(const mcpwm_capture_event_data_t & eventData) {
        bool highTaskWakeup = false;

        //calculate the interval in the ISR,
        //so that the interval will be always correct even when capture_queue is not handled in time and overflow.
        if (eventData.cap_edge == MCPWM_CAP_EDGE_POS) {
            // store the timestamp when pos edge is detected
            _lastCapValue.first = eventData.cap_value;
            _lastCapValue.second = _lastCapValue.first;
        } else {
            _lastCapValue.second = eventData.cap_value;
            uint32_t tofTicks = _lastCapValue.second - _lastCapValue.first;

            // notify the task to calculate the distance
            _rspQueue.sendFromISR(tofTicks, highTaskWakeup);
        }

        return highTaskWakeup;
    }

    HcSr04(const HcSr04 &) = delete;
    HcSr04 & operator=(const HcSr04 &) = delete;

    idf::GPIO_Output _trigGpio;
    idf::mcpwm::CaptureTimer _capTimer;
    idf::mcpwm::CaptureChannel _capChannel;
    idf::Queue<uint32_t> _rspQueue;
    std::pair<uint32_t,uint32_t> _lastCapValue;
};

extern "C" void app_main(void)
{
    try {
        HcSr04 hcSr04(idf::GPIONum(HC_SR04_TRIG_GPIO), idf::GPIONum(HC_SR04_ECHO_GPIO));
        hcSr04.enable();
        hcSr04.start();

        while (true) {
            // trigger the sensor to start a new sample
            ESP_LOGI(TAG, "Trig output");
            hcSr04.readRequest();
            // wait for echo done signal
            auto rsp = hcSr04.receive(pdMS_TO_TICKS(1000));
            if (rsp.has_value()) {
                ESP_LOGI(TAG, "Measured distance: %.2fcm", rsp.value());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    } catch (idf::ESPException & e) {
        ESP_LOGE(TAG, "GPIO exception occurred: %s", esp_err_to_name(e.error));
        ESP_LOGE(TAG, "stopping.");
    }
}
