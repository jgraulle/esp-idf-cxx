#pragma once

#ifdef __cpp_exceptions

#include "gpio_cxx.hpp"

#include <driver/pulse_cnt.h>
#include <optional>
#include <functional>

namespace idf {

class PulseCounter
{
public:
    /**
     * @brief Create a new PCNT unit
     *
     * @note The newly created PCNT unit is put in the init state.
     *
     * @param[in] lowLimit Low limitation of the count unit, should be lower than 0
     * @param[in] highLimit High limitation of the count unit, should be higher than 0
     * @param[in] accumCount Whether to accumulate the count value when overflows at the high/low limit
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument (e.g. high/low limit value out of the range)
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if all PCNT units are used up and no more free one
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    PulseCounter(int lowLimit, int highLimit, bool accumCount);

    /**
     * @brief Delete the PCNT unit handle
     *
     * @note If not in "init" state this function call stop() and/or disable() before
     */
    ~PulseCounter();

    class Channel
    {
    public:
        /**
         * @brief Create PCNT channel for specific unit, each PCNT has several channels associated with it
         *
         * @note This function should be called when the unit is in init state (i.e. before calling `enable()`)
         *
         * @param[in] pulseCounter PCNT unit
         * @param[in] edge GPIO number used by the edge signal, input mode with pull up enabled.
         * @param[in] level GPIO number used by the level signal, input mode with pull up enabled.
         * @return channel handle
         *
         * @throw
         *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
         *      - idf::ESPException(ESP_ERR_NO_MEM) if insufficient memory
         *      - idf::ESPException(ESP_ERR_NOT_FOUND) if all PCNT channels are used up and no more free one
         *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the unit is not in the init state
         *      - idf::ESPException(ESP_FAIL) if other error
         */
        Channel(const PulseCounter & pulseCounter, std::optional<GPIONum> edge, std::optional<GPIONum> level);

        /**
         * @brief Delete the PCNT channel
         */
        ~Channel();

        /**
         * @brief Set channel actions when edge signal changes (e.g. falling or rising edge occurred).
         *        The edge signal is input from the `GPIONum` configured in `addChannel`.
         *        We use these actions to control when and how to change the counter value.
         *
         * @param[in] posedgeAction Action on posedge signal
         * @param[in] negedgeAction Action on negedge signal
         *
         * @throw
         *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
         *      - idf::ESPException(ESP_FAIL) if other error
         */
        void setEdgeChannelAction(pcnt_channel_edge_action_t posedgeAction,
            pcnt_channel_edge_action_t negedgeAction);

        /**
         * @brief Set channel actions when level signal changes (e.g. signal level goes from high to low).
         *        The level signal is input from the `GPIONum` configured in `addChannel`.
         *        We use these actions to control when and how to change the counting mode.
         *
         * @param[in] hightAction Action on high level signal
         * @param[in] lowAction Action on low level signal
         *
         * @throw
         *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
         *      - idf::ESPException(ESP_FAIL) if other error
         */
        void setLevelChannelAction(pcnt_channel_level_action_t hightAction,
            pcnt_channel_level_action_t lowAction);

    private:
        Channel(const Channel &) = delete;
        Channel & operator=(const Channel &) = delete;

        pcnt_channel_handle_t _channel;
    };

    /**
     * @brief Add a watch point for PCNT unit, PCNT will generate an event when the counter value reaches the watch point value
     *
     * @param[in] watchPoint Value to be watched
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument (e.g. the value to be watched is out of the limitation set in constructor)
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the same watch point has already been added
     *      - idf::ESPException(ESP_ERR_NOT_FOUND) if no more hardware watch point can be configured
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void addWatchPoints(int watchPoint);

    /**
     * @brief Remove a watch point for PCNT unit
     *
     * @param[in] watchPoint Watch point value
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the watch point was not added by `addWatchPoints()` yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void removeWatchPoints(int watchPoint);

    using EventCallBack = std::function<bool(const PulseCounter &, const pcnt_watch_event_data_t &)>;

    /**
     * @brief Set event callbacks for PCNT unit
     *
     * @note User registered callbacks are expected to be runnable within ISR context
     * @note The first call to this function needs to be before the call to `enable`
     * @note User can deregister a previously registered callback by calling this function and setting the callback member in the `cbs` structure to NULL.
     *
     * @param[in] unit PCNT unit handle created by `pcnt_new_unit()`
     * @param[in] cbs Group of callback functions
     * @param[in] user_data User data, which will be passed to callback functions directly
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not in init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void registerEventCallbacks(const EventCallBack & callback);

    /**
     * @brief Deregister a previously registered callback
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the timer is not in init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void unregisterEventCallbacks();

    /**
     * @brief Set glitch filter for PCNT unit
     *
     * @note The glitch filter module is clocked from APB, and APB frequency can be changed during DFS, which in return make the filter out of action.
     *       So this function will lazy-install a PM lock internally when the power management is enabled. With this lock, the APB frequency won't be changed.
     *       The PM lock can be uninstalled in `~PulseCounter()`.
     * @note This function should be called when the PCNT unit is in the init state (i.e. before calling `enable()`)
     *
     * @param[in] maxGlitchNs Pulse width smaller than this threshold will be treated as glitch and ignored, in the unit of ns
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument (e.g. glitch width is too big)
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not in the init state
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void setGlitchFilter(uint32_t maxGlitchNs);

    /**
     * @brief Enable the PCNT unit
     *
     * @note This function will transit the unit state from init to enable.
     * @note This function will enable the interrupt service, if it's lazy installed in `registerEventCallbacks()`.
     * @note This function will acquire the PM lock if it's lazy installed in `setGlitchFilter()`.
     * @note Enable a PCNT unit doesn't mean to start it. See also `start()` for how to start the PCNT counter.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if is already enabled
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void enable();

    /**
     * @brief Disable the PCNT unit
     *
     * @note This function will do the opposite work to the `enable()`
     * @note Disable a PCNT unit doesn't mean to stop it. See also `stop()` for how to stop the PCNT counter.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void disable();

    /**
     * @brief Start the PCNT unit, the counter will start to count according to the edge and/or level input signals
     *
     * @note This function should be called when the unit is in the enable state (i.e. after calling `enable()`)
     * @note This function is allowed to run within ISR context
     * @note This function will be placed into IRAM if `CONFIG_PCNT_CTRL_FUNC_IN_IRAM` is on, so that it's allowed to be executed when Cache is disabled
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void start();

    /**
     * @brief Stop PCNT from counting
     *
     * @note This function should be called when the unit is in the enable state (i.e. after calling `enable()`)
     * @note The stop operation won't clear the counter. Also see `clearCount()` for how to clear pulse count value.
     * @note This function is allowed to run within ISR context
     * @note This function will be placed into IRAM if `CONFIG_PCNT_CTRL_FUNC_IN_IRAM`, so that it is allowed to be executed when Cache is disabled
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if not enabled yet
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void stop();

    /**
     * @brief Get PCNT count value
     *
     * @note This function is allowed to run within ISR context
     * @note This function will be placed into IRAM if `CONFIG_PCNT_CTRL_FUNC_IN_IRAM`, so that it's allowed to be executed when Cache is disabled
     *
     * @return count value
     *
     * @note The internal hardware counter will be cleared to zero automatically when it reaches high or low limit.
     *       If you want to compensate for that count loss and extend the counter’s bit-width, you have to:
     *       - Set accumCount to true in counstructor
     *       - Add the high/low limit as the Watch Points.
     * 
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid argument
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    int getCount() const;

    /**
     * @brief Clear PCNT pulse count value to zero
     *
     * @note It's recommended to call this function after adding a watch point by `addWatchPoint()`, so that the newly added watch point is effective immediately.
     * @note This function is allowed to run within ISR context
     * @note This function will be placed into IRAM if `CONFIG_PCNT_CTRL_FUNC_IN_IRAM`, so that it's allowed to be executed when Cache is disabled
     *
     * @throw
     *      - idf::ESPException(ESP_FAIL) if other error
     */
    void clearCount();

    enum class State {INIT, ENABLE, RUN};

    /**
     * @brief Return the current state of this GPTimer
     */
    inline State getState() const {return _state;}

private:
    PulseCounter(const PulseCounter &) = delete;
    PulseCounter & operator=(const PulseCounter &) = delete;

    pcnt_unit_handle_t _unit;
    EventCallBack _callback;
    State _state;
};

} // idf

#endif // __cpp_exceptions
