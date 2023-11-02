#ifdef __cpp_exceptions

#include "adc_cxx.hpp"

#include "esp_exception.hpp"

namespace idf {

AdcOneshot::AdcOneshot(adc_unit_t unit)
{
    adc_oneshot_unit_init_cfg_t config = {
        .unit_id = unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    CHECK_THROW(adc_oneshot_new_unit(&config, &_handle));
}

AdcOneshot::~AdcOneshot()
{
    assert(adc_oneshot_del_unit(_handle) == ESP_OK);
    _handle = nullptr;
}

void AdcOneshot::configure(adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth)
{
    adc_oneshot_chan_cfg_t config = {
        .atten = atten,
        .bitwidth = bitwidth,
    };
    int bitwithInt = bitwidth;
    if (bitwidth == ADC_BITWIDTH_DEFAULT)
        bitwithInt = ADC_BITWIDTH_12;
    _maxRawValue[channel] = 1<<bitwithInt;
    CHECK_THROW(adc_oneshot_config_channel(_handle, channel, &config));
}

std::pair<adc_unit_t, adc_channel_t> AdcOneshot::ioToChannel(int ioNum)
{
    adc_unit_t unitId;
    adc_channel_t channel;
    CHECK_THROW(adc_oneshot_io_to_channel(ioNum, &unitId, &channel));
    return std::make_pair(unitId, channel);
}

int AdcOneshot::readRaw(adc_channel_t channel)
{
    int rawValue;
    CHECK_THROW(adc_oneshot_read(_handle, channel, &rawValue));
    return rawValue;
}

int AdcOneshot::readMlVolt(adc_channel_t channel)
{
    int raw = readRaw(channel);
    return raw*3300/_maxRawValue.at(channel);
}

} // idf

#endif // __cpp_exceptions
