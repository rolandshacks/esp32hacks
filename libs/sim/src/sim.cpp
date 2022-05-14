//
// Sim
//

#include "sim/sim.h"

#include "application/application.h"
#include "sim/ssd1306.h"

#include <chrono>
#include <thread>
#include <algorithm>

#include "SDL.h"

/*
    All examples are compiled into one big binary to make sure changes never break any
    existing example and to have one additional quality gate.
    The "DECLARE_SIM_ENTRY()" macro defines the entry point into one example to be used
    for a specific simulation run.
    In CMake, ${SIMULATOR_APP} is defined to choose between examples. This is the
    counter part to the DECLARE_APP() macro each example uses to implement and export
    the main program entry and task user functions.
*/
DECLARE_SIM_ENTRY(SIMULATOR_APP);

using namespace simulator;

Sim* Sim::__instance = nullptr;

Sim::Sim() {

    if (nullptr == __instance) {
        __instance = this;
    }

    display_emu_ = new EmuSSD1306();
}

Sim::~Sim() {
    if (nullptr != display_emu_) {
        delete display_emu_;
        display_emu_ = nullptr;
    }
}

Sim* Sim::instance() {
    return __instance;
}

EmuSSD1306* Sim::getDisplayDevice() {
    return display_emu_;
}

int Sim::init() {

    int screen_width = 640;
    int screen_height = 480;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "could not init SDL: %s\n", SDL_GetError());
        return 1;
    }

    window_ = SDL_CreateWindow(
        "ESP32 Simulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screen_width, screen_height,
        SDL_WINDOW_SHOWN
    );

    if (window_ == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }

    screen_surface_ = SDL_GetWindowSurface(window_);

    buffer_surface_ = SDL_CreateRGBSurface(
        0, 128, 64, 32, 0x0, 0x0, 0x0, 0x0
    );

    if (nullptr == buffer_surface_) {
        fprintf(stderr, "could not create buffer surface: %s\n", SDL_GetError());
        return 1;
    }

    error_ = false;
    running_ = true;

    while (running_ && !error_) {
        clearDisplay();
        app_main();
    }

    return 0;
}

void Sim::restart() {
    printf("restart\n");
    error_ = false;
    running_ = true;
}

bool Sim::update() {
    SDL_Event e;

    while( SDL_PollEvent( &e ) != 0 ) {
        if(e.type == SDL_QUIT) {
            return false;
        }
    }

    if (!running_ || error_) {
        return false;
    }

    display_emu_->update();

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (0 == time_offset_) {
        time_offset_ = now;
        now = 0;
    } else {
        now -= time_offset_;
    }

    uint32_t cycle_time_ms = display_emu_->getCycleTimeMs();

    uint64_t elapsed = now - time_last_update_;
    if (elapsed >= cycle_time_ms) {
        time_last_update_ = now;
        updateDisplay();

        fps_counter_++;
        if (now - fps_time_ > 5000) {
            fps_ = fps_counter_ * 1000 / (now - fps_time_);
            fps_counter_ = 0;
            fps_time_ = now;

            printf("INFO sim: frames/sec: %d\n", fps_);
        }

    }

    return true;
}

void Sim::clearDisplay() {
    if (nullptr == window_ || nullptr == screen_surface_) {
        return;
    }

    SDL_FillRect(screen_surface_, NULL, SDL_MapRGB(screen_surface_->format, 0x08, 0x22, 0x28));
    SDL_UpdateWindowSurface(window_);
}

void Sim::copyDisplayToSurface(SDL_Surface* surface, bool zoom) {
    if (nullptr == surface) return;

    if (0 != SDL_LockSurface(surface)) {
        fprintf(stderr, "lock surface failed: %s\n", SDL_GetError());
        return;
    }

    int w = display_emu_->width();
    int h = display_emu_->height();

    int zoom_factor = zoom ? std::min(surface->w / w, surface->h / h) : 1;

    auto format = surface->format;
    auto pixels = surface->pixels;

    //uint32_t white_pixel = 0xffffffff;
    //uint32_t black_pixel = 0x000000ff;

    uint8_t* line = (uint8_t*) pixels;

    for (auto y=0; y<h; y++) {
        uint8_t* ptr = line;

        for (int j=0; j<zoom_factor; j++) {

            for (auto x=0; x<w; x++) {

                bool pixel = display_emu_->getPixel(x, y);
                uint8_t intensity = (pixel ? 0xff : 0x0);

                for (int i=0; i<zoom_factor; i++) {
                    *(ptr+0) = intensity;
                    *(ptr+1) = intensity;
                    *(ptr+2) = intensity;
                    *(ptr+3) = 0xff;
                    ptr += format->BytesPerPixel;
                }

            }

            line += surface->pitch;
        }

    }

    SDL_UnlockSurface(surface);
}

void Sim::updateDisplay() {
    if (nullptr == window_ || nullptr == screen_surface_ || nullptr == buffer_surface_) {
        return;
    }

    SDL_FillRect(screen_surface_, NULL, SDL_MapRGB(screen_surface_->format, 0x08, 0x22, 0x28));

    /*
    copyDisplayToSurface(screen_surface_, false);
    */

    copyDisplayToSurface(buffer_surface_, false);

    SDL_Rect src_rect { 0, 0, buffer_surface_->w, buffer_surface_->h };

    int w = buffer_surface_->w;
    while (w + buffer_surface_->w <= screen_surface_->w) {
        w += buffer_surface_->w;
    }
    int h = w * buffer_surface_->h / buffer_surface_->w;

    if (h >= screen_surface_->h) {
        h = screen_surface_->h;
        w = h * buffer_surface_->w / buffer_surface_->h;
    }

    SDL_Rect dest_rect { (screen_surface_->w - w) / 2, (screen_surface_->h -h) / 2, w, h };
    if (dest_rect.x < 0) dest_rect.x = 0;
    if (dest_rect.y < 0) dest_rect.y = 0;

    int res = SDL_SoftStretch(buffer_surface_, &src_rect, screen_surface_, &dest_rect);
    if (-1 == res) {
        fprintf(stderr, "blit failed: %s\n", SDL_GetError());
        return;
    }

    SDL_UpdateWindowSurface(window_);

}
