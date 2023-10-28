#ifdef __cpp_exceptions

#include "pulse_counter_cxx.hpp"

#include "esp_exception.hpp"

namespace idf {

PulseCounter::PulseCounter(int lowLimit, int highLimit, bool accumCount)
    : _unit(nullptr)
    , _callback()
    , _state(State::INIT)
{
    pcnt_unit_config_t config = {
        .low_limit = lowLimit,
        .high_limit = highLimit,
        .flags = {.accum_count = accumCount?1u:0u}
    };
    CHECK_THROW(pcnt_new_unit(&config, &_unit));
}

PulseCounter::~PulseCounter()
{
    // For all gptimer call, ignore potential error return code to not throw exception.
    switch (_state)
    {
        case State::RUN:
            pcnt_unit_stop(_unit);
            [[fallthrough]];
        case State::ENABLE:
            pcnt_unit_disable(_unit);
            [[fallthrough]];
        case State::INIT:
            break;
    }
    assert(pcnt_del_unit(_unit) == ESP_OK);
    _unit = nullptr;
}

PulseCounter::Channel::Channel(const PulseCounter & pulseCounter, std::optional<GPIONum> edge, std::optional<GPIONum> level)
{
    pcnt_chan_config_t config = {
        .edge_gpio_num = edge.has_value()?static_cast<int>(edge.value().get_value()):-1,
        .level_gpio_num = level.has_value()?static_cast<int>(level.value().get_value()):-1,
        .flags {}
    };
    CHECK_THROW(pcnt_new_channel(pulseCounter._unit, &config, &_channel));
}

PulseCounter::Channel::~Channel()
{
    assert(pcnt_del_channel(_channel) == ESP_OK);
}

void PulseCounter::Channel::setEdgeChannelAction(pcnt_channel_edge_action_t posedgeAction,
        pcnt_channel_edge_action_t negedgeAction)
{
    CHECK_THROW(pcnt_channel_set_edge_action(_channel, posedgeAction, negedgeAction));
}

void PulseCounter::Channel::setLevelChannelAction(pcnt_channel_level_action_t hightAction,
        pcnt_channel_level_action_t lowAction)
{
    CHECK_THROW(pcnt_channel_set_level_action(_channel, hightAction, lowAction));
}

void PulseCounter::addWatchPoints(int watchPoint)
{
    CHECK_THROW(pcnt_unit_add_watch_point(_unit, watchPoint));
    CHECK_THROW(pcnt_unit_clear_count(_unit));
}

void PulseCounter::removeWatchPoints(int watchPoint)
{
    CHECK_THROW(pcnt_unit_remove_watch_point(_unit, watchPoint));
}

void PulseCounter::registerEventCallbacks(const EventCallBack & callback)
{
    _callback = callback;
    pcnt_event_callbacks_t callbacks = {
        .on_reach = [](pcnt_unit_handle_t unit, const pcnt_watch_event_data_t * edata, void * userCtx) {
            const PulseCounter * pulseCounter = reinterpret_cast<const PulseCounter *>(userCtx);
            return pulseCounter->_callback(*pulseCounter, *edata);
        }
    };
    CHECK_THROW(pcnt_unit_register_event_callbacks(_unit, &callbacks, this));
}

void PulseCounter::unregisterEventCallbacks()
{
    CHECK_THROW(pcnt_unit_register_event_callbacks(_unit, nullptr, nullptr));
}

void PulseCounter::setGlitchFilter(uint32_t maxGlitchNs)
{
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = maxGlitchNs,
    };
    CHECK_THROW(pcnt_unit_set_glitch_filter(_unit, maxGlitchNs==0?nullptr:&filter_config));
}

void PulseCounter::enable()
{
    CHECK_THROW(pcnt_unit_enable(_unit));
    _state = State::ENABLE;
}

void PulseCounter::disable()
{
    CHECK_THROW(pcnt_unit_disable(_unit));
    _state = State::INIT;
}

void PulseCounter::start()
{
    CHECK_THROW(pcnt_unit_start(_unit));
    _state = State::RUN;
}

void PulseCounter::stop()
{
    CHECK_THROW(pcnt_unit_stop(_unit));
    _state = State::ENABLE;
}

int PulseCounter::getCount() const
{
    int value = 0;
    CHECK_THROW(pcnt_unit_get_count(_unit, &value));
    return value;
}

void PulseCounter::clearCount()
{
    CHECK_THROW(pcnt_unit_clear_count(_unit));
}

} // idf

#endif // __cpp_exceptions
