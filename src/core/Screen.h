#pragma once
#include <memory>
#include <vector>
#include "../hal/ButtonDecode.h"
#include "../hal/Display.h"

class App;

// A full-screen view. The App owns a stack of these; Back pops by default.
class Screen {
 public:
  virtual ~Screen() = default;
  virtual const char* title() const = 0;
  // Return true if the screen needs redrawing.
  virtual bool handle(const Event& ev, App& app) = 0;
  virtual void draw(Panel& g, App& app) = 0;
  virtual void onEnter(App&) {}
  // Called about once a second; return true to redraw (e.g. a clock ticked over).
  virtual bool tick(App&) { return false; }
  // Live views redraw often; skip the periodic full (flashing) refresh while they're open.
  virtual bool liveView() const { return false; }
};

using ScreenPtr = std::unique_ptr<Screen>;
