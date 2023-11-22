#include "ledc_cxx.hpp"

#include "esp_exception.hpp"
#include "gpio_cxx.hpp"


namespace idf {

LedcTimer::LedcTimer(ledc_timer_t timerId, uint32_t freq, ledc_mode_t speedMode,
        ledc_timer_bit_t dutyBitWidth, ledc_clk_cfg_t clkCfg)
    : _timerId(timerId)
    , _speedMode(speedMode)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = speedMode,
        .duty_resolution = dutyBitWidth,
        .timer_num = timerId,
        .freq_hz = freq,
        .clk_cfg = clkCfg
    };
    CHECK_THROW(ledc_timer_config(&ledc_timer));
}

LedcTimer::~LedcTimer()
{
    assert(ledc_timer_rst(_speedMode, _timerId)==ESP_OK);
}

void LedcTimer::setFreq(uint32_t freq)
{
    CHECK_THROW(ledc_set_freq(_speedMode, _timerId, freq));
}

uint32_t LedcTimer::getFreq() const
{
    uint32_t freq = ledc_get_freq(_speedMode, _timerId);
    if (freq == 0)
        throw ESPException(ESP_FAIL);
    return freq;
}

void LedcTimer::pause()
{
    CHECK_THROW(ledc_timer_pause(_speedMode, _timerId));
}

void LedcTimer::resume()
{
    CHECK_THROW(ledc_timer_resume(_speedMode, _timerId));
}

LedcChannel::LedcChannel(ledc_channel_t channelId, const LedcTimer & timer, GPIONum gpioNum, uint32_t duty)
    : _channelId(channelId)
    , _timer(timer)
{
    ledc_channel_config_t ledc_channel = {
        .gpio_num = static_cast<int>(gpioNum.get_value()),
        .speed_mode = timer.getSpeedMode(),
        .channel = channelId,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer.getTimerId(),
        .duty = duty,
        .hpoint = 0,
        .flags = {
            .output_invert = 0,
        }
    };
    CHECK_THROW(ledc_channel_config(&ledc_channel));
}

LedcChannel::~LedcChannel()
{
    assert(ledc_stop(_timer.getSpeedMode(), _channelId, 0) == ESP_OK);
}

void LedcChannel::setDutyWithHpoint(uint32_t duty, uint32_t hpoint)
{
    CHECK_THROW(ledc_set_duty_with_hpoint(_timer.getSpeedMode(), _channelId, duty, hpoint));
}

int LedcChannel::getHpoint() const
{
    int hpoint = ledc_get_hpoint(_timer.getSpeedMode(), _channelId);
    if (hpoint == LEDC_ERR_VAL)
        throw ESPException(ESP_ERR_INVALID_ARG);
    return hpoint;
}

void LedcChannel::setDuty(uint32_t duty)
{
    CHECK_THROW(ledc_set_duty(_timer.getSpeedMode(), _channelId, duty));
}

uint32_t LedcChannel::getDuty() const
{
    uint32_t duty = ledc_get_duty(_timer.getSpeedMode(), _channelId);
    if (duty == LEDC_ERR_DUTY)
        throw ESPException(ESP_ERR_INVALID_ARG);
    return duty;
}

void LedcChannel::updateDuty()
{
    CHECK_THROW(ledc_update_duty(_timer.getSpeedMode(), _channelId));
}

void LedcChannel::stop(bool idleLevel)
{
    CHECK_THROW(ledc_stop(_timer.getSpeedMode(), _channelId, idleLevel?0:1));
}

}
