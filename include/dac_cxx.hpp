#pragma once

#ifdef __cpp_exceptions

#include <driver/dac_oneshot.h>


namespace idf {

class DacOneshot
{
public:
    /**
     * @brief Allocate a new DAC oneshot channel
     * @note The channel will be enabled as well when the channel allocated
     *
     * @param[in] channel The channel to use
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if the input parameter is invalid
     *      - idf::ESPException(ESP_ERR_INVALID_STATE) if the DAC channel has been registered already
     *      - idf::ESPException(ESP_ERR_NO_MEM) if no memory for the DAC oneshot channel resources
     */
    DacOneshot(dac_channel_t channel);

    /**
     * @brief Delete the DAC oneshot channel
     * @note  The channel will be disabled as well when the channel deleted
     */
    ~DacOneshot();

    /**
     * @brief Output the voltage
     * @note  Generally it'll take 7~11 us on ESP32 and 10~21 us on ESP32-S2
     *
     * @param[in] digiValue The digital value that need to be converted
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if the input parameter is invalid
     */
    void setVoltageRaw(uint8_t valueRaw);

    /**
     * @brief Output the voltage
     * @note  Generally it'll take 7~11 us on ESP32 and 10~21 us on ESP32-S2
     *
     * @param[in] valueMlVolt The value in millivolt
     * @throw
     *      - idf::ESPException(ESP_ERR_INVALID_ARG) if the input parameter is invalid
     */
    void setVoltageMlVolt(uint16_t valueMlVolt);

private:
    DacOneshot(const DacOneshot &) = delete;
    DacOneshot & operator=(const DacOneshot &) = delete;

    dac_oneshot_handle_t _handle;
};

} // idf

#endif // __cpp_exceptions
