/* Wifi C++ Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_exception.hpp"
#include "wifi_cxx.hpp"
#include "esp_event_cxx.hpp"

#include "nvs_flash.h"

#include <thread>
#include <iostream>

using namespace std::chrono_literals;


extern "C" void app_main(void)
{
    try
    {
        // Initialize NVS
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        // Init event loop
        idf::event::ESPEventLoop eventLoop;

        // Initialize Wifi
        idf::Wifi wifi(eventLoop);
#define WIFI_AP
#ifdef WIFI_AP
        wifi.setMode(WIFI_MODE_AP);
        wifi.setConfigAp(CONFIG_ESP_WIFI_SSID, 1, CONFIG_ESP_WIFI_PASSWORD, 4, WIFI_AUTH_WPA_WPA2_PSK);
#else
        wifi.setMode(WIFI_MODE_STA);
        wifi.setConfigSta(CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD, WIFI_AUTH_WPA_WPA2_PSK);
#endif
        wifi.start();
#ifndef WIFI_AP
        wifi.waitConnected();
        auto ipInfo = wifi.waitIp();
        std::cout << "My IP : " << ipInfo.ip << std::endl;
#endif

        while (true)
        {
            std::this_thread::sleep_for(1s);
        }
    }
    catch (idf::ESPException & e)
    {
        std::cout << "Exception occurred: " << esp_err_to_name(e.error) << std::endl
                 << "stopping." << std::endl;
    }
}
