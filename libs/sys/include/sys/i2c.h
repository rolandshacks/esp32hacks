//
// I2C
//
#pragma once

#include <cstddef>
#include <cstdint>

namespace sys {

class I2C {
    public:
        I2C(int scl, int sda, uint8_t address);
        ~I2C();

    public:
        bool write(uint8_t data);
        size_t write(const uint8_t* data, size_t sz);
        uint8_t read(void);

    public:
        bool sendControl(uint8_t control);
        bool sendControlBuffer(const uint8_t* buffer, size_t buffer_size);
        bool sendData(uint8_t data);
        bool sendDataBuffer(const uint8_t* buffer, size_t buffer_size);
        bool sendEmpty();

    private:
        bool init();
        bool begin();
        bool end();
        bool start();
        void stop();
        void setAck(bool ack);
        bool sendRaw(uint8_t value, int flags);
        bool sendBufferRaw(const uint8_t* buffer, size_t buffer_size, int flags);

    private:
        int scl_pin_;                 // I2C SCL pin number
        int sda_pin_;                 // I2C SDA pin number
        uint8_t address_;             // I2C address
        uint8_t port_;                // I2C controller port
        void* handle_{0};             // I2C link handle

    public:
        static const size_t IC2_MAX_LIST_LEN = 32;
};

}  // namespace sys
