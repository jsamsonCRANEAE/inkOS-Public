#pragma once
#include "Screen.h"
#include "Settings.h"
#include "Clock.h"
#include "../hal/Buttons.h"
#include "../hal/Display.h"

// Owns the hardware, settings, clock and the screen stack. Single instance.
class App {
 public:
  Display display;
  Buttons buttons;
  Settings settings;
  Clock clock;

  void begin(ScreenPtr root);
  void loop();

  void push(ScreenPtr s);
  void pop();
  void requestRedraw() { dirty_ = true; }
  [[noreturn]] void sleepNow();

 private:
  void render();
  void drawStatusBar(Panel& g, const char* title);

  std::vector<ScreenPtr> stack_;
  bool dirty_ = true;
  uint32_t lastInput_ = 0;
  uint32_t lastTick_ = 0;
  int lastMinute_ = -1;
};

// Layout constants shared by all screens (portrait: 480 x 800).
namespace ui {
constexpr int STATUS_H = 44;   // top bar height
constexpr int MARGIN = 20;
}  // namespace ui
