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

        setPeriod(40);

        channel_ = ADC1_CHANNEL_4;
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(channel_, ADC_ATTEN_0db);

        auto display = getDisplay();  // get display reference
        display->clear();              // clear display
        display->setBuiltinFont(1);
        display->update(true);  // update display

        oscilloscope.init();
    }

    void update() override  {

        auto display = getDisplay();  // get display reference

        display->clear();  // clear display (ignore locked areas)

        int top = display->font()->height + 2;

        display->drawHorizontalLine(0, top-1, display->width()-1);

        int value = adc1_get_raw(channel_);
        oscilloscope.add(value);
        oscilloscope.draw(display, 0, top, display->width()-1, display->height()-1, true, 2, 1);

        display->update(true);  // refresh display
    }

   private:
    adc1_channel_t channel_;
    graphics::Oscilloscope oscilloscope;

    _NODEFAULTS(OscilloscopeDemo)
};

DECLARE_APP(OscilloscopeDemo);
