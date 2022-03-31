//
// Graphics Demo
//

#include "application/application.h"
#include "driver/adc.h"
#include "graphics/graphics.h"
#include "graphics/oscilloscope.h"

class OscilloscopeDemo : public application::Application {
   public:
    explicit OscilloscopeDemo() : Application() {}

    void init() override {

        set_period_ms(100);

        channel_ = ADC1_CHANNEL_4;
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(channel_, ADC_ATTEN_0db);

        auto display = get_display();  // get display reference
        display->clear();              // clear display
        display->select_font(1);
        display->refresh(true);  // update display

        oscilloscope.init();
    }

    void update() override  {

        auto display = get_display();  // get display reference

        display->clear();  // clear display (ignore locked areas)

        display->draw_hline(0, 12, display->get_width(), graphics::WHITE);

        int value = adc1_get_raw(channel_);
        oscilloscope.add(value);
        oscilloscope.draw(display, 0, 13, 128, 51, true, 2, 1);

        display->refresh(true);  // refresh display
    }

   private:
    adc1_channel_t channel_;
    graphics::Oscilloscope oscilloscope;

    _NODEFAULTS(OscilloscopeDemo)
};

DECLARE_APP(OscilloscopeDemo);
