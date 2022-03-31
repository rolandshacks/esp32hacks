//
// Sim
//
#include "driver/adc.h"

#include <cmath>

static int adc1_bits_width = 12;
static const double PI2 = 3.14159265358979323846 * 2.0;
static double adc1_counter = 0.0;

int adc1_get_raw(adc1_channel_t channel) {
    double range = (double)(1 << adc1_bits_width);
    int value = range * cos(adc1_counter);
    adc1_counter += 0.1;
    if (adc1_counter >= PI2) adc1_counter -= PI2;

    return value;
}

esp_err_t adc1_config_width(adc_bits_width_t width_bit) {
    if (ADC_WIDTH_BIT_9 == width_bit) {
        adc1_bits_width = 9;
    } else if (ADC_WIDTH_BIT_10 == width_bit) {
        adc1_bits_width = 10;
    } else if (ADC_WIDTH_BIT_11 == width_bit) {
        adc1_bits_width = 11;
    } else if (ADC_WIDTH_BIT_12 == width_bit) {
        adc1_bits_width = 12;
    } else {
        return -1;
    }

    return 0;
}

esp_err_t adc1_config_channel_atten(adc1_channel_t channel, adc_atten_t atten) { return 0; }
