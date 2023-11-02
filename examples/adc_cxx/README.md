# Example: ADC C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `AdcOneshotUnit` C++ class in ESP-IDF.

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

Any ESP32 family development board.

Connect DAC_2 (GPIO26) to ADC1_6 (GPIO34) preferably through a resistor (1k).

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
I (358) main_task: Calling app_main()
I (358) DAC-ADC_Example: Configure DAC
I (358) DAC-ADC_Example: Configure ADC
I (368) DAC-ADC_Example: Set voltage to 5 => 64mV
I (468) DAC-ADC_Example: Read raw value: 0 => 0mV
I (868) DAC-ADC_Example: Set voltage to 15 => 193mV
I (968) DAC-ADC_Example: Read raw value: 29 => 93mV
I (1368) DAC-ADC_Example: Set voltage to 25 => 322mV
I (1468) DAC-ADC_Example: Read raw value: 66 => 212mV
I (1868) DAC-ADC_Example: Set voltage to 35 => 451mV
I (1968) DAC-ADC_Example: Read raw value: 100 => 322mV
I (2368) DAC-ADC_Example: Set voltage to 45 => 580mV
I (2478) DAC-ADC_Example: Read raw value: 137 => 441mV
I (2878) DAC-ADC_Example: Set voltage to 55 => 708mV
I (2978) DAC-ADC_Example: Read raw value: 173 => 558mV
I (3378) DAC-ADC_Example: Set voltage to 65 => 837mV
```
