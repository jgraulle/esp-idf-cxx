# ESP-IDF-C++

This project is forked from https://github.com/espressif/esp-idf-cxx and provides C++ wrapper classes around some components of [esp-idf](https://github.com/espressif/esp-idf). It is organized as a component for the [IDF component manager](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html). You can find this component at https://gitlab.cri.epita.fr/jeremie.graulle/esp-idf-cxx.

## *NOTE*
This component is in a beta-release phase. Some bits that are still missing (non-exhaustive list):
* GPIO interrupt
* Analog to Digital Converter (ADC)
* Wifi

## Requirements

* ESP-IDF and its requirements.
  Please follow the [ESP-IDF "Get Started" Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to download, install and setup esp-idf.

No other special requirements are necessary.

## Usage

In `main/idf_component.yml` file add `jgraulle/esp-idf-cxx:` and `git@gitlab.cri.epita.fr:jeremie.graulle/esp-idf-cxx.git`
Set up the IDF environment (i.e., `. ./export.sh` inside [esp-idf](https://github.com/espressif/esp-idf)) then run `idf.py reconfigure`.
