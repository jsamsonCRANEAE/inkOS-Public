#pragma once
// Xteink X4 hardware map.
// Cross-checked against: Adafruit CircuitPython X4 guide, Idered/xteink-x4 sample,
// quireink_x4 Rust crate, and the X3 firmware-analysis gist (X4 column).
#include <stdint.h>

namespace board {

// ---- E-paper (SSD1677, GDEQ0426T82, 800x480 native landscape) ----
constexpr int EPD_SCLK = 8;
constexpr int EPD_MOSI = 10;
constexpr int EPD_CS   = 21;
constexpr int EPD_DC   = 4;
constexpr int EPD_RST  = 5;
constexpr int EPD_BUSY = 6;

// ---- microSD (shares the SPI bus) ----
constexpr int SD_MISO = 7;
constexpr int SD_CS   = 12;   // must be held HIGH while talking to the display

// ---- Inputs ----
constexpr int BTN_ADC_FRONT = 1;  // resistor ladder: Back / Confirm / Left / Right
constexpr int BTN_ADC_SIDE  = 2;  // resistor ladder: Up / Down
constexpr int BTN_POWER     = 3;  // active LOW, needs pull-up

// ---- Power / battery ----
constexpr int BATTERY_ADC = 0;    // battery voltage / 2 (2x10k divider)
constexpr int USB_DETECT  = 20;   // HIGH when USB power present

// ---- Measured 12-bit ADC centres (community measurements, 11 dB attenuation) ----
// Front ladder: Back ~3470, Confirm ~2655, Left ~1470, Right ~3, idle ~4095
// Side ladder:  Up ~2205, Down ~3, idle ~4095
// Decision thresholds sit at the midpoints. If a button misreads on YOUR unit,
// open Settings > Diagnostics, read the raw values, and adjust these.
constexpr uint16_t FRONT_IDLE_MIN    = 3800;  // above this: nothing pressed
constexpr uint16_t FRONT_BACK_MIN    = 3060;  // 3060..3800  -> Back
constexpr uint16_t FRONT_CONFIRM_MIN = 2060;  // 2060..3060  -> Confirm
constexpr uint16_t FRONT_LEFT_MIN    = 740;   //  740..2060  -> Left, below -> Right

constexpr uint16_t SIDE_IDLE_MIN = 3150;      // above this: nothing pressed
constexpr uint16_t SIDE_UP_MIN   = 1100;      // 1100..3150 -> Up, below -> Down

}  // namespace board
