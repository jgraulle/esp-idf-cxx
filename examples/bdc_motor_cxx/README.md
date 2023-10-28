# Example: Brushed DC motor C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `BdcMotor` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

Any ESP32 family development board.

For debug:
Connect 2 LED to the corresponding pin (default is pin 25 and 26). If the board has a normal LED already, you can use the pin number to which that one is connected.

For reel test:
Connect a H-bridge like a DRV8833 to corresponding pin (default is pin 25 and 26) with a Brushed DC motor to this H-bridge and a corresponding power supply.

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
I (356) main_task: Calling app_main()
I (356) gpio: GPIO[25]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (366) gpio: GPIO[26]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (376) BdcMotorTest: Forward
I (2376) BdcMotorTest: Reverse
I (4376) BdcMotorTest: Coast
I (6376) BdcMotorTest: Break
I (8376) BdcMotorTest: Forward
```
