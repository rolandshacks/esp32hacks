//
// I2C
//
#pragma once
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"

namespace oled {

class I2C {
   private:
    gpio_num_t scl_pin, sda_pin;

   public:
    I2C(gpio_num_t scl, gpio_num_t sda);
    void init(uint8_t scl_pin, uint8_t sda_pin);
    bool start(void);
    void stop(void);
    bool write(uint8_t data);
    uint8_t read(void);
    void set_ack(bool ack);
};

} // namespace
