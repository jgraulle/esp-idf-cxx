#if __cpp_exceptions

#include "mcpwm_cxx.hpp"

#include <driver/mcpwm_timer.h>
#include <driver/mcpwm_oper.h>
#include <driver/mcpwm_cmpr.h>
#include <driver/mcpwm_gen.h>
#include <driver/mcpwm_fault.h>
#include <driver/mcpwm_sync.h>
#include <driver/mcpwm_cap.h>

namespace idf {

namespace mcpwm {

SyncSrc::~SyncSrc()
{
    assert(mcpwm_del_sync_src(_handle) == ESP_OK);
    _handle = nullptr;
}

GpioSyncSrc::GpioSyncSrc(int groupId, GPIONum gpioNum, bool activeNeg, bool ioLoopBack, bool pullUp, bool pullDown)
{
    mcpwm_gpio_sync_src_config_t config = {
        .group_id = groupId,
        .gpio_num = static_cast<int>(gpioNum.get_value()),
        .flags = {
            .active_neg = activeNeg?1u:0u,
            .io_loop_back = ioLoopBack?1u:0u,
            .pull_up = pullUp?1u:0u,
            .pull_down = pullDown?1u:0u
        }
    };
    CHECK_THROW(mcpwm_new_gpio_sync_src(&config, &getHandle()));
}

TimerSyncSrc::TimerSyncSrc(const Timer & timer, mcpwm_timer_event_t timerEvent, bool propagateInputSync)
{
    mcpwm_timer_sync_src_config_t config = {
        .timer_event = timerEvent,
        .flags = {
            .propagate_input_sync = propagateInputSync?1u:0u
        }
    };
    CHECK_THROW(mcpwm_new_timer_sync_src(timer._handle, &config, &getHandle()));
}

SoftSyncSrc::SoftSyncSrc()
{
    mcpwm_soft_sync_config_t config;
    CHECK_THROW(mcpwm_new_soft_sync_src(&config, &getHandle()));
}

void SoftSyncSrc::activate()
{
    CHECK_THROW(mcpwm_soft_sync_activate(getHandle()));
}

Timer::Timer(int groupId, uint32_t resolutionHz, mcpwm_timer_count_mode_t countMode, uint32_t periodTicks, bool updatePeriodOnEmpty, bool updatePeriodOnSync)
    : _handle(nullptr)
    , _onFull()
    , _onEmpty()
    , _onStop()
    , _isEnable(false)
{
    mcpwm_timer_config_t config {
        .group_id = groupId,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = resolutionHz,
        .count_mode = countMode,
        .period_ticks = periodTicks,
        .flags = {
            .update_period_on_empty = updatePeriodOnEmpty?1u:0u,
            .update_period_on_sync = updatePeriodOnSync?1u:0u
        }
    };
    CHECK_THROW(mcpwm_new_timer(&config, &_handle));
}

Timer::~Timer()
{
    if (_isEnable)
    {
        assert(mcpwm_timer_disable(_handle) == ESP_OK);
        _isEnable = false;
    }
    assert(mcpwm_del_timer(_handle) == ESP_OK);
    _handle = nullptr;
}

void Timer::registerEventCallBacks(const EventCallBack & onFull, const EventCallBack & onEmpty,
        const EventCallBack & onStop)
{
    _onFull = onFull;
    _onEmpty = onEmpty;
    _onStop = onStop;
    mcpwm_timer_event_callbacks_t cbs = {
        .on_full = _onFull?static_cast<mcpwm_timer_event_cb_t>([](mcpwm_timer_handle_t, const mcpwm_timer_event_data_t * edata, void * userCtx) {
            const Timer * timer = reinterpret_cast<const Timer *>(userCtx);
            return timer->_onFull(*timer, *edata);
        }):nullptr,
        .on_empty = _onEmpty?static_cast<mcpwm_timer_event_cb_t>([](mcpwm_timer_handle_t, const mcpwm_timer_event_data_t * edata, void * userCtx) {
            const Timer * timer = reinterpret_cast<const Timer *>(userCtx);
            return timer->_onEmpty(*timer, *edata);
        }):nullptr,
        .on_stop = _onStop?static_cast<mcpwm_timer_event_cb_t>([](mcpwm_timer_handle_t, const mcpwm_timer_event_data_t * edata, void * userCtx) {
            const Timer * timer = reinterpret_cast<const Timer *>(userCtx);
            return timer->_onStop(*timer, *edata);
        }):nullptr
    };
    CHECK_THROW(mcpwm_timer_register_event_callbacks(_handle, &cbs, this));
}

void Timer::enable()
{
    CHECK_THROW(mcpwm_timer_enable(_handle));
    _isEnable = true;
}

void Timer::disable()
{
    CHECK_THROW(mcpwm_timer_disable(_handle));
    _isEnable = false;
}

void Timer::start_stop(mcpwm_timer_start_stop_cmd_t command)
{
    CHECK_THROW(mcpwm_timer_start_stop(_handle, command));
}

void Timer::setPhaseOnSync(const std::optional<SyncSrc> & syncSrc, uint32_t countValue, mcpwm_timer_direction_t direction)
{
    mcpwm_timer_sync_phase_config_t config {
        .sync_src = syncSrc.has_value()?syncSrc.value()._handle:nullptr,
        .count_value = countValue,
        .direction = direction
    };
    CHECK_THROW(mcpwm_timer_set_phase_on_sync(_handle, &config));
}

Fault::~Fault()
{
    assert(mcpwm_del_fault(_handle) == ESP_OK);
    _handle = nullptr;
}

void Fault::registerEventCallbacks(const EventCallBack & onFaultEnter, const EventCallBack & onFaultExit)
{
    _onFaultEnter = onFaultEnter;
    _onFaultExit = onFaultExit;
    mcpwm_fault_event_callbacks_t cbs {
        .on_fault_enter = _onFaultEnter?static_cast<mcpwm_fault_event_cb_t>([](mcpwm_fault_handle_t, const mcpwm_fault_event_data_t * edata, void * userCtx) {
            const Fault * fault = reinterpret_cast<const Fault *>(userCtx);
            return fault->_onFaultEnter(*fault, *edata);
        }):nullptr,
        .on_fault_exit = _onFaultExit?static_cast<mcpwm_fault_event_cb_t>([](mcpwm_fault_handle_t, const mcpwm_fault_event_data_t * edata, void * userCtx) {
            const Fault * fault = reinterpret_cast<const Fault *>(userCtx);
            return fault->_onFaultExit(*fault, *edata);
        }):nullptr
    };
    CHECK_THROW(mcpwm_fault_register_event_callbacks(_handle, &cbs, this));
}

GpioFault::GpioFault(int groupId, GPIONum gpioNum, bool activeLevel, bool ioLoopBack, bool pullUp, bool pullDown)
{
    mcpwm_gpio_fault_config_t config = {
        .group_id = groupId,
        .gpio_num = static_cast<int>(gpioNum.get_value()),
        .flags = {
            .active_level = activeLevel?1u:0u,
            .io_loop_back = ioLoopBack?1u:0u,
            .pull_up = pullUp?1u:0u,
            .pull_down = pullDown?1u:0u
        }
    };
    CHECK_THROW(mcpwm_new_gpio_fault(&config, &getHandle()));
}

SoftFault::SoftFault()
{
    mcpwm_soft_fault_config_t config;
    CHECK_THROW(mcpwm_new_soft_fault(&config, &getHandle()));
}

void SoftFault::activate()
{
    CHECK_THROW(mcpwm_soft_fault_activate(getHandle()));
}

Operator::Operator(int groupId, UpdateOnFlags updateGenAction, UpdateOnFlags updateDeadTime)
    : _handle(nullptr)
{
    mcpwm_operator_config_t config {
        .group_id = groupId,
        .flags = {
            .update_gen_action_on_tez = updateGenAction.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_ZERO),
            .update_gen_action_on_tep = updateGenAction.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_PEAK),
            .update_gen_action_on_sync = updateGenAction.isSet0uOr1u(UpdateOn::SYNC),
            .update_dead_time_on_tez = updateDeadTime.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_ZERO),
            .update_dead_time_on_tep = updateDeadTime.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_PEAK),
            .update_dead_time_on_sync = updateDeadTime.isSet0uOr1u(UpdateOn::SYNC)
        }
    };
    CHECK_THROW(mcpwm_new_operator(&config, &_handle));
}

Operator::~Operator()
{
    assert(mcpwm_del_operator(_handle) == ESP_OK);
    _handle = nullptr;
}

void Operator::connect(const Timer & timer)
{
    CHECK_THROW(mcpwm_operator_connect_timer(_handle, timer._handle));
}

void Operator::applyCarrier(uint32_t frequencyHz, uint32_t firstPulseDurationUs, float dutyCycle, bool invertBeforeModulate, bool invertAfterModulate)
{
    mcpwm_carrier_config_t config {
        .frequency_hz = frequencyHz,
        .first_pulse_duration_us = firstPulseDurationUs,
        .duty_cycle = dutyCycle,
        .flags = {
            .invert_before_modulate = invertBeforeModulate?1u:0u,
            .invert_after_modulate = invertAfterModulate?1u:0u
        }
    };
    CHECK_THROW(mcpwm_operator_apply_carrier(_handle, &config));
}

void Operator::removeCarrier()
{
    CHECK_THROW(mcpwm_operator_apply_carrier(_handle, nullptr));
}

void Operator::setBrakeOnFault(const Fault & fault, mcpwm_operator_brake_mode_t brakeMode, bool cbcRecoverOnTez, bool cbcRecoverOnTep)
{
    mcpwm_brake_config_t config {
        .fault = fault._handle,
        .brake_mode = brakeMode,
        .flags = {
            .cbc_recover_on_tez = cbcRecoverOnTez?1u:0u,
            .cbc_recover_on_tep = cbcRecoverOnTep?1u:0u
        }
    };
    CHECK_THROW(mcpwm_operator_set_brake_on_fault(_handle, &config));
}

void Operator::recoverFromFault(const Fault & fault)
{
    CHECK_THROW(mcpwm_operator_recover_from_fault(_handle, fault._handle));
}

void Operator::registerEventCallbacks(const EventCallBack & onBrakeCbc, const EventCallBack & onBrakeOst)
{
    _onBrakeCbc = onBrakeCbc;
    _onBrakeOst = onBrakeOst;
    mcpwm_operator_event_callbacks_t cbs {
        .on_brake_cbc = _onBrakeCbc?static_cast<mcpwm_brake_event_cb_t>([](mcpwm_oper_handle_t, const mcpwm_brake_event_data_t * edata, void * userCtx) {
            const Operator * operat = reinterpret_cast<const Operator *>(userCtx);
            return operat->_onBrakeCbc(*operat, *edata);
        }):nullptr,
        .on_brake_ost = _onBrakeOst?static_cast<mcpwm_brake_event_cb_t>([](mcpwm_oper_handle_t, const mcpwm_brake_event_data_t * edata, void * userCtx) {
            const Operator * operat = reinterpret_cast<const Operator *>(userCtx);
            return operat->_onBrakeOst(*operat, *edata);
        }):nullptr
    };
    CHECK_THROW(mcpwm_operator_register_event_callbacks(_handle, &cbs, this));
}

Comparators::Comparators(const Operator & operat, UpdateOnFlags flags)
    : _handle(nullptr)
{
    mcpwm_comparator_config_t config {
        .flags {
            .update_cmp_on_tez = flags.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_ZERO),
            .update_cmp_on_tep = flags.isSet0uOr1u(UpdateOn::TIMER_COUNT_TO_PEAK),
            .update_cmp_on_sync = flags.isSet0uOr1u(UpdateOn::SYNC)
        }
    };
    CHECK_THROW(mcpwm_new_comparator(operat._handle, &config, &_handle));
}

Comparators::~Comparators()
{
    assert(mcpwm_del_comparator(_handle) == ESP_OK);
    _handle = nullptr;
}

void Comparators::registerEventCallBacks(const EventCallBack & callback)
{
    _callback = callback;
    mcpwm_comparator_event_callbacks_t cbs = {
        .on_reach = _callback?static_cast<mcpwm_compare_event_cb_t>([](mcpwm_cmpr_handle_t, const mcpwm_compare_event_data_t * edata, void * userCtx) {
            const Comparators * comparators = reinterpret_cast<const Comparators *>(userCtx);
            return comparators->_callback(*comparators, *edata);
        }):nullptr
    };
    CHECK_THROW(mcpwm_comparator_register_event_callbacks(_handle, &cbs, this));
}

void Comparators::setCompareValue(uint32_t cmpTicks)
{
    CHECK_THROW(mcpwm_comparator_set_compare_value(_handle, cmpTicks));
}

Generators::Generators(const Operator & operat, GPIONum gpioNum, bool invertPwm, bool ioLoopBack)
    : _handle(nullptr)
{
    mcpwm_generator_config_t config {
        .gen_gpio_num = static_cast<int>(gpioNum.get_value()),
        .flags = {
            .invert_pwm = invertPwm?1u:0u,
            .io_loop_back = ioLoopBack?1u:0u
        }
    };
    CHECK_THROW(mcpwm_new_generator(operat._handle, &config, &_handle));
}

Generators::~Generators()
{
    assert(mcpwm_del_generator(_handle) == ESP_OK);
    _handle = nullptr;
}

void Generators::setActionOnTimerEvent(mcpwm_timer_direction_t direction, mcpwm_timer_event_t event, mcpwm_generator_action_t action)
{
    mcpwm_gen_timer_event_action_t eventAction {
        .direction = direction,
        .event = event,
        .action = action
    };
    CHECK_THROW(mcpwm_generator_set_action_on_timer_event(_handle, eventAction));
}

void Generators::setActionOnCompareEvent(mcpwm_timer_direction_t direction, const Comparators & comparator, mcpwm_generator_action_t action)
{
    mcpwm_gen_compare_event_action_t eventAction {
        .direction = direction,
        .comparator = comparator._handle,
        .action = action
    };
    CHECK_THROW(mcpwm_generator_set_action_on_compare_event(_handle, eventAction));
}

void Generators::setDeadTime(uint32_t posedgeDelayTicks, uint32_t negedgeDelayTicks, bool invertOutput)
{
    mcpwm_dead_time_config_t config {
        .posedge_delay_ticks = posedgeDelayTicks,
        .negedge_delay_ticks = negedgeDelayTicks,
        .flags= {
            .invert_output = invertOutput?1u:0u
        }
    };
    CHECK_THROW(mcpwm_generator_set_dead_time(_handle, _handle, &config));
}

void Generators::setActionOnBrakeEvent(mcpwm_timer_direction_t direction, mcpwm_operator_brake_mode_t brakeMode, mcpwm_generator_action_t action)
{
    mcpwm_gen_brake_event_action_t eventAction {
        .direction = direction,
        .brake_mode = brakeMode,
        .action = action
    };
    CHECK_THROW(mcpwm_generator_set_action_on_brake_event(_handle, eventAction));
}

void Generators::setForceLevel(std::optional<bool> level, bool holdOn)
{
    int levelInt = -1;
    if (level.has_value())
        levelInt = level.value()?1u:0u;
    CHECK_THROW(mcpwm_generator_set_force_level(_handle, levelInt, holdOn));
}

CaptureTimer::CaptureTimer(int groupId)
    : _handle(nullptr)
{
    mcpwm_capture_timer_config_t config = {
        .group_id = groupId,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT
    };
    CHECK_THROW(mcpwm_new_capture_timer(&config, &_handle));
}

CaptureTimer::~CaptureTimer()
{
    assert(mcpwm_del_capture_timer(_handle) == ESP_OK);
    _handle = nullptr;
}

void CaptureTimer::setPhaseOnSync(const std::optional<SyncSrc> & syncSrc, uint32_t countValue, mcpwm_timer_direction_t direction)
{
    mcpwm_capture_timer_sync_phase_config_t config {
        .sync_src = syncSrc.has_value()?syncSrc.value()._handle:nullptr,
        .count_value = countValue,
        .direction = direction
    };
    CHECK_THROW(mcpwm_capture_timer_set_phase_on_sync(_handle, &config));
}

void CaptureTimer::enable()
{
    CHECK_THROW(mcpwm_capture_timer_enable(_handle));
}

void CaptureTimer::disable()
{
    CHECK_THROW(mcpwm_capture_timer_disable(_handle));
}

void CaptureTimer::start()
{
    CHECK_THROW(mcpwm_capture_timer_start(_handle));
}

void CaptureTimer::stop()
{
    CHECK_THROW(mcpwm_capture_timer_stop(_handle));
}

uint32_t CaptureTimer::getResolution() const
{
    uint32_t outResolution;
    CHECK_THROW(mcpwm_capture_timer_get_resolution(_handle, &outResolution));
    return outResolution;
}

CaptureChannel::CaptureChannel(const CaptureTimer & captureTimer, GPIONum gpioNum,
        uint32_t prescale, bool posEdge, bool negEdge, bool pullUp, bool pullDown,
        bool invertCapSignal, bool ioLoopBack, bool keepIoConfAtExit)
    : _handle(nullptr)
{
    mcpwm_capture_channel_config_t config = {
        .gpio_num = static_cast<int>(gpioNum.get_value()),
        .prescale = prescale,
        .flags = {
            .pos_edge = posEdge?1:0u,
            .neg_edge = negEdge?1:0u,
            .pull_up = pullUp?1:0u,
            .pull_down = pullDown?1:0u,
            .invert_cap_signal = invertCapSignal?1:0u,
            .io_loop_back = ioLoopBack?1:0u,
            .keep_io_conf_at_exit = keepIoConfAtExit?1:0u
        }
    };
    CHECK_THROW(mcpwm_new_capture_channel(captureTimer._handle, &config, &_handle));
}

CaptureChannel::~CaptureChannel()
{
    assert(mcpwm_del_capture_channel(_handle) == ESP_OK);
    _handle = nullptr;
}

void CaptureChannel::registerEventCallbacks(const EventCallBack & callback)
{
    _callback = callback;
    mcpwm_capture_event_callbacks_t cbs {
        .on_cap = _callback?static_cast<mcpwm_capture_event_cb_t>([](mcpwm_cap_channel_handle_t, const mcpwm_capture_event_data_t * edata, void * userCtx) {
            const CaptureChannel * captureChannel = reinterpret_cast<const CaptureChannel *>(userCtx);
            return captureChannel->_callback(*captureChannel, *edata);
        }):nullptr
    };
    CHECK_THROW(mcpwm_capture_channel_register_event_callbacks(_handle, &cbs, this));
}

void CaptureChannel::enable()
{
    CHECK_THROW(mcpwm_capture_channel_enable(_handle));
}

void CaptureChannel::disable()
{
    CHECK_THROW(mcpwm_capture_channel_disable(_handle));
}

void CaptureChannel::triggerSoftCatch()
{
    CHECK_THROW(mcpwm_capture_channel_trigger_soft_catch(_handle));
}

} // mcpwm

} // idf

#endif // __cpp_exceptions
