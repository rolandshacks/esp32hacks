//
// Oscilloscope
//
#pragma once

class Oscilloscope {
    public:
        Oscilloscope();
        virtual ~Oscilloscope();

    public:
        void init();
        void update();

    private:
        int fetch_data();

    private:
        adc1_channel_t channel_;
        uint16_t graph_[128];
        char textbuffer_[128];
};
