//
// Oscilloscope
//
#include <string>
#include <memory.h>

#include "app/bits.h"

#include "app/oscilloscope.h"

extern oled::OLED display;

Oscilloscope::Oscilloscope() :
    channel_(ADC1_CHANNEL_4) {
    ;
}

Oscilloscope::~Oscilloscope() {
    ;
}

void Oscilloscope::init() {
    ESP_LOGI("oscilloscope", "Oscilloscope initialized");

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(channel_, ADC_ATTEN_0db);

    memset(graph_, 0, sizeof(graph_));
}

int Oscilloscope::fetch_data() {
    return adc1_get_raw(channel_);
}

void Oscilloscope::update() {

    display.clear();
    display.select_font(1);

    auto data = fetch_data();

    sprintf(textbuffer_, "DATA: %d", data);

    for (int i = 0; i < 127; i++) {
        graph_[i] = graph_[i + 1];
    }
    graph_[127] = data;

    display.draw_string(0, 0, textbuffer_, oled::WHITE, oled::BLACK);

    graph_[127] = graph_[127] * 48 / 4096;

    for (uint8_t i = 0; i < 128; i++) {
        display.draw_pixel(i, 63 - graph_[i], oled::WHITE);
    }

    display.draw_pixel(127, 63 - graph_[127], oled::WHITE);

    display.refresh(true);

}
