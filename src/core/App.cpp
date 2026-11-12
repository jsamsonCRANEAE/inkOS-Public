#include "App.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "../hal/Power.h"
#include <esp_sleep.h>

void App::begin(ScreenPtr root) {
  // 80 MHz is plenty for an e-ink UI and roughly halves active current vs. 160 MHz.
  setCpuFrequencyMhz(80);
  settings.load();
  clock.begin();
  buttons.begin();
  display.begin(settings.rotation);
  refreshPower();
  lastInput_ = millis();
  push(std::move(root));
}

void App::push(ScreenPtr s) {
  stack_.push_back(std::move(s));
  stack_.back()->onEnter(*this);
  dirty_ = true;
}

void App::pop() {
  if (stack_.size() > 1) {
    stack_.pop_back();
    stack_.back()->onEnter(*this);
    dirty_ = true;
  }
}

void App::sleepNow() {
  clock.persist();
  Panel& g = display.frame();
  g.setFont(&FreeSansBold9pt7b);
  int16_t x, y; uint16_t w, h;
  const char* msg = "Sleeping - press Power to wake";
  g.getTextBounds(msg, 0, 0, &x, &y, &w, &h);
  g.setCursor((g.width() - w) / 2, g.height() / 2);
  g.print(msg);
  display.requestFull();
  display.show(settings.fullEvery);
  display.sleep();
  power::deepSleep();
}

void App::loop() {
  Event ev = buttons.poll();
  uint32_t now = millis();

  if (ev) {
    lastInput_ = now;
    lastActivity_ = now;
    if (ev.btn == Btn::Power && ev.type == EvType::Long) sleepNow();
    if (ev.btn == Btn::Power && ev.type == EvType::Press) {
      // Short power press: force a clean full refresh (handy for ghosting).
      display.requestFull();
      dirty_ = true;
    } else {
      bool handled = stack_.back()->handle(ev, *this);
      if (!handled && ev.btn == Btn::Back && ev.type == EvType::Press) pop();
      else if (handled) dirty_ = true;
    }
  }

  if (now - lastTick_ >= 1000) {
    lastTick_ = now;
    if (stack_.back()->tick(*this)) dirty_ = true;
    int m = clock.minute();
    if (m != lastMinute_) {  // once a minute: status-bar clock + battery
      lastMinute_ = m;
      refreshPower();
      dirty_ = true;
    }
  }

  if (settings.sleepMinutes && now - lastInput_ > settings.sleepMinutes * 60000UL) sleepNow();

  if (dirty_) { dirty_ = false; render(); }
  idle();
}

// Adaptive polling.
//  FAST: after any button activity, poll every 5 ms (CPU awake) for FAST_HOLD_MS.
//  IDLE: once things go quiet, light-sleep 20 ms between polls to save battery.
// A press from idle is caught within 20 ms, then the next FAST_HOLD_MS run at full speed.
// On USB power we never light-sleep, so the serial console stays connected.
void App::idle() {
  uint32_t now = millis();
  if (buttons.held() != Btn::None) lastActivity_ = now;  // holding counts as activity
  bool fast = now - lastActivity_ < FAST_HOLD_MS;
  if (fast) { delay(5); return; }
  if (usb_) { delay(20); return; }
  esp_sleep_enable_timer_wakeup(20 * 1000);
  esp_light_sleep_start();
}

void App::refreshPower() {
  battery_ = power::batteryPercent();
  usb_ = power::usbPowered();
}

void App::render() {
  Panel& g = display.frame();
  drawStatusBar(g, stack_.back()->title());
  stack_.back()->draw(g, *this);
  display.show(stack_.back()->liveView() ? 0 : settings.fullEvery);
}

void App::drawStatusBar(Panel& g, const char* title) {
  g.setFont(&FreeSansBold9pt7b);
  g.setCursor(ui::MARGIN, 29);
  g.print(title);

  char right[32], t[16];
  clock.format(t, sizeof t, settings.clock24h);
  snprintf(right, sizeof right, "%s%s  %d%%%s", clock.approximate() ? "~" : "", t,
           battery_, usb_ ? "+" : "");
  g.setFont(&FreeSans9pt7b);
  int16_t x, y; uint16_t w, h;
  g.getTextBounds(right, 0, 0, &x, &y, &w, &h);
  g.setCursor(g.width() - ui::MARGIN - w, 29);
  g.print(right);

  g.drawFastHLine(0, ui::STATUS_H - 1, g.width(), GxEPD_BLACK);
  g.drawFastHLine(0, ui::STATUS_H, g.width(), GxEPD_BLACK);
}
