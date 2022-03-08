# ESP32 OLED Demo

This is a OLED/I2C graphics library and simple demo for ESP32.
I2C code and oled data transfer has been optimized.

## Setup

Make sure you have your "esp-idf" sdk setup correctly and have run the
"export.sh" or "export.bat" scripts to setup the environment variables.

## Build

CMake (and Ninja) is used to build the project.

Build just the app: `idf.py app`

Build all: `idf.py all`

## Flash

Flash binary to device: `idf.py flash`

## Dependencies

SSD1306 I2C library for ESP32: https://github.com/imxieyi/esp32-i2c-ssd1306-oled
THANKS for the great work!
