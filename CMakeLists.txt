idf_build_get_property(target IDF_TARGET)

set(srcs "esp_timer_cxx.cpp" "esp_exception.cpp" "gpio_cxx.cpp" "i2c_cxx.cpp" "spi_cxx.cpp"
         "spi_host_cxx.cpp" "gptimer_cxx.cpp" "pulse_counter_cxx.cpp" "mcpwm_cxx.cpp"
         "bdc_motor_cxx.cpp" "ledc_cxx.cpp" "wifi_cxx.cpp" "dac_cxx.cpp" "adc_cxx.cpp")
set(requires "esp_timer" "esp_wifi" "esp_adc")

if(NOT ${target} STREQUAL "linux")
    list(APPEND srcs
        "esp_event_api.cpp"
        "esp_event_cxx.cpp")
    list(APPEND requires "esp_event" "pthread")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "private_include"
                    PRIV_REQUIRES freertos driver
                    REQUIRES ${requires})
