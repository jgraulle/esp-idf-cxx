# Example: Wifi C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `Wifi` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.
In this example, the `sdkconfig.defaults` file also sets `CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE`
to have enought place to fit the binary.

## How to use example

### Hardware Required

Any ESP32 family development board.

### Configure the project

```
idf.py menuconfig
```

### Build and Flash

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
...
I (756) wifi:wifi driver task: 3ffc1c38, prio:23, stack:6656, core=0
I (776) wifi:wifi firmware version: ce9244d
I (776) wifi:wifi certification version: v7.0
I (776) wifi:config NVS flash: enabled
I (776) wifi:config nano formating: disabled
I (786) wifi:Init data frame dynamic rx buffer num: 32
I (786) wifi:Init management frame dynamic rx buffer num: 32
I (796) wifi:Init management short buffer num: 32
I (796) wifi:Init dynamic tx buffer num: 32
I (806) wifi:Init static rx buffer size: 1600
I (806) wifi:Init static rx buffer num: 10
I (806) wifi:Init dynamic rx buffer num: 32
I (816) wifi_init: rx ba win: 6
I (816) wifi_init: tcpip mbox: 32
I (816) wifi_init: udp mbox: 6
I (826) wifi_init: tcp mbox: 6
I (826) wifi_init: tcp tx win: 5744
I (836) wifi_init: tcp rx win: 5744
I (836) wifi_init: tcp mss: 1440
I (836) wifi_init: WiFi IRAM OP enabled
I (846) wifi_init: WiFi RX IRAM OP enabled
W (2036) wifi:Cannot set pmf to required when in wpa-wpa2 mixed mode! Setting pmf to optional mode.
I (2046) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
I (2156) wifi:mode : softAP (xx:xx:xx:xx:xx:xx)
I (2156) wifi:Total power save buffer number: 16
I (2156) wifi:Init max length of beacon: 752/752
I (2166) wifi:Init max length of beacon: 752/752
I (2166) esp_netif_lwip: DHCP server started on interface WIFI_AP_DEF with IP: 192.168.4.1
I (66466) wifi:new:<1,1>, old:<1,1>, ap:<1,1>, sta:<255,255>, prof:1
I (66466) wifi:station: xx:xx:xx:xx:xx:xx join, AID=1, bgn, 40U
I (66586) esp_netif_lwip: DHCP server assigned IP to a client, IP is: 192.168.4.2
I (110806) wifi:station: xx:xx:xx:xx:xx:xx leave, AID = 1, bss_flags is 658531, bss:0x3ffcbad8
```
