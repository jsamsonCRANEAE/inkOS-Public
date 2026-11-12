#pragma once
// The ONLY file that knows which e-paper controller is fitted.
// If your unit carries a UC8179/UC8279 panel instead of SSD1677, swap the driver class here.
#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include "board.h"

using Panel = GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT>;

class Display {
 public:
  Display() : epd_(GxEPD2_426_GDEQ0426T82(board::EPD_CS, board::EPD_DC, board::EPD_RST, board::EPD_BUSY)) {}

  void begin(uint8_t rotation) {
    pinMode(board::SD_CS, OUTPUT);
    digitalWrite(board::SD_CS, HIGH);  // keep the SD card off the shared bus
    SPI.begin(board::EPD_SCLK, board::SD_MISO, board::EPD_MOSI, board::EPD_CS);
    epd_.init(0, true, 2, false);
    epd_.setRotation(rotation);
    epd_.setTextWrap(false);
    forceFull_ = true;
  }

  void setRotation(uint8_t r) { epd_.setRotation(r); forceFull_ = true; }

  // Start a frame: clear the RAM buffer to white.
  Panel& frame() {
    epd_.setFullWindow();
    epd_.fillScreen(GxEPD_WHITE);
    epd_.setTextColor(GxEPD_BLACK);
    return epd_;
  }

  // Push the frame. Partial refresh is fast but accumulates ghosting;
  // every `fullEvery` partials we do one full (flashing) refresh to clean the panel.
  void show(uint8_t fullEvery) {
    bool full = forceFull_ || (fullEvery > 0 && partials_ >= fullEvery);
    epd_.display(!full);
    epd_.powerOff();  // panel holds the image with zero power; cut its charge pumps
    partials_ = full ? 0 : partials_ + 1;
    forceFull_ = false;
  }

  void requestFull() { forceFull_ = true; }
  void sleep() { epd_.hibernate(); }
  int16_t width() { return epd_.width(); }
  int16_t height() { return epd_.height(); }

 private:
  Panel epd_;
  uint8_t partials_ = 0;
  bool forceFull_ = true;
};
