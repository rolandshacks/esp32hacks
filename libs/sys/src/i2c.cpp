//
// I2C
//

#include "sys/i2c.h"

#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include <cstddef>
#include <cstdint>
#include <algorithm>

#define TAG "i2c"

using namespace sys;

// Standard: 100000Hz, Highspeed: 400000Hz, or beyond...
//   Freq limitation when using different clock sources (taken from driver/i2c.c):)
//   I2C_CLK_LIMIT_REF_TICK  => 50000    Limited by REF_TICK, no more than REF_TICK/20
//   I2C_CLK_LIMIT_APB       => 4000000  Limited by APB, no more than APB/20
//   I2C_CLK_LIMIT_RTC       => 1000000  Limited by RTC, no more than RTC/20
//   I2C_CLK_LIMIT_XTAL      => 2000000  Limited by RTC, no more than XTAL/20
static const uint32_t i2c_frequency = 1200000; // Hz (1.2MHz works with SSD1306)

#ifndef SIMULATOR
//#define USE_DIRECT_GPIO 1 // legacy mode: direct GPIO access
#endif

// ############################################################################
// Construction
// ############################################################################

I2C::I2C(int scl, int sda, uint8_t address) :
    scl_pin_{scl},
    sda_pin_{sda},
    address_{address},
    port_{0}
{
    ESP_LOGI(TAG,
             "init sda=%d, scl=%d, addr=0x%02x, port=%d",
             sda_pin_, scl_pin_, (int) address_, (int) port_);
    init();
}

I2C::~I2C() {
}

// ############################################################################
// High-level functionality
// ############################################################################

bool I2C::sendControl(uint8_t control) {
    return sendRaw(control, 0x0); // Co = 0, D/C = 0
}

bool I2C::sendControlBuffer(const uint8_t* buffer, size_t buffer_size) {
    return sendBufferRaw(buffer, buffer_size, 0x0);  // Co = 0, D/C = 0
}

bool I2C::sendDataBuffer(const uint8_t* buffer, size_t buffer_size) {
    return sendBufferRaw(buffer, buffer_size, 0x40);  // Co = 0, D/C = 1
}

bool I2C::sendData(uint8_t data) {
    return sendRaw(data, 0x40); // Co = 0, D/C = 1
}

bool I2C::sendBufferRaw(const uint8_t* buffer, size_t buffer_size, int flags) {

    if (nullptr == buffer || 0 == buffer_size) {
        return false;
    }

    size_t k = 0;

    size_t max_bytes = I2C::IC2_MAX_LIST_LEN-1;  // max = sz - address byte
    if (flags >= 0) max_bytes--;                 // and flag byte

    while (k < buffer_size) {
        begin();
        start();
        if (false == write(address_)) {
            stop();
            end();
            return false;
        }

        if (flags >= 0) write((uint8_t) flags);

        size_t bytes_to_send = std::min(buffer_size-k, max_bytes);
        size_t bytes_written = write(buffer + k, bytes_to_send);
        if (0 == bytes_written) {
            stop();
            end();
            return false;
        }

        k += bytes_written;

        stop();
        end();
    }

    return true;
}

bool I2C::sendRaw(uint8_t value, int flags) {
    begin();
    start();
    bool ret = write(address_);
    if (!ret) {  // NACK
        stop();
        end();
        return false;
    }
    if (flags >= 0) write((uint8_t) flags);
    write(value);
    stop();
    end();
    return true;
}

bool I2C::sendEmpty() {
    begin();
    start();
    bool ret = write(address_);
    stop();
    end();
    return ret;
}

// ############################################################################
// Low level functionality
// ############################################################################

#if (USE_DIRECT_GPIO)

static inline gpio_num_t get_gpio_num(int pin) {
    return static_cast<gpio_num_t>(pin);
}

#define _SDA1 gpio_set_level(get_gpio_num(sda_pin_), 1)
#define _SDA0 gpio_set_level(get_gpio_num(sda_pin_), 0)

#define _SCL1 gpio_set_level(get_gpio_num(scl_pin_), 1)
#define _SCL0 gpio_set_level(get_gpio_num(scl_pin_), 0)

#define _SDAX gpio_get_level(get_gpio_num(sda_pin_))
#define _SCLX gpio_get_level(get_gpio_num(scl_pin_))

//#define _DELAY portDISABLE_INTERRUPTS();ets_delay_us(1);portENABLE_INTERRUPTS()
#define _DELAY ets_delay_us(1)

bool I2C::init() {

    auto sda_pin = get_gpio_num(sda_pin_);
    auto scl_pin = get_gpio_num(scl_pin_);

    gpio_set_pull_mode(scl_pin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(sda_pin, GPIO_PULLUP_ONLY);

    gpio_set_direction(scl_pin, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(sda_pin, GPIO_MODE_INPUT_OUTPUT);

    // I2C bus idle state.
    gpio_set_level(scl_pin, 1);
    gpio_set_level(sda_pin, 1);

    return true;
}


bool I2C::begin() {
    return true;
}

bool I2C::end() {
    return true;
}

bool I2C::start(void) {
    _SDA1;
    _SCL1;
    _DELAY;
    if (_SDAX == 0) return false;  // Bus busy
    _SDA0;
    _DELAY;
    _SCL0;
    return true;
}

void I2C::stop(void) {
    _SDA0;
    _SCL1;
    _DELAY;
    while (_SCLX == 0)
        ;  // clock stretching
    _SDA1;
    _DELAY;
}

bool I2C::write(uint8_t data) {
    // return: true - ACK; false - NACK

    uint8_t ibit;
    bool ret;

    for (ibit = 0; ibit < 8; ++ibit) {
        if (data & 0x80)
            _SDA1;
        else
            _SDA0;
        _DELAY;
        _SCL1;
        _DELAY;
        data = data << 1;
        _SCL0;
    }
    _SDA1;
    _DELAY;
    _SCL1;
    _DELAY;
    ret = (_SDAX == 0);
    _SCL0;
    _DELAY;

    return ret;
}

size_t I2C::write(const uint8_t* data, size_t sz) {
    size_t bytes_to_write = std::min(sz, sys::I2C::IC2_MAX_LIST_LEN-1);
    for (auto i=0; i<bytes_to_write; i++) {
        if (false == write(*(data + i))) {
            return 0;
        }
    }
    return sz;
}

uint8_t I2C::read(void) {
    uint8_t data = 0;
    uint8_t ibit = 8;

    _SDA1;
    while (ibit--) {
        data = data << 1;
        _SCL0;
        _DELAY;
        _SCL1;
        _DELAY;
        if (_SDAX)
            data = data | 0x01;
    }
    _SCL0;

    return data;
}

void I2C::setAck(bool ack) {
    _SCL0;
    if (ack)
        _SDA0;  // ACK
    else
        _SDA1;  // NACK
    _DELAY;
    // Send clock
    _SCL1;
    _DELAY;
    _SCL0;
    _DELAY;
    // ACK end
    _SDA1;
}

#else

bool I2C::init() {

    i2c_config_t conf = {};

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda_pin_;
    conf.scl_io_num = scl_pin_;
    conf.sda_pullup_en = true;
    conf.scl_pullup_en = true;

    conf.master.clk_speed = i2c_frequency; // xx Hz
    conf.clk_flags = 0;

    ESP_LOGI(TAG, "i2c_param_config(speed=%dHz)", conf.master.clk_speed);
    if (0 != i2c_param_config(port_, &conf)) {
        ESP_LOGE(TAG, "i2c_param_config failed");
    }

    ESP_LOGI(TAG, "i2c_driver_install()");
    if (0 != i2c_driver_install(port_, conf.mode, 0, 0, 0)) {
        ESP_LOGE(TAG, "i2c_driver_install failed");
        return false;
    }

    return true;
}

bool I2C::begin() {

    //ESP_LOGI(TAG, "i2c_cmd_link_create()");
    handle_ = i2c_cmd_link_create();

    //ESP_LOGI(TAG, "handle=%p", handle_);

    if (nullptr == handle_) {
        ESP_LOGE(TAG, "i2c_cmd_link_create failed");
        return false;
    }

    return true;
}

bool I2C::end() {
    if (nullptr == handle_) {
        return false;
    }

    bool status = true;

    auto err = i2c_master_cmd_begin(port_, handle_, 1000 / portTICK_PERIOD_MS);

    if (0 != err) {
        ESP_LOGE(TAG, "i2c_master_cmd_begin failed: 0x%x", err);
        status = false;
    }

    i2c_cmd_link_delete(handle_);
    handle_ = nullptr;

    return status;
}

bool I2C::start() {
    if (0 != i2c_master_start(handle_)) {
        ESP_LOGE(TAG, "i2c_master_start failed");
        return false;
    }
    return true;
}

void I2C::stop() {
    if (0 != i2c_master_stop(handle_)) {
        ESP_LOGE(TAG, "i2c_master_stop failed");
    }
}

uint8_t I2C::read(void) {
    return 0x0;
}

void I2C::setAck(bool ack) {
}

bool I2C::write(uint8_t data) {

    //ESP_LOGI(TAG, "write 1 byte");

    if (0 != i2c_master_write_byte(handle_, data, true)) {
        ESP_LOGE(TAG, "i2c_master_write_byte failed");
        return false;
    }
    return true;
}

size_t I2C::write(const uint8_t* data, size_t sz) {

    //ESP_LOGI(TAG, "write %d bytes", (int) sz);

    if (0 != i2c_master_write(handle_, data, sz, true)) {
        ESP_LOGE(TAG, "i2c_master_write failed");
        return 0;
    }

    return sz;
}

#endif
