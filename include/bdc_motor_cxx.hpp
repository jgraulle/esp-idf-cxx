#pragma once

#if __cpp_exceptions

#include <mcpwm_cxx.hpp>

namespace idf {

class BdcMotor
{
public:
    //! @brief Create a brushed DC electric motor.
    //! @param pwmAGpio One of the two output gpio_num, if you want to use gpio16, idf::GPIONum(16)
    //! @param pwmBGpio One of the two output gpio_num, if you want to use gpio16, idf::GPIONum(16)
    //! @param pwmFreqHz The PWM frequency in Hz. The final value will be a multiple of timerResolutionHz.
    //! @param groupId Specifies the MCPWM group ID. The ID should belong to
    //!                [0, SOC_MCPWM_GROUPS - 1] range.
    //!                Please note, timers located in different groups are totally independent.
    //! @param timerResolutionHz The internal timer frequency resolution in Hz from 300KHz to 80MHz.
    BdcMotor(GPIONum pwmAGpio, GPIONum pwmBGpio, uint32_t pwmFreqHz, int groupId, uint32_t timerResolutionHz);

    //! @brief Delete a brushed DC electric motor and free all resources.
    ~BdcMotor();

    //! @brief Start the PWM timer.
    void enable();

    //! @brief Stop the PWM timer.
    void disable();

    //! @brief Set the motor power.
    //! @param power The duty cycle of the PWM in range [0, timerResolutionHz/pwmFreqHz]
    void setPower(uint32_t power);

    //! Drive this motor in normal direction.
    void forward();

    //! Drive this motor in reverse direction.
    void reverse();

    //! Set this motor in free wheel.
    void coast();

    //! Set this motor in electromagnetic braking mode.
    void brake();

private:
    BdcMotor(const BdcMotor &) = delete;
    BdcMotor & operator=(const BdcMotor &) = delete;

    idf::mcpwm::Timer _timer;
    idf::mcpwm::Operator _operat;
    idf::mcpwm::Comparators _cmpa;
    idf::mcpwm::Comparators _cmpb;
    idf::mcpwm::Generators _gena;
    idf::mcpwm::Generators _genb;
};

} // idf

#endif // __cpp_exceptions
