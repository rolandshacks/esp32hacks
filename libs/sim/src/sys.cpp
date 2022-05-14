//
// ESP32 System
//

#include "sim/sim.h"
#include "sim/ssd1306.h"

#include "driver/i2c.h"

#include <cassert>
#include <cstdint>

extern EmuSSD1306 emu1306;

using namespace sys;

#define ESP_OK 0
#define ESP_FAIL -1

static i2c_port_t i2c_port = 0x0;
static const size_t i2c_buffer_size = 4096;
static uint8_t i2c_buffer[i2c_buffer_size] = {};
static size_t i2c_buffer_ofs = 0;
static const size_t i2c_buffer_max_write_len = 32;

void esp_restart(void) {
    auto sim = simulator::Sim::instance();
    if (nullptr != sim) {
        sim->restart();
    }
}

esp_err_t i2c_param_config(i2c_port_t i2c_num, const i2c_config_t *i2c_conf) {
    i2c_port = i2c_num;
    return ESP_OK;
}

esp_err_t i2c_driver_install(i2c_port_t i2c_num, i2c_mode_t mode, size_t slv_rx_buf_len, size_t slv_tx_buf_len, int intr_alloc_flags) {
    return ESP_OK;
}

i2c_cmd_handle_t i2c_cmd_link_create(void) {
    return &i2c_port;
}

void i2c_cmd_link_delete(i2c_cmd_handle_t cmd_handle) {
}

esp_err_t i2c_master_start(i2c_cmd_handle_t cmd_handle) {
    if (nullptr == cmd_handle) return ESP_FAIL;

    if (0 != i2c_buffer_ofs) {
        i2c_buffer_ofs = 0;
        return ESP_FAIL;
    }
    i2c_buffer_ofs = 0;
    return ESP_OK;
}

esp_err_t i2c_master_stop(i2c_cmd_handle_t cmd_handle) {
    if (nullptr == cmd_handle) return ESP_FAIL;

    auto device = simulator::Sim::instance()->getDisplayDevice();

    size_t i2c_buffer_usage = i2c_buffer_ofs;
    i2c_buffer_ofs = 0;

    if (i2c_buffer_usage < 2) {
        device->onCommand(nullptr, 0);
        return ESP_FAIL;
    }

    size_t ofs = 0;

    uint8_t slave_address = i2c_buffer[ofs++];  // slave address
    if (0x0 == slave_address) return ESP_FAIL;  // validate slave address
    if (ofs >= i2c_buffer_usage) return ESP_FAIL;

    uint8_t flags = i2c_buffer[ofs++];              // read flags
    bool control_switch = ((flags & 0x80) != 0x0);  // control switch
    if (control_switch) return ESP_OK;
    if (ofs >= i2c_buffer_usage) return ESP_FAIL;

    bool data_or_command_switch = ((flags & 0x40) != 0x0);  // data or command switch
    if (data_or_command_switch) {
        device->onData(i2c_buffer + ofs, i2c_buffer_usage - ofs);
    } else {
        device->onCommand(i2c_buffer + ofs, i2c_buffer_usage - ofs);
    }

    return ESP_OK;
}

esp_err_t i2c_master_write_byte(i2c_cmd_handle_t cmd_handle, uint8_t data, bool ack_en) {
    if (nullptr == cmd_handle) return ESP_FAIL;
    if (i2c_buffer_ofs >= i2c_buffer_size) {
        return ESP_FAIL;
    }
    i2c_buffer[i2c_buffer_ofs++] = data;
    return ESP_OK;
}

esp_err_t i2c_master_write(i2c_cmd_handle_t cmd_handle, const uint8_t *data, size_t data_len, bool ack_en) {
    assert(data_len <= i2c_buffer_max_write_len);

    if (nullptr == cmd_handle) return ESP_FAIL;
    size_t i = 0;
    while (i<data_len) {
        if (i > i2c_buffer_max_write_len) {
            break;
        }

        if (ESP_OK != i2c_master_write_byte(cmd_handle, *(data + i), ack_en)) {
            return ESP_FAIL;
        }
        i++;
    }
    return ESP_OK;
}

esp_err_t i2c_master_cmd_begin(i2c_port_t i2c_num, i2c_cmd_handle_t cmd_handle, TickType_t ticks_to_wait) {
    if (i2c_num != i2c_port || nullptr == cmd_handle) {
        return ESP_FAIL;
    }
    return ESP_OK;
}
