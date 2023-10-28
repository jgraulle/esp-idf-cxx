#pragma once

#if __cpp_exceptions

#include "esp_exception.hpp"
#include "system_cxx.hpp"
#include "driver/mcpwm_types.h"
#include "gpio_cxx.hpp"
#include <functional>
#include <optional>

namespace idf {

template <typename Flag, typename UInt>
class Flags {
public:
    inline Flags() : _value(0) {}
    inline Flags(UInt value) : _value(value) {}
    friend inline Flags operator|(Flag a, Flag b)
        {return Flags(static_cast<UInt>(a) | static_cast<UInt>(b));}
    inline bool isSet(Flag flag) const {return flag&_value;}
    inline UInt isSet0uOr1u(Flag flag) const {return (flag&_value)?1u:0u;}
private:
    UInt _value;
};


namespace mcpwm {

enum UpdateOn {
    TIMER_COUNT_TO_ZERO = 1,
    TIMER_COUNT_TO_PEAK = 2,
    SYNC = 4
};
using UpdateOnFlags = Flags<UpdateOn, uint8_t>;

class SyncSrc
{
public:
    /**
     * @brief Delete MCPWM sync source
     *
     * @note assert if failed
     */
    virtual ~SyncSrc();

protected:
    inline SyncSrc() : _handle(nullptr) {}
    inline mcpwm_sync_handle_t & getHandle() {return _handle;}

private:
    SyncSrc(const SyncSrc &) = delete;
    SyncSrc & operator=(const SyncSrc &) = delete;
    friend class Timer;
    friend class CaptureTimer;

    mcpwm_sync_handle_t _handle;
};

class GpioSyncSrc : public SyncSrc
{
public:
    /**
     * @brief Create MCPWM GPIO sync source
     *
     * @param[in] groupId MCPWM group ID
     * @param[in] gpioNum GPIO used by sync source
     * @param[in] activeNeg Whether the sync signal is active on negedge, by default, the sync signal's posedge is treated as active
     * @param[in] ioLoopBack For debug/test, the signal output from the GPIO will be fed to the input path as well
     * @param[in] pullUp Whether to pull up internally
     * @param[in] pullDown Whether to pull down internally
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    GpioSyncSrc(int groupId, GPIONum gpioNum, bool activeNeg, bool ioLoopBack, bool pullUp, bool pullDown);
};

class Timer;
class TimerSyncSrc : public SyncSrc
{
public:
    /**
     * @brief Create MCPWM timer sync source
     *
     * @param[in] timer MCPWM timer
     * @param timerEvent Timer event, upon which MCPWM timer will generate the sync signal
     * @param propagateInputSync The input sync signal would be routed to its sync output
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the timer has created a sync source before
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    TimerSyncSrc(const Timer & timer, mcpwm_timer_event_t timerEvent, bool propagateInputSync);
};

class SoftSyncSrc : public SyncSrc
{
public:
    /**
     * @brief Create MCPWM software sync source
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    SoftSyncSrc();

    /**
     * @brief Activate the software sync, trigger the sync event for once
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void activate();
};

class Timer
{
public:
    /**
     * @brief Create MCPWM timer
     *
     * @param groupId Specifies the MCPWM group ID. The ID should belong to [0, SOC_MCPWM_GROUPS - 1] range.
     *        Please note, timers located in different groups are totally independent.
     * @param resolutionHz Counter resolution in Hz, ranges from around 300KHz to 80MHz.
     *        The step size of each count tick equals to (1 / resolution_hz) seconds
     * @param countMode Count mode
     * @param periodTicks Number of count ticks within a period
     * @param updatePeriodOnEmpty Whether to update the period value when timer counts to zero
     * @param updatePeriodOnSync Whether to update the period value when the timer takes a sync signal
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG): if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM): if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND): if all hardware timers are used up and no more free one
     *      - idf::ESPException(ESP_FAIL): if other error
     */
    Timer(int groupId, uint32_t resolutionHz, mcpwm_timer_count_mode_t countMode,
            uint32_t periodTicks, bool updatePeriodOnEmpty = false, bool updatePeriodOnSync = false);

    /**
     * @brief Delete MCPWM timer
     *
     * @note assert if failed
     */
    ~Timer();

    using EventCallBack = std::function<bool(const Timer &, const mcpwm_timer_event_data_t &)>;

    /**
     * @brief Set event callbacks for MCPWM timer
     *
     * @note The first call to this function needs to be before the call to `enable`
     * @note User can deregister a previously registered callback by calling this function with default constructed EventCallBack.
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note User registered callbacks must return true if a high priority task has been waken up by this function.
     * @note User registered callbacks can be a lambda with captured param.
     *
     * @param onFull callback function when MCPWM timer counts to peak value
     * @param onEmpty callback function when MCPWM timer counts to zero
     * @param onStop callback function when MCPWM timer stops
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is not in init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallBacks(const EventCallBack & onFull, const EventCallBack & onEmpty,
            const EventCallBack & onStop);

    /**
     * @brief Enable MCPWM timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is enabled already
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void enable();

    /**
     * @brief Disable MCPWM timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is disabled already
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void disable();

    /**
     * @brief Send specific start/stop commands to MCPWM timer
     *
     * @param[in] command Supported command list for MCPWM timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is not enabled
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void start_stop(mcpwm_timer_start_stop_cmd_t command);

    /**
     * @brief Set sync phase for MCPWM timer
     *
     * @param syncSrc The sync event source. Set to empty optional value will disable the timer being synced by others
     * @param countValue The count value that should lock to upon sync event
     * @param direction The count direction that should lock to upon sync event
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setPhaseOnSync(const std::optional<SyncSrc> & syncSrc, uint32_t countValue, mcpwm_timer_direction_t direction);

private:
    Timer(const Timer &) = delete;
    Timer & operator=(const Timer &) = delete;
    friend class Operator;
    friend class TimerSyncSrc;

    mcpwm_timer_handle_t _handle;
    EventCallBack _onFull;
    EventCallBack _onEmpty;
    EventCallBack _onStop;
    bool _isEnable;
};

class Fault
{
public:
    /**
     * @brief Delete MCPWM fault
     *
     * @note assert if failed
     */
    virtual ~Fault();

    using EventCallBack = std::function<bool(const Fault &, const mcpwm_fault_event_data_t &)>;

    /**
     * @brief Set event callbacks for MCPWM fault
     *
     * @note User can deregister a previously registered callback by calling this function with default constructed EventCallBack.
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note User registered callbacks must return true if a high priority task has been waken up by this function.
     * @note User registered callbacks can be a lambda with captured param.
     *
     * @param onFaultEnter Group of callback functions
     * @param onFaultExit Group of callback functions
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallbacks(const EventCallBack & onFaultEnter, const EventCallBack & onFaultExit);

protected:
    inline Fault() : _handle(nullptr) {}
    inline mcpwm_fault_handle_t & getHandle() {return _handle;}

private:
    Fault(const Fault &) = delete;
    Fault & operator=(const Fault &) = delete;
    friend class Operator;

    mcpwm_fault_handle_t _handle;
    EventCallBack _onFaultEnter;
    EventCallBack _onFaultExit;
};

class GpioFault : public Fault
{
public:
    /**
     * @brief Create MCPWM GPIO fault
     *
     * @param groupId In which MCPWM group that the GPIO fault belongs to
     * @param gpioNum GPIO used by the fault signal
     * @param activeLevel On which level the fault signal is treated as active
     * @param ioLoopBack For debug/test, the signal output from the GPIO will be fed to the input path as well
     * @param pullUp Whether to pull up internally
     * @param pullDown Whether to pull down internally
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    GpioFault(int groupId, GPIONum gpioNum, bool activeLevel, bool ioLoopBack, bool pullUp, bool pullDown);
};

class SoftFault : public Fault
{
public:
    /**
     * @brief Create MCPWM software fault
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    SoftFault();

    /**
     * @brief Activate the software fault, trigger the fault event for once
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void activate();
};

class Operator
{
public:
    /**
     * @brief Create MCPWM operator
     *
     * @param groupId Specify from which group to allocate the MCPWM operator
     * @param updateGenAction Whether to update generator action
     * @param updateDeadTime Whether to update dead time
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG): if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM): if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND): if can't find free resource
     *      - idf::ESPException(ESP_FAIL): if other error
     */
    Operator(int groupId, UpdateOnFlags updateGenAction, UpdateOnFlags updateDeadTime);

    /**
     * @brief Delete MCPWM operator
     *
     * @note assert if failed
     */
    ~Operator();

    /**
     * @brief Connect MCPWM operator and timer, so that the operator can be driven by the timer
     *
     * @note Make sure the MCPWM timer and operator are in the same group, otherwise, this function
     *       will throw idf::ESPException(ESP_ERR_INVALID_ARG)
     *
     * @param[in] timer MCPWM timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void connect(const Timer & timer);

    /**
     * @brief Apply carrier feature for MCPWM operator
     *
     * @param frequencyHz Carrier frequency in Hz
     * @param firstPulseDurationUs The duration of the first PWM pulse, in us
     * @param dutyCycle Carrier duty cycle
     * @param invertBeforeModulate Invert the raw signal
     * @param invertAfterModulate Invert the modulated signal
     * 
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void applyCarrier(uint32_t frequencyHz, uint32_t firstPulseDurationUs, float dutyCycle, bool invertBeforeModulate, bool invertAfterModulate);
    void removeCarrier();

    /**
     * @brief Set brake method for MCPWM operator
     *
     * @param fault Which fault causes the operator to brake
     * @param brakeMode Brake mode
     * @param cbcRecoverOnTez Recovery CBC brake state on tez event
     * @param cbcRecoverOnTep Recovery CBC brake state on tep event
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setBrakeOnFault(const Fault & fault, mcpwm_operator_brake_mode_t brakeMode, bool cbcRecoverOnTez, bool cbcRecoverOnTep);

    /**
     * @brief Try to make the operator recover from fault
     *
     * @note To recover from fault or escape from trip, you make sure the fault signal has dissappeared already.
     *       Otherwise the recovery can't succeed.
     *
     * @param fault MCPWM fault
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the fault source is still active
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void recoverFromFault(const Fault & fault);

    using EventCallBack = std::function<bool(const Operator &, const mcpwm_brake_event_data_t &)>;

    /**
     * @brief Set event callbacks for MCPWM operator
     *
     * @note User can deregister a previously registered callback by calling this function with default constructed EventCallBack.
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note User registered callbacks must return true if a high priority task has been waken up by this function.
     * @note User registered callbacks can be a lambda with captured param.
     *
     * @param onBrakeCbc callback function when mcpwm operator brakes in CBC
     * @param onBrakeOst callback function when mcpwm operator brakes in OST
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallbacks(const EventCallBack & onBrakeCbc, const EventCallBack & onBrakeOst);

private:
    Operator(const Operator &) = delete;
    Operator & operator=(const Operator &) = delete;
    friend class Comparators;
    friend class Generators;

    mcpwm_oper_handle_t _handle;
    EventCallBack _onBrakeCbc;
    EventCallBack _onBrakeOst;
};

class Comparators
{
public:
    /**
     * @brief Create MCPWM comparator
     *
     * @param operator MCPWM operator, the new comparator will be allocated from this operator
     * @param flags Whether to update the compare threshold
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    Comparators(const Operator & operat, UpdateOnFlags flags);

    /**
     * @brief Delete MCPWM comparator
     *
     * @note assert if failed
     */
    ~Comparators();

    using EventCallBack = std::function<bool(const Comparators &, const mcpwm_compare_event_data_t &)>;

    /**
     * @brief Set event callbacks for MCPWM comparator
     *
     * @note User can deregister a previously registered callback by calling this function with a default constructed EventCallBack
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note User registered callbacks must return true if a high priority task has been waken up by this function.
     * @note User registered callbacks can be a lambda with captured param.
     *
     * @param[in] callback ISR callback function which would be invoked when counter reaches compare value
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallBacks(const EventCallBack & callback);

    /**
     * @brief Set MCPWM comparator's compare value
     *
     * @param[in] cmpTicks The new compare value
     * @note New compare value might won't take effect immediately. The update time for the compare
     *       value is set by :cpp:func:`Comparators`.
     * @note Make sure the operator has connected to one MCPWM timer already by
     *       :cpp:func:`Operator::connect`. Otherwise, it will throw error code `ESP_ERR_INVALID_STATE`.
     * @note The compare value shouldn't exceed timer's count peak, otherwise, the compare event
     *       will never got triggered.
     * @note This function can be call in ISR context
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument (e.g. the cmp_ticks is out of range)
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the operator doesn't have a timer connected
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setCompareValue(uint32_t cmpTicks);

private:
    Comparators(const Comparators &) = delete;
    Comparators & operator=(const Comparators &) = delete;
    friend class Generators;

    mcpwm_cmpr_handle_t _handle;
    EventCallBack _callback;
};

class Generators
{
public:
    /**
     * @brief Allocate MCPWM generator from given operator
     *
     * @param[in] operat MCPWM operator
     * @param gpioNum The GPIO number used to output the PWM signal
     * @param invertPwm Whether to invert the PWM signal (done by GPIO matrix)
     * @param ioLoopBack For debug/test, the signal output from the GPIO will be fed to the input path as well
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    Generators(const Operator & operat, GPIONum gpioNum, bool invertPwm = false, bool ioLoopBack = false);

    /**
     * @brief Delete MCPWM generator
     *
     * @note assert if failed
     */
    ~Generators();

    /**
     * @brief Set generator action on MCPWM timer event
     *
     * @param direction Timer direction
     * @param event Timer event
     * @param action Generator action should perform
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if is not connected to operator
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setActionOnTimerEvent(mcpwm_timer_direction_t direction, mcpwm_timer_event_t event, mcpwm_generator_action_t action);

    /**
     * @brief Set generator action on MCPWM compare event
     *
     * @param direction Timer direction
     * @param comparator Comparator
     * @param action Generator action should perform
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setActionOnCompareEvent(mcpwm_timer_direction_t direction, const Comparators & comparator, mcpwm_generator_action_t action);

    /**
     * @brief Set dead time for MCPWM generator
     *
     * @note Due to a hardware limitation, you can't set rising edge delay for both MCPWM generator 0 and 1 at the same time,
     *       otherwise, there will be a conflict inside the dead time module. The same goes for the falling edge setting.
     *       But you can set both the rising edge and falling edge delay for the same MCPWM generator.
     *
     * @param posedgeDelayTicks delay time applied to rising edge, 0 means no rising delay time
     * @param negedgeDelayTicks delay time applied to falling edge, 0 means no falling delay time
     * @param invertOutput Invert the signal after applied the dead time
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if invalid state (e.g. delay module is already in use by other generator)
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setDeadTime(uint32_t posedgeDelayTicks, uint32_t negedgeDelayTicks, bool invertOutput = false);

    /**
     * @brief Set generator action on MCPWM brake event
     *
     * @param direction Timer direction
     * @param brakeMode Brake mode
     * @param action Generator action should perform
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setActionOnBrakeEvent(mcpwm_timer_direction_t direction, mcpwm_operator_brake_mode_t brakeMode, mcpwm_generator_action_t action);

    /**
     * @brief Set force level for MCPWM generator
     *
     * @note The force level will be applied to the generator immediately, regardless any other events that would change the generator's behaviour.
     * @note If the `hold_on` is true, the force level will retain forever, until user removes the force level by setting the force level with empty optional value.
     * @note If the `hold_on` is false, the force level can be overridden by the next event action.
     * @note The force level set by this function can be inverted by GPIO matrix or dead-time module. So the level set here doesn't equal to the final output level.
     *
     * @param[in] level GPIO level to be applied to MCPWM generator, specially, empty optional means to remove the force level
     * @param[in] holdOn Whether the forced PWM level should retain (i.e. will remain unchanged until manually remove the force level)
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setForceLevel(std::optional<bool> level, bool holdOn);

private:
    Generators(const Generators &) = delete;
    Generators & operator=(const Generators &) = delete;

    mcpwm_gen_handle_t _handle;
};

class CaptureTimer
{
public:
    /**
     * @brief Create MCPWM capture timer
     *
     * @param groupId Specify from which group to allocate the capture timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    CaptureTimer(int groupId);

    /**
     * @brief Delete MCPWM capture timer
     *
     * @note assert if failed
     */
    ~CaptureTimer();

    /**
     * @brief Set sync phase for MCPWM capture timer
     *
     * @param syncSrc The sync event source. Set to empty optional value will disable the timer being synced by others
     * @param countValue The count value that should lock to upon sync event
     * @param direction The count direction that should lock to upon sync event
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setPhaseOnSync(const std::optional<SyncSrc> & syncSrc, uint32_t countValue, mcpwm_timer_direction_t direction);

    /**
     * @brief Enable MCPWM capture timer
     *
     * Internally, this function will:
     * - switch the capture timer state from **init** to **enable**.
     * - acquire a proper power management lock if a specific clock source (e.g. APB clock) is selected.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is enabled already
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void enable();

    /**
     * @brief Disable MCPWM capture timer
     *
     * Internally, this function will put the timer driver back to **init** state, and release the power management lock.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if timer is disabled already
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void disable();

    /**
     * @brief Start MCPWM capture timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void start();

    /**
     * @brief Stop MCPWM capture timer
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void stop();

    /**
     * @brief Get MCPWM capture timer resolution, in Hz
     *
     * @return Returned capture timer resolution, in Hz
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    uint32_t getResolution() const;

private:
    CaptureTimer(const CaptureTimer &) = delete;
    CaptureTimer & operator=(const CaptureTimer &) = delete;
    friend class CaptureChannel;

    mcpwm_cap_timer_handle_t _handle;
};

class CaptureChannel
{
public:
    /**
     * @brief Create MCPWM capture channel
     *
     * @note The created capture channel won't be enabled until calling `enable`
     *
     * @param[in] captureTimer MCPWM capture timer will be connected to the new capture channel
     * @param gpioNum GPIO used capturing input signal
     * @param prescale Prescale of input signal, effective frequency = cap_input_clk/prescale
     * @param posEdge Whether to capture on positive edge
     * @param negEdge Whether to capture on negative edge
     * @param pullUp Whether to pull up internally
     * @param pullDown Whether to pull down internally
     * @param invertCapSignal Invert the input capture signal
     * @param ioLoopBack For debug/test, the signal output from the GPIO will be fed to the input path as well
     * @param keepIoConfAtExit For debug/test, whether to keep the GPIO configuration when capture channel is deleted.
     *        By default, driver will reset the GPIO pin at exit.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if can't find free resource
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    CaptureChannel(const CaptureTimer & captureTimer, GPIONum gpioNum,
            uint32_t prescale, bool posEdge, bool negEdge, bool pullUp, bool pullDown,
            bool invertCapSignal, bool ioLoopBack, bool keepIoConfAtExit);

    /**
     * @brief Delete MCPWM capture channel
     *
     * @note assert if failed
     */
    ~CaptureChannel();

    using EventCallBack = std::function<bool(const CaptureChannel &, const mcpwm_capture_event_data_t &)>;

    /**
     * @brief Set event callbacks for MCPWM capture channel
     *
     * @note The first call to this function needs to be before the call to `enable`
     * @note User can deregister a previously registered callback by calling this function with a default constructed EventCallBack
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note User registered callbacks must return true if a high priority task has been waken up by this function.
     * @note User registered callbacks can be a lambda with captured param.
     *
     * @param callback Callback function that would be invoked when capture event occurred
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the channel is not in init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallbacks(const EventCallBack & callback);

    /**
     * @brief Enable MCPWM capture channel
     *
     * @note This function will transit the channel state from init to enable.
     * @note This function will enable the interrupt service, if it's lazy installed in `registerEventCallbacks()`.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the channel is already enabled
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void enable();

    /**
     * @brief Disable MCPWM capture channel
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the channel is not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void disable();

    /**
     * @brief Trigger a catch by software
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the channel is not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void triggerSoftCatch();

private:
    CaptureChannel(const CaptureChannel &) = delete;
    CaptureChannel & operator=(const CaptureChannel &) = delete;

    mcpwm_cap_channel_handle_t _handle;
    EventCallBack _callback;
};

} // mcpwm

} // idf

#endif // __cpp_exceptions
