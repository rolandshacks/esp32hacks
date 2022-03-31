//
// I2C
//
#pragma once

#include "driver/gpio.h"

#ifndef SIMULATOR
#include "esp32/rom/ets_sys.h"
#endif

#include <cstdint>

namespace sys {

class I2C {
   private:
    gpio_num_t scl_pin{GPIO_NUM_NC};
    gpio_num_t sda_pin{GPIO_NUM_NC};

   public:
    I2C(gpio_num_t scl, gpio_num_t sda);
    void init(uint8_t scl_pin, uint8_t sda_pin);
    bool start(void);
    void stop(void);
    bool write(uint8_t data);
    uint8_t read(void);
    void set_ack(bool ack);
};

}  // namespace sys
