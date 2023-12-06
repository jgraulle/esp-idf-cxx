#ifdef __cpp_exceptions

#include "wifi_cxx.hpp"

#include "esp_wifi.h"
#include "esp_exception.hpp"
#include "esp_event_cxx.hpp"

#include <cstring>
#include <pthread.h>


namespace idf {

Wifi::Wifi(idf::event::ESPEventLoop & eventLoop)
    : _eventLoop(eventLoop)
    , _netifSta(nullptr)
    , _netifAp(nullptr)
    , _regEventWifi()
    , _connectedNotifier(0)
    , _regEventIp()
    , _ipNotifier(0)
    , _ipInfo()
{
    static pthread_once_t netifInit = PTHREAD_ONCE_INIT;
    pthread_once(&netifInit, [](){CHECK_THROW(esp_netif_init());}); // TODO try to use std::call_once instead

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    CHECK_THROW(esp_wifi_init(&config));
}

Wifi::~Wifi()
{
    esp_netif_destroy_default_wifi(_netifSta);
    esp_netif_destroy_default_wifi(_netifAp);
    if (_netifSta != nullptr)
    {
       esp_netif_destroy(_netifSta);
        _netifSta = nullptr;
    }
    if (_netifAp != nullptr)
    {
        esp_netif_destroy(_netifAp);
        _netifAp = nullptr;
    }
    assert(esp_wifi_deinit()==ESP_OK);
}

void Wifi::setMode(wifi_mode_t mode)
{
    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
        _netifSta = esp_netif_create_default_wifi_sta();
    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        _netifAp = esp_netif_create_default_wifi_ap();

    CHECK_THROW(esp_wifi_set_mode(mode));
}

wifi_mode_t Wifi::getMode() const
{
    wifi_mode_t mode;
    CHECK_THROW(esp_wifi_get_mode(&mode));
    return mode;
}

void Wifi::start()
{
    if (_netifSta != nullptr)
    {
        const idf::event::ESPEvent EVENT_WIFI(WIFI_EVENT, idf::event::ESPEventID(ESP_EVENT_ANY_ID));
        _regEventWifi = _eventLoop.register_event(EVENT_WIFI,
            [this](const idf::event::ESPEvent & event, void * data) {
                if (event.id.get_id() == WIFI_EVENT_STA_START)
                    connect();
                else if (event.id.get_id() == WIFI_EVENT_STA_DISCONNECTED)
                    std::cout << "Connect to the AP fail" << std::endl;
                else if (event.id.get_id() == WIFI_EVENT_STA_CONNECTED)
                    _connectedNotifier.release();
            });
        const idf::event::ESPEvent EVENT_IP(IP_EVENT, idf::event::ESPEventID(IP_EVENT_STA_GOT_IP));
        _regEventIp = _eventLoop.register_event(EVENT_IP,
            [&isIpReceive = _ipNotifier, &ipInfo = _ipInfo](const idf::event::ESPEvent & event, void * data) {
                ip_event_got_ip_t * eventDataIp = (ip_event_got_ip_t*) data;
                ipInfo = eventDataIp->ip_info;
                isIpReceive.release();
            });
    }

    CHECK_THROW(esp_wifi_start());
}

void Wifi::stop()
{
    CHECK_THROW(esp_wifi_stop());
}

void Wifi::restore()
{
    CHECK_THROW(esp_wifi_restore());
}

void Wifi::setConfigSta(const std::string & ssid, const std::string & password, wifi_auth_mode_t authmodeThreshold)
{
    wifi_config_t config = {.sta = {}};
    strncpy(reinterpret_cast<char *>(config.sta.ssid), ssid.c_str(), sizeof(config.sta.ssid));
    strncpy(reinterpret_cast<char *>(config.sta.password), password.c_str(), sizeof(config.sta.password));
    config.sta.threshold.authmode = authmodeThreshold;

    CHECK_THROW(esp_wifi_set_config(WIFI_IF_STA, &config));
}

void Wifi::setConfigAp(const std::string & ssid, uint8_t channel, const std::string & password, uint8_t maxConnection, wifi_auth_mode_t authmode)
{
    wifi_config_t config;
    strncpy(reinterpret_cast<char *>(config.ap.ssid), ssid.c_str(), sizeof(config.sta.ssid));
    config.ap.ssid_len = ssid.size();
    config.ap.channel = channel;
    strncpy(reinterpret_cast<char *>(config.ap.password), password.c_str(), sizeof(config.sta.password));
    config.ap.max_connection = maxConnection;
    config.ap.authmode = authmode;
    config.ap.pmf_cfg.required = true;

    CHECK_THROW(esp_wifi_set_config(WIFI_IF_AP, &config));
}

void Wifi::waitConnected()
{
    _connectedNotifier.acquire();
    _regEventWifi.release();
}

esp_netif_ip_info_t Wifi::waitIp()
{
    _ipNotifier.acquire();
    _regEventIp.release();
    return _ipInfo;
}

void Wifi::connect()
{
    CHECK_THROW(esp_wifi_connect());
}

void Wifi::disconnect()
{
    CHECK_THROW(esp_wifi_disconnect());
}

} // idf

std::ostream & operator<<(std::ostream & os, const esp_ip4_addr_t & ip)
{
    os << (int)esp_ip4_addr1_16(&ip) << "." << (int)esp_ip4_addr2_16(&ip)
            << "." << (int)esp_ip4_addr3_16(&ip) << "." << (int)esp_ip4_addr4_16(&ip);
    return os;
}

#endif // __cpp_exceptions
