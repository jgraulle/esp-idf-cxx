#pragma once

#ifdef __cpp_exceptions

#include <driver/gptimer_types.h>
#include <functional>

namespace idf {

class GpTimer
{
public:
    /**
     * @brief Create a new General Purpose Timer, and return the handle
     *
     * @note The newly created timer is put in the "init" state.
     *
     * @param[in] direction Count direction
     * @param[in] resolution Counter resolution (working frequency) in Hz, hence, the step size of
     *            each count tick equals to (1 / resolution_hz) seconds
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if all hardware timers are used up and no more free one
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    GpTimer(gptimer_count_direction_t direction, uint32_t resolution);

    /**
     * @brief Delete the GPTimer handle
     *
     * @note If not in "init" state call stop() and/or disable() before
     */
    ~GpTimer();

    using EventCallBack = std::function<bool(const GpTimer &, const gptimer_alarm_event_data_t &)>;

    /**
     * @brief Set callbacks for GPTimer
     *
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note The first call to this function needs to be before the call to `enable`
     * @note User can deregister a previously registered callback by calling this function with a default constructed EventCallBack.
     *
     * @param[in] callback Timer alarm callback, you can use lambda to capture param.
     *            Must return true if a high priority task has been waken up by this function.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the timer is not in init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallbacks(const EventCallBack & callback);

    /**
     * @brief Enable GPTimer
     *
     * @note This function will transit the timer state from "init" to "enable".
     * @note This function will enable the interrupt service, if it's lazy installed in `registerEventCallbacks`.
     * @note Enable a timer doesn't mean to start it. See also `start` for how to make the timer start counting.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if already enabled
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void enable();

    /**
     * @brief Disable GPTimer
     *
     * @note This function will transit the timer state from "enable" to "init".
     * @note This function will disable the interrupt service if it's installed.
     * @note Disable a timer doesn't mean to stop it. See also `stop` for how to make the timer stop counting.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void disable();

    /**
     * @brief Start GPTimer (internal counter starts counting)
     *
     * @note This function will transit the timer state from "enable" to "run".
     * @note This function is allowed to run within ISR context
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not enabled or is already in running
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void start();

    /**
     * @brief Stop GPTimer (internal counter stops counting)
     *
     * @note This function will transit the timer state from "run" to "enable".
     * @note This function is allowed to run within ISR context
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if is not in running.
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void stop();

    /**
     * @brief Set GPTimer raw count value
     *
     * @note When updating the raw count of an active timer, the timer will immediately start counting from the new value.
     * @note This function is allowed to run within ISR context
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @param[in] value Count value to be set
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setRawCount(uint64_t value);

    /**
     * @brief Get GPTimer raw count value
     *
     * @note This function will trigger a software capture event and then return the captured count value.
     * @note With the raw count value and the resolution returned from `getResolution`, you can convert the count value into seconds.
     * @note This function is allowed to run within ISR context
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @return GPTimer count value
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    uint64_t getRawCount() const;

    /**
     * @brief Set period for a periodic GPTimer.
     *
     * @note This function is allowed to run within ISR context, so that user can set new alarm action immediately in the ISR callback.
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @param[in] alarmCount target count value
     * @param[in] reloadCount Alarm reload count value
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    inline void setAlarmAction(uint64_t alarmCount, uint64_t reloadCount)
            {setAlarmActionHelper(alarmCount, true, reloadCount);}

    /**
     * @brief Set time for a one shot GPTimer.
     *
     * @note This function is allowed to run within ISR context, so that user can set new alarm action immediately in the ISR callback.
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @param[in] alarmCount target count value
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    inline void setAlarmAction(uint64_t alarmCount)
            {setAlarmActionHelper(alarmCount, false, 0u);}

    /**
     * @brief Disabling the alarm function
     *
     * @note This function is allowed to run within ISR context, so that user can set new alarm action immediately in the ISR callback.
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void unsetAlarmAction();

    enum class State {INIT, ENABLE, RUN};

    /**
     * @brief Return the current state of this GPTimer
     */
    inline State getState() const {return _state;}

    /**
     * @brief Return the real resolution of the timer
     *
     * @note usually the timer resolution is same as what you configured in the constructor,
     *       but some unstable clock source (e.g. RC_FAST) will do a calibration, the real
     *       resolution can be different from the configured one.
     *
     * @return timer resolution, in Hz
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL): if other error
     */
    uint32_t getResolution() const;

    /**
     * @brief Get GPTimer captured count value
     *
     * @note The capture action can be issued either by ETM event or by software (see also `getRawCount`).
     * @note This function is allowed to run within ISR context
     * @note If `CONFIG_GPTIMER_CTRL_FUNC_IN_IRAM` is enabled, this function will be placed in the IRAM by linker,
     *       makes it possible to execute even when the Flash Cache is disabled.
     *
     * @return captured count value
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    uint64_t getCapturedCount() const;

private:
    void setAlarmActionHelper(uint64_t alarmCount, bool isAutoReloadOnAlarm, uint64_t reloadCount);

    GpTimer(const GpTimer &) = delete;
    GpTimer & operator=(const GpTimer &) = delete;

    gptimer_handle_t _gptimer;
    State _state;
    EventCallBack _callback;
};

} // idf

#endif // __cpp_exceptions
