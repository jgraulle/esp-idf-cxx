# Example: DAC C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `DacOneshot` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

Any ESP32 family development board.

Connect a voltmeter to the DAC_2 (GPIO26).

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
I (347) main_task: Calling app_main()
I (347) DAC_Example: Configure DAC
I (347) DAC_Example: Set voltage to 1.65V
```
