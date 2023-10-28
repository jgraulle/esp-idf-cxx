# Example: PulseCounter C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `PulseCounter` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

Any ESP32 family development board.

Connect an switch to the corresponding pin (default is pin 0). If the board has a normal switch already like boot button at pin 0, you can use the pin number to which that one is connected.

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
I (339) cpu_start: Starting scheduler.
I (343) gpio: GPIO[0]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0
pulse counter = 0
pulse counter = 2
pulse counter = 2
pulse counter = 3
pulse counter = 4
pulse counter = 5
pulse counter = 6
pulse counter = 7
pulse counter = 8
pulse counter = 9
interruption pulse counter = 10
pulse counter = 10
pulse counter = 11
```
