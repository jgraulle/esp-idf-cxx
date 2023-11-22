#pragma once

#ifdef __cpp_exceptions

#include "driver/ledc.h"
#include "gpio_cxx.hpp"


namespace idf {

class LedcTimer
{
public:
    /**
     * @brief LEDC timer configuration
     *        Configure LEDC timer with the given source timer/frequency(Hz)/duty_resolution
     *
     * @param timerId The timer source of channel (0 - LEDC_TIMER_MAX-1)
     * @param freq LEDC timer frequency (Hz)
     * @param speedMode LEDC speed speed_mode, high-speed mode or low-speed mode
     * @param dutyBitWidth LEDC channel duty resolution
     * @param clkCfg Configure LEDC source clock from ledc_clk_cfg_t.
     *               Note that LEDC_USE_RC_FAST_CLK and LEDC_USE_XTAL_CLK are
     *               non-timer-specific clock sources. You can not have one LEDC timer uses
     *               RC_FAST_CLK as the clock source and have another LEDC timer uses XTAL_CLK
     *               as its clock source. All chips except esp32 and esp32s2 do not have
     *               timer-specific clock sources, which means clock source for all timers
     *               must be the same one.
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     *        - idf::ESPException(ESP_FAIL) Can not find a proper pre-divider number base on the
     *          given frequency and the current duty_resolution.
     */
    LedcTimer(ledc_timer_t timerId, uint32_t freq, ledc_mode_t speedMode,
            ledc_timer_bit_t dutyBitWidth, ledc_clk_cfg_t clkCfg = LEDC_AUTO_CLK);

    /**
     * \brief Reset timer
     */
    ~LedcTimer();

    inline ledc_timer_t getTimerId() const {return _timerId;}
    inline ledc_mode_t getSpeedMode() const {return _speedMode;}

    /**
     * @brief LEDC set channel frequency (Hz)
     *
     * @param freq Set the LEDC frequency
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     *        - idf::ESPException(ESP_FAIL) Can not find a proper pre-divider number base on the
     *          given frequency and the current duty_resolution.
     */
    void setFreq(uint32_t freq);

    /**
     * @brief LEDC get channel frequency (Hz)
     *
     * @return Current LEDC frequency in Hz
     * @throw
     *        - idf::ESPException(ESP_FAIL) Error
     *
     */
    uint32_t getFreq() const;

    /**
     * @brief Pause LEDC timer counter
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    void pause();

    /**
     * @brief Resume LEDC timer
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    void resume();

private:
    LedcTimer(const LedcTimer &) = delete;
    LedcTimer & operator=(const LedcTimer &) = delete;

    ledc_timer_t _timerId;
    ledc_mode_t _speedMode;
};

class LedcChannel
{
public:
    /**
     * @brief LEDC channel configuration
     *        Configure LEDC channel with the given channel/output gpio_num/interrupt/source timer/frequency(Hz)/LEDC duty resolution
     *
     * @param channelId LEDC channel (0 - LEDC_CHANNEL_MAX-1)
     * @param timer The associated timer source of this channel
     * @param gpioNum the LEDC output gpio_num, if you want to use gpio16, idf::GPIONum(16)
     * @param duty LEDC channel duty, the range of duty setting is [0, (2**dutyBitWidth)]
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    LedcChannel(ledc_channel_t channelId, const LedcTimer & timer, GPIONum gpioNum, uint32_t duty = 0);

    /**
     * @brief Stop the channel
     */
    ~LedcChannel();

    /**
     * @brief LEDC set duty and hpoint value
     *        Only after calling updateDuty will the duty update.
     * @note  setDuty, setDutyWithHpoint and updateDuty are not thread-safe, do not call these functions to
     *        control one LEDC channel in different tasks at the same time.
     * @note  For ESP32, hardware does not support any duty change while a fade operation is running in progress on that channel.
     *        Other duty operations will have to wait until the fade operation has finished.
     * @param duty Set the LEDC duty, the range of duty setting is [0, (2**dutyBitWidth) - 1]
     * @param hpoint Set the LEDC hpoint value(max: 0xfffff)
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    void setDutyWithHpoint(uint32_t duty, uint32_t hpoint);

    /**
     * @brief LEDC get hpoint value, the counter value when the output is set high level.
     *
     * @return Current hpoint value of LEDC channel
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    int getHpoint() const;

    /**
     * @brief LEDC set duty
     *        This function do not change the hpoint value of this channel. if needed, please call setDutyWithHpoint.
     *        only after calling updateDuty will the duty update.
     * @note  setDuty, setDutyWithHpoint and updateDuty are not thread-safe, do not call these functions to
     *        control one LEDC channel in different tasks at the same time.
     * @note  For ESP32, hardware does not support any duty change while a fade operation is running in progress on that channel.
     *        Other duty operations will have to wait until the fade operation has finished.
     * @param duty Set the LEDC duty, the range of duty setting is [0, (2**dutyBitWidth) - 1]
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    void setDuty(uint32_t duty);

    /**
     * @brief LEDC get duty
     *        This function returns the duty at the present PWM cycle.
     *        You shouldn't expect the function to return the new duty in the same cycle of calling ledc_update_duty,
     *        because duty update doesn't take effect until the next cycle.
     *
     * @return Current LEDC duty
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    uint32_t getDuty() const;

    /**
     * @brief LEDC update channel parameters
     * @note  Call this function to activate the LEDC updated parameters.
     *        After setDuty, we need to call this function to update the settings.
     *        And the new LEDC parameters don't take effect until the next PWM cycle.
     * @note  setDuty, setDutyWithHpoint and updateDuty are not thread-safe, do not call these functions to
     *        control one LEDC channel in different tasks at the same time.
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     *
     */
    void updateDuty();

    /**
     * @brief LEDC stop.
     *        Disable LEDC output, and set idle level
     *
     * @param idleLevel Set output idle level after LEDC stops.
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) Parameter error
     */
    void stop(bool idleLevel = false);

private:
    LedcChannel(const LedcChannel &) = delete;
    LedcChannel & operator=(const LedcChannel &) = delete;

    ledc_channel_t _channelId;
    const LedcTimer & _timer;
};

} // idf

#endif // __cpp_exceptions
