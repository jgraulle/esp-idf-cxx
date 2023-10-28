#if __cpp_exceptions

#include "bdc_motor_cxx.hpp"

namespace idf {

BdcMotor::BdcMotor(GPIONum pwmAGpio, GPIONum pwmBGpio, uint32_t pwmFreqHz, int groupId, uint32_t timerResolutionHz)
    : _timer(groupId, timerResolutionHz, MCPWM_TIMER_COUNT_MODE_UP, timerResolutionHz / pwmFreqHz)
    , _operat(groupId, {}, {})
    , _cmpa(_operat, mcpwm::UpdateOnFlags(mcpwm::UpdateOn::TIMER_COUNT_TO_ZERO))
    , _cmpb(_operat, mcpwm::UpdateOnFlags(mcpwm::UpdateOn::TIMER_COUNT_TO_ZERO))
    , _gena(_operat, pwmAGpio)
    , _genb(_operat, pwmBGpio)
{
    _operat.connect(_timer);
    _cmpa.setCompareValue(0);
    _cmpb.setCompareValue(0);
    _gena.setActionOnTimerEvent(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH);
    _gena.setActionOnCompareEvent(MCPWM_TIMER_DIRECTION_UP, _cmpa, MCPWM_GEN_ACTION_LOW);
    _genb.setActionOnTimerEvent(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH);
    _genb.setActionOnCompareEvent(MCPWM_TIMER_DIRECTION_UP, _cmpb, MCPWM_GEN_ACTION_LOW);
}

BdcMotor::~BdcMotor()
{
}

void BdcMotor::enable()
{
    _timer.enable();
    _timer.start_stop(MCPWM_TIMER_START_NO_STOP);
}

void BdcMotor::disable()
{
    _timer.start_stop(MCPWM_TIMER_STOP_EMPTY);
    _timer.disable();
}

void BdcMotor::setPower(uint32_t power)
{
    _cmpa.setCompareValue(power);
    _cmpb.setCompareValue(power);
}

void BdcMotor::forward()
{
    _gena.setForceLevel({}, true); // Drive GPIO from PWM
    _genb.setForceLevel(false, true); // Set GPIO to 0V
}

void BdcMotor::reverse()
{
    _gena.setForceLevel(false, true); // Set GPIO to 0V
    _genb.setForceLevel({}, true); // Drive GPIO from PWM
}

void BdcMotor::coast()
{
    _gena.setForceLevel(false, true); // Set GPIO to 0V
    _genb.setForceLevel(false, true); // Set GPIO to 0V
}

void BdcMotor::brake()
{
    _gena.setForceLevel(true, true); // Set GPIO to 3.3V
    _genb.setForceLevel(true, true); // Set GPIO to 3.3V
}

} // idf

#endif // __cpp_exceptions
