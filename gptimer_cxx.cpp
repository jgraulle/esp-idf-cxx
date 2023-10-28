#ifdef __cpp_exceptions

#include "gptimer_cxx.hpp"

#include "esp_exception.hpp"
#include <driver/gptimer.h>


namespace idf {

GpTimer::GpTimer(gptimer_count_direction_t direction, uint32_t resolution)
    : _gptimer(nullptr)
    , _state(State::INIT)
    , _callback()
{
    gptimer_config_t timerConfig = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = direction,
        .resolution_hz = resolution,
        .flags = {}
    };
    CHECK_THROW(gptimer_new_timer(&timerConfig, &_gptimer));
}

GpTimer::~GpTimer()
{
    // For all gptimer call, ignore potential error return code to not throw exception.
    switch (_state)
    {
        case State::RUN:
            gptimer_stop(_gptimer);
            [[fallthrough]];
        case State::ENABLE:
            gptimer_disable(_gptimer);
            [[fallthrough]];
        case State::INIT:
            break;
    }
    assert(gptimer_del_timer(_gptimer) == ESP_OK);
    _gptimer = nullptr;
}

void GpTimer::registerEventCallbacks(const EventCallBack & callback)
{
    _callback = callback;
    gptimer_event_callbacks_t cbs = {
        .on_alarm = _callback?static_cast<gptimer_alarm_cb_t>([](gptimer_handle_t, const gptimer_alarm_event_data_t * eventData, void * userCtx) {
            const GpTimer * timer = reinterpret_cast<const GpTimer *>(userCtx);
            return timer->_callback(*timer, *eventData);
        }):nullptr
    };
    CHECK_THROW(gptimer_register_event_callbacks(_gptimer, &cbs, this));
}

void GpTimer::enable()
{
    CHECK_THROW(gptimer_enable(_gptimer));
    _state = State::ENABLE;
}

void GpTimer::disable()
{
    CHECK_THROW(gptimer_disable(_gptimer));
    _state = State::INIT;
}

void GpTimer::start()
{
    CHECK_THROW(gptimer_start(_gptimer));
    _state = State::RUN;
}

void GpTimer::stop()
{
    CHECK_THROW(gptimer_stop(_gptimer));
    _state = State::ENABLE;
}

uint64_t GpTimer::getRawCount() const
{
    decltype(getRawCount()) value;
    CHECK_THROW(gptimer_get_raw_count(_gptimer, &value));
    return value;
}

void GpTimer::setRawCount(uint64_t value)
{
    CHECK_THROW(gptimer_set_raw_count(_gptimer, value));
}

void GpTimer::unsetAlarmAction()
{
    CHECK_THROW(gptimer_set_alarm_action(_gptimer, nullptr));
}

uint32_t GpTimer::getResolution() const
{
    decltype(getResolution()) value;
    CHECK_THROW(gptimer_get_resolution(_gptimer, &value));
    return value;
}

uint64_t GpTimer::getCapturedCount() const
{
    decltype(getCapturedCount()) value;
    CHECK_THROW(gptimer_get_captured_count(_gptimer, &value));
    return value;
}

void GpTimer::setAlarmActionHelper(uint64_t alarmCount, bool isAutoReloadOnAlarm, uint64_t reloadCount)
{
    gptimer_alarm_config_t alarmConfig = {
        .alarm_count = alarmCount,
        .reload_count = reloadCount,
        .flags = {.auto_reload_on_alarm = isAutoReloadOnAlarm?1u:0u}
    };
    CHECK_THROW(gptimer_set_alarm_action(_gptimer, &alarmConfig));
}

} // idf

#endif // __cpp_exceptions
