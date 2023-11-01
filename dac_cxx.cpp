#ifdef __cpp_exceptions

#include "dac_cxx.hpp"

#include "esp_exception.hpp"

namespace idf {

DacOneshot::DacOneshot(dac_channel_t channel)
{
    dac_oneshot_config_t config = {
        .chan_id = channel
    };
    CHECK_THROW(dac_oneshot_new_channel(&config, &_handle));
}

DacOneshot::~DacOneshot()
{
    assert(dac_oneshot_del_channel(_handle) == ESP_OK);
    _handle = nullptr;
}

void DacOneshot::setVoltageRaw(uint8_t valueRaw)
{
    CHECK_THROW(dac_oneshot_output_voltage(_handle, valueRaw));
}

void DacOneshot::setVoltageMlVolt(uint16_t valueMlVolt)
{
    setVoltageRaw(valueMlVolt*255/3300);
}

} // idf

#endif // __cpp_exceptions
