#pragma once

#ifdef __cpp_exceptions

#include "esp_wifi_types.h"
#include "esp_netif_types.h"
#include <string>
#include <semaphore>
#include <iostream>
#include <memory>

namespace idf::event {class ESPEventLoop;}
namespace idf::event {class ESPEventReg;}
typedef struct esp_netif_obj esp_netif_t;


namespace idf {

class Wifi
{
public:
    /**
     * @brief  Initialize WiFi
     *         Allocate resource for WiFi driver, such as WiFi control structure, RX/TX buffer,
     *         WiFi NVS structure etc. This WiFi also starts WiFi task
     *
     * @attention 1. This API must be called before all other WiFi API can be called
     * @attention 2. Always use WIFI_INIT_CONFIG_DEFAULT macro to initialize the configuration to default values, this can
     *               guarantee all the fields get correct value when more fields are added into wifi_init_config_t
     *               in future release. If you want to set your own initial values, overwrite the default values
     *               which are set by WIFI_INIT_CONFIG_DEFAULT. Please be notified that the field 'magic' of
     *               wifi_init_config_t should always be WIFI_INIT_CONFIG_MAGIC!
     *
     * @param config pointer to WiFi initialized configuration structure; can point to a temporary variable.
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_NO_MEM): out of memory
     *    - idf::ESPException(others): refer to error code esp_err.h
     */
    Wifi(idf::event::ESPEventLoop & eventLoop);

    /**
     * @brief  Deinit WiFi
     *         Free all resource allocated in esp_wifi_init and stop WiFi task
     *
     * @attention 1. This API should be called if you want to remove WiFi driver from the system
     */
    ~Wifi();

    /**
     * @brief     Set the WiFi operating mode
     *
     *            Set the WiFi operating mode as station, soft-AP, station+soft-AP or NAN.
     *            The default mode is station mode.
     *
     * @param     mode  WiFi operating mode
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_INVALID_ARG): invalid argument
     *    - idf::ESPException(others): refer to error code in esp_err.h
     */
    void setMode(wifi_mode_t mode);

    /**
     * @brief Get current operating mode of WiFi
     *
     * @return current WiFi mode
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_INVALID_ARG): invalid argument
     */
    wifi_mode_t getMode() const;

    /**
     * @brief  Start WiFi according to current configuration
     *         If mode is WIFI_MODE_STA, it creates station control block and starts station
     *         If mode is WIFI_MODE_AP, it creates soft-AP control block and starts soft-AP
     *         If mode is WIFI_MODE_APSTA, it creates soft-AP and station control block and starts soft-AP and station
     *         If mode is WIFI_MODE_NAN, it creates NAN control block and starts NAN
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_INVALID_ARG): invalid argument
     *    - idf::ESPException(ESP_ERR_NO_MEM): out of memory
     *    - idf::ESPException(ESP_ERR_WIFI_CONN): WiFi internal error, station or soft-AP control block wrong
     *    - idf::ESPException(ESP_FAIL): other WiFi internal errors
     */
    void start();

    /**
     * @brief  Stop WiFi
     *         If mode is WIFI_MODE_STA, it stops station and frees station control block
     *         If mode is WIFI_MODE_AP, it stops soft-AP and frees soft-AP control block
     *         If mode is WIFI_MODE_APSTA, it stops station/soft-AP and frees station/soft-AP control block
     *         If mode is WIFI_MODE_NAN, it stops NAN and frees NAN control block
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     */
    void stop();

    /**
     * @brief  Restore WiFi stack persistent settings to default values
     *
     * This function will reset settings made using the following APIs:
     * - esp_wifi_set_bandwidth,
     * - esp_wifi_set_protocol,
     * - esp_wifi_set_config related
     * - esp_wifi_set_mode
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     */
    void restore();

    /**
     * @brief Set the configuration for Station mode
     *
     * @attention 1. This API can be called only when specified interface is enabled, otherwise, API fail
     * @attention 2. The configuration will be stored in NVS for station and soft-AP
     *
     * @param ssid SSID of target AP (32 char max)
     * @param password Password of target AP (64 char max)
     * @param authmodeThreshold The weakest authmode to accept in the fast scan mode
     *      Note: Incase this value is not set and password is set as per WPA2 standards(password len >= 8),
     *      it will be defaulted to WPA2 and device won't connect to deprecated WEP/WPA networks.
     *      Please set authmode threshold as WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK to connect to WEP/WPA networks
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_INVALID_ARG): invalid argument
     *    - idf::ESPException(ESP_ERR_WIFI_IF): invalid interface
     *    - idf::ESPException(ESP_ERR_WIFI_MODE): invalid mode
     *    - idf::ESPException(ESP_ERR_WIFI_PASSWORD): invalid password
     *    - idf::ESPException(ESP_ERR_WIFI_NVS): WiFi internal NVS error
     *    - idf::ESPException(others): refer to the erro code in esp_err.h
     */
    void setConfigSta(const std::string & ssid, const std::string & password, wifi_auth_mode_t authmodeThreshold = WIFI_AUTH_OPEN);

    /**
     * @brief Set the configuration for Access Point mode (aka Soft-AP mode)
     *
     * @attention 1. This API can be called only when specified interface is enabled, otherwise, API fail
     * @attention 2. The configuration will be stored in NVS for station and soft-AP
     *
     * @param ssid SSID of target AP (32 char max)
     * @param password Password of target AP (64 char max)
     * @param authmodeThreshold The weakest authmode to accept in the fast scan mode
     *      Note: Incase this value is not set and password is set as per WPA2 standards(password len >= 8),
     *      it will be defaulted to WPA2 and device won't connect to deprecated WEP/WPA networks.
     *      Please set authmode threshold as WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK to connect to WEP/WPA networks
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_INVALID_ARG): invalid argument
     *    - idf::ESPException(ESP_ERR_WIFI_IF): invalid interface
     *    - idf::ESPException(ESP_ERR_WIFI_MODE): invalid mode
     *    - idf::ESPException(ESP_ERR_WIFI_PASSWORD): invalid password
     *    - idf::ESPException(ESP_ERR_WIFI_NVS): WiFi internal NVS error
     *    - idf::ESPException(others): refer to the erro code in esp_err.h
     */
    void setConfigAp(const std::string & ssid, uint8_t channel, const std::string & password, uint8_t maxConnection, wifi_auth_mode_t authmode);

    /**
     * @brief Block until connected to the AP
     */
    void waitConnected();

    /**
     * @brief Block until a IP address is affected to this device
     *
     * @return IP, mask and gateway address
     */
    esp_netif_ip_info_t waitIp();

private:
    /**
     * @brief     Connect WiFi station to the AP.
     *
     * @attention 1. This API only impact WIFI_MODE_STA or WIFI_MODE_APSTA mode
     * @attention 2. If station interface is connected to an AP, call esp_wifi_disconnect to disconnect.
     * @attention 3. The scanning triggered by esp_wifi_scan_start() will not be effective until connection between device and the AP is established.
     *               If device is scanning and connecting at the same time, it will abort scanning and return a warning message and error
     *               number ESP_ERR_WIFI_STATE.
     *               If you want to do reconnection after device received disconnect event, remember to add the maximum retry time, otherwise the called
     *               scan will not work. This is especially true when the AP doesn't exist, and you still try reconnection after device received disconnect
     *               event with the reason code WIFI_REASON_NO_AP_FOUND.
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi is not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_STARTED): WiFi is not started by esp_wifi_start
     *    - idf::ESPException(ESP_ERR_WIFI_CONN): WiFi internal error, station or soft-AP control block wrong
     *    - idf::ESPException(ESP_ERR_WIFI_SSID): SSID of AP which station connects is invalid
     */
    void connect();

    /**
     * @brief     Disconnect WiFi station from the AP.
     *
     * @throw
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_INIT): WiFi was not initialized by esp_wifi_init
     *    - idf::ESPException(ESP_ERR_WIFI_NOT_STARTED): WiFi was not started by esp_wifi_start
     *    - idf::ESPException(ESP_FAIL): other WiFi internal errors
     */
    void disconnect();

    Wifi(const Wifi &) = delete;
    Wifi & operator=(const Wifi &) = delete;

    idf::event::ESPEventLoop & _eventLoop;
    esp_netif_t * _netifSta;
    esp_netif_t * _netifAp;
    std::unique_ptr<idf::event::ESPEventReg> _regEventWifi;
    std::binary_semaphore _connectedNotifier;
    std::unique_ptr<idf::event::ESPEventReg> _regEventIp;
    std::binary_semaphore _ipNotifier;
    esp_netif_ip_info_t _ipInfo;
};

} // idf

std::ostream & operator<<(std::ostream & os, const esp_ip4_addr_t & ip);

#endif // __cpp_exceptions
