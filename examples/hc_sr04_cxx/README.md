# Example: HC_SR04 C++ example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates usage of the `CaptureTimer` and `CaptureChannel` C++ class in ESP-IDF width signals generated from a common HC-SR04 sonar sensor -- [HC-SR04](https://www.sparkfun.com/products/15569)..

The signal that HC-SR04 produces (and what can be handled by this example) is a simple pulse whose width indicates the measured distance. An excitation pulse is required to send to HC-SR04 on `Trig` pin to begin a new measurement. Then the pulse described above will appear on the `Echo` pin after a while.

Typical signals:

```
Trig       +-----+
           |     |
           |     |
      -----+     +-----------------------
Echo                   +-----+
                       |     |
                       |     |
      -----------------+     +-----------

 +--------------------------------------->
                Timeline
```

In this example, the `sdkconfig.defaults` file sets the `CONFIG_COMPILER_CXX_EXCEPTIONS` option. 
This enables both compile time support (`-fexceptions` compiler flag) and run-time support for C++ exception handling.
This is necessary for the C++ APIs.

## How to use example

### Hardware Required

* An ESP development board that features the MCPWM peripheral
* An HC-SR04 sensor module

Connection :

```
        +------+              +---------------------------------+
+-------+      |              |                                 |
|       |  VCC +--------------+ 5V                              |
+-------+      |              |                                 |
        + Echo +----=====>----+ GPIO32                          |
        |      |              |                                 |
        + Trig +----<=====----+ GPIO33                          |
+-------|      |              |                                 |
|       |  GND +--------------+ GND                             |
+-------|      |              |                                 |
        +------+              +---------------------------------+
```
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
I (354) main_task: Calling app_main()
I (354) gpio: GPIO[33]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (364) gpio: GPIO[32]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (374) hcSr04Example: Trig output
I (374) hcSr04Example: Measured distance: 0.55cm
I (884) hcSr04Example: Trig output
I (884) hcSr04Example: Measured distance: 0.29cm
I (1384) hcSr04Example: Trig output
I (1384) hcSr04Example: Measured distance: 29.85cm
I (1884) hcSr04Example: Trig output
I (1894) hcSr04Example: Measured distance: 29.86cm
I (2394) hcSr04Example: Trig output
```
