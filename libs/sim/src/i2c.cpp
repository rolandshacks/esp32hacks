//
// I2C
//

#include "sys/i2c.h"

#include <cstdint>

#include "sim/sim.h"
#include "sim/ssd1306.h"

extern EmuSSD1306 emu1306;

using namespace sys;

static const size_t ic2_buffer_size = 32768;
static uint8_t ic2_buffer[ic2_buffer_size] = {};
static size_t ic2_buffer_ofs = {0};

I2C::I2C(gpio_num_t scl, gpio_num_t sda) {}

bool I2C::start(void) {
    if (0 != ic2_buffer_ofs) {
        ic2_buffer_ofs = 0;
        return false;
    }
    ic2_buffer_ofs = 0;
    return true;
}

void I2C::stop(void) {
    auto device = simulator::Sim::instance()->getDisplayDevice();

    size_t ic2_buffer_usage = ic2_buffer_ofs;
    ic2_buffer_ofs = 0;

    if (ic2_buffer_usage < 2) {
        device->onCommand(nullptr, 0);
        return;
    }

    size_t ofs = 0;

    uint8_t slave_address = ic2_buffer[ofs++];  // slave address
    if (0x78 != slave_address) return;  // check for default slave address
    if (ofs >= ic2_buffer_usage) return;

    uint8_t flags = ic2_buffer[ofs++];              // read flags
    bool control_switch = ((flags & 0x80) != 0x0);  // control switch
    if (control_switch) return;
    if (ofs >= ic2_buffer_usage) return;

    bool data_or_command_switch = ((flags & 0x40) != 0x0);  // data or command switch
    if (data_or_command_switch) {
        device->onData(ic2_buffer + ofs, ic2_buffer_usage - ofs);
    } else {
        device->onCommand(ic2_buffer + ofs, ic2_buffer_usage - ofs);
    }
}

// return: true - ACK; false - NACK
bool I2C::write(uint8_t data) {
    if (ic2_buffer_ofs >= ic2_buffer_size) {
        return false;
    }

    ic2_buffer[ic2_buffer_ofs++] = data;

    return true;
}

uint8_t I2C::read(void) {
    // unused, not implemented
    return 0x0;
}

void I2C::set_ack(bool ack) {
    // unused, not implemented
}
