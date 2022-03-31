#pragma once

typedef int esp_err_t;

typedef enum {
    ADC_CHANNEL_0 = 0, /*!< ADC channel */
    ADC_CHANNEL_1,     /*!< ADC channel */
    ADC_CHANNEL_2,     /*!< ADC channel */
    ADC_CHANNEL_3,     /*!< ADC channel */
    ADC_CHANNEL_4,     /*!< ADC channel */
    ADC_CHANNEL_5,     /*!< ADC channel */
    ADC_CHANNEL_6,     /*!< ADC channel */
    ADC_CHANNEL_7,     /*!< ADC channel */
    ADC_CHANNEL_8,     /*!< ADC channel */
    ADC_CHANNEL_9,     /*!< ADC channel */
    ADC_CHANNEL_MAX,
} adc_channel_t;

typedef enum {
    ADC1_CHANNEL_0 = 0, /*!< ADC1 channel 0 is GPIO36 */
    ADC1_CHANNEL_1,     /*!< ADC1 channel 1 is GPIO37 */
    ADC1_CHANNEL_2,     /*!< ADC1 channel 2 is GPIO38 */
    ADC1_CHANNEL_3,     /*!< ADC1 channel 3 is GPIO39 */
    ADC1_CHANNEL_4,     /*!< ADC1 channel 4 is GPIO32 */
    ADC1_CHANNEL_5,     /*!< ADC1 channel 5 is GPIO33 */
    ADC1_CHANNEL_6,     /*!< ADC1 channel 6 is GPIO34 */
    ADC1_CHANNEL_7,     /*!< ADC1 channel 7 is GPIO35 */
    ADC1_CHANNEL_MAX,
} adc1_channel_t;

typedef enum {
    ADC_WIDTH_BIT_9 = 0,  /*!< ADC capture width is 9Bit. */
    ADC_WIDTH_BIT_10 = 1, /*!< ADC capture width is 10Bit. */
    ADC_WIDTH_BIT_11 = 2, /*!< ADC capture width is 11Bit. */
    ADC_WIDTH_BIT_12 = 3, /*!< ADC capture width is 12Bit. */
    ADC_WIDTH_MAX,
} adc_bits_width_t;

typedef enum {
    ADC_ATTEN_DB_0 =
        0, /*!<No input attenumation, ADC can measure up to approx. 800 mV. */
    ADC_ATTEN_DB_2_5 =
        1, /*!<The input voltage of ADC will be attenuated extending the range
              of measurement by about 2.5 dB (1.33 x) */
    ADC_ATTEN_DB_6 =
        2, /*!<The input voltage of ADC will be attenuated extending the range
              of measurement by about 6 dB (2 x) */
    ADC_ATTEN_DB_11 =
        3, /*!<The input voltage of ADC will be attenuated extending the range
              of measurement by about 11 dB (3.55 x) */
    ADC_ATTEN_MAX,
} adc_atten_t;

#define ADC_WIDTH_12Bit ADC_WIDTH_BIT_12
#define ADC_ATTEN_0db ADC_ATTEN_DB_0

int adc1_get_raw(adc1_channel_t channel);
esp_err_t adc1_config_width(adc_bits_width_t width_bit);
esp_err_t adc1_config_channel_atten(adc1_channel_t channel, adc_atten_t atten);
