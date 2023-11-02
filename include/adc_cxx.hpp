#pragma once

#ifdef __cpp_exceptions

#include <esp_adc/adc_oneshot.h>
#include <utility>
#include <map>


namespace idf {

class AdcOneshot
{
public:
    /**
     * @brief Create a handle to a specific ADC unit
     *
     * @note This API is thread-safe. For more details, see ADC programming guide
     *
     * @param[in] unit Unit to allocate
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid arguments
     *        - idf::ESPException(ESP_ERR_NO_MEM) if no memory
     *        - idf::ESPException(ESP_ERR_NOT_FOUND) if the ADC peripheral to be claimed is already in use
     *        - idf::ESPException(ESP_FAIL) if lock source isn't initialised correctly
     */
    AdcOneshot(adc_unit_t unit);

    /**
     * @brief Delete the ADC unit handle
     *
     * @note This API is thread-safe. For more details, see ADC programming guide
     */
    ~AdcOneshot();

    /**
     * @brief Set ADC oneshot mode required configurations
     *
     * @note This API is thread-safe. For more details, see ADC programming guide
     *
     * @param[in] channel ADC channel to be configured
     * @param[in] atten ADC attenuation
     * @param[in] bitwidth ADC conversion result bits
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) if invalid arguments
     */
    void configure(adc_channel_t channel, adc_atten_t atten = ADC_ATTEN_DB_11,
            adc_bitwidth_t bitwidth = ADC_BITWIDTH_DEFAULT);

    /**
     * @brief Get ADC channel from the given GPIO number
     *
     * @param[in] ioNum     GPIO number
     * 
     * @return a std::pair with ADC unit as first and ADC channel as second
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) If invalid argument
     *        - idf::ESPException(ESP_ERR_NOT_FOUND) If the IO is not a valid ADC pad
     */
    static std::pair<adc_unit_t, adc_channel_t> ioToChannel(int ioNum);

    /**
     * @brief Get one ADC conversion raw result
     *
     * @note This API is thread-safe. For more details, see ADC programming guide
     * @note This API should NOT be called in an ISR context
     *
     * @param[in] channel ADC channel
     * @return ADC conversion raw result
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) If invalid arguments
     *        - idf::ESPException(ESP_ERR_TIMEOUT) If timeout, the ADC result is invalid
     */
    int readRaw(adc_channel_t channel);

    /**
     * @brief Get one ADC conversion millivolt result
     *
     * @note This API is thread-safe. For more details, see ADC programming guide
     * @note This API should NOT be called in an ISR context
     *
     * @param[in] channel ADC channel
     * @return ADC conversion millivolt result
     *
     * @throw
     *        - idf::ESPException(ESP_ERR_INVALID_ARG) If invalid arguments
     *        - idf::ESPException(ESP_ERR_TIMEOUT) If timeout, the ADC result is invalid
     */
    int readMlVolt(adc_channel_t channel);

private:
    AdcOneshot(const AdcOneshot &) = delete;
    AdcOneshot & operator=(const AdcOneshot &) = delete;

    adc_oneshot_unit_handle_t _handle;
    std::map<adc_channel_t, int> _maxRawValue;
};

} // idf

#endif // __cpp_exceptions
