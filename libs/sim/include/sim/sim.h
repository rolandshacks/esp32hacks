//
// Simulator Application
//
#pragma once

#ifndef SIMULATOR
#define SIMULATOR
#endif

#include "SDL.h"
#include "graphics/graphics.h"
#include "sim/ssd1306.h"

namespace simulator {

class Sim {
   public:
    Sim();
    ~Sim();

   public:
    static Sim* instance();

   public:
    int init();
    void restart();
    bool update();
    EmuSSD1306* getDisplayDevice();

   private:
    void clearDisplay();
    void updateDisplay();
    void copyDisplayToSurface(SDL_Surface* surface, bool zoom);

   private:
    static Sim* __instance;

    bool running_{false};
    bool error_{false};

    EmuSSD1306* display_emu_{nullptr};
    uint64_t time_offset_{0};
    uint64_t time_last_update_{0};

    SDL_Window* window_{nullptr};
    SDL_Surface* screen_surface_{nullptr};
    SDL_Surface* buffer_surface_{nullptr};

    int fps_{0};
    uint64_t fps_counter_{0};
    uint64_t fps_time_{0};

   private:

   public:
    Sim(const Sim&) = delete;
    Sim(const Sim&&) = delete;
    Sim& operator=(const Sim&) = delete;
    Sim& operator=(const Sim&&) = delete;
};

} // namespace
