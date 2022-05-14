//
// SSD1306 Commands
//
#pragma once

#include <cstdint>

namespace graphics {

enum class Command : uint8_t {
    None = 0x0,
    SetMemoryAddressingMode = 0x20,
    SetColumnAddress = 0x21,
    SetPageAddress = 0x22,
    SetStartLine = 0x40, // 0x40..7F
    SetContrastControl = 0x81,
    SetSegmentRemapNormal = 0xA0,
    SetSegmentRemapInverse = 0xA1,
    EntireDisplayOnResume = 0xA4,
    EntireDisplayOn = 0xA5, // unused
    SetNormalDisplay = 0xA6,
    SetInverseDisplay = 0xA7,
    SetMultiplexRatio = 0xA8,
    SetDisplayOff = 0xAE,
    SetDisplayOn = 0xAF,
    SetComOutputScanInc = 0xC0,
    SetComOutputScanDec = 0xC8,
    SetDisplayOffset = 0xD3,
    SetDisplayClockDivider = 0xD5,
    SetPreChargePeriod = 0xD9,
    SetComPinsHardwareConfiguration = 0xDA,
    SetVComHDeselectLevel = 0xDB,
    NOP = 0xE3,
    RightScroll = 0x26,
    LeftScroll = 0x27,
    VerticalRightScroll = 0x29,
    VerticalLeftScroll = 0x2A,
    DeactivateScroll = 0x2E,
    ActivateScroll = 0x2F,
    SetVerticalScrollArea = 0xA3,
    ChargePumpSetting = 0x8D
};

enum class MemoryMode : uint8_t {
    HorizontalMode = 0x0,
    VerticalMode = 0x1,
    PageMode = 0x2
};

enum class PanelType : uint8_t {
    SSD1306_128x64 = 1,  //!< 128x32 panel
    SSD1306_128x32 = 2   //!< 128x64 panel
};

} // namespace graphics
