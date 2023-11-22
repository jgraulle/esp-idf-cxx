# Example: LED controleur C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `LedcTimer` and `LedcChannel` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

Any ESP32 family development board.

Connect a buzzer behind a transistor or a logic analyser to the GPIO16.

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
I (436) main_task: Calling app_main()
I (436) Ledc_Example: Configure LED controleur
I (436) Ledc_Example: Play a 440Hz sound for 1s
I (1446) Ledc_Example: Pause for 1s
I (2446) Ledc_Example: Play a 880Hz sound for 1s
I (3446) Ledc_Example: Pause
```
