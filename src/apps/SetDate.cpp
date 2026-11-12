#include "Screens.h"
#include "../core/App.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

// Set date & time.  Left/Right = choose field, Up/Down = change (hold to repeat),
// Confirm = save, Back = cancel.
class SetDateScreen : public Screen {
 public:
  const char* title() const override { return "Set date & time"; }

  void onEnter(App& app) override {
    if (loaded_) return;
    loaded_ = true;
    d_ = app.clock.today();
    hour_ = app.clock.hour();
    minute_ = app.clock.minute();
  }

  bool handle(const Event& ev, App& app) override {
    if (ev.btn == Btn::Left && ev.type == EvType::Press) { field_ = (field_ + 4) % 5; return true; }
    if (ev.btn == Btn::Right && ev.type == EvType::Press) { field_ = (field_ + 1) % 5; return true; }
    if (ev.btn == Btn::Up && ev.type != EvType::None) { adjust(+1); return true; }
    if (ev.btn == Btn::Down && ev.type != EvType::None) { adjust(-1); return true; }
    if (ev.btn == Btn::Confirm && ev.type == EvType::Press) {
      app.clock.set(d_, hour_, minute_);
      app.pop();
      return false;  // screen is gone; App already marked dirty
    }
    return false;
  }

  void draw(Panel& g, App&) override {
    char buf[8];
    struct F { const char* label; int value; int digits; };
    F f[5] = {{"Year", d_.y, 4}, {"Month", d_.m, 2}, {"Day", d_.d, 2}, {"Hour", hour_, 2}, {"Min", minute_, 2}};

    // Row 1: Year / Month / Day   Row 2: Hour / Min
    const int boxH = 110, gap = 16;
    int y1 = ui::STATUS_H + 70, y2 = y1 + boxH + 70;
    int x = ui::MARGIN;
    for (int i = 0; i < 5; i++) {
      if (i == 3) x = ui::MARGIN;
      int w = (i == 0) ? 170 : 125;
      int y = i < 3 ? y1 : y2;
      g.setFont(&FreeSans9pt7b);
      g.setTextColor(GxEPD_BLACK);
      g.setCursor(x + 4, y - 10);
      g.print(f[i].label);
      bool on = i == field_;
      if (on) g.fillRoundRect(x, y, w, boxH, 10, GxEPD_BLACK);
      else { g.drawRoundRect(x, y, w, boxH, 10, GxEPD_BLACK); g.drawRoundRect(x + 1, y + 1, w - 2, boxH - 2, 9, GxEPD_BLACK); }
      snprintf(buf, sizeof buf, f[i].digits == 4 ? "%04d" : "%02d", f[i].value);
      g.setFont(&FreeSansBold24pt7b);
      g.setTextColor(on ? GxEPD_WHITE : GxEPD_BLACK);
      int16_t bx, by; uint16_t bw, bh;
      g.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
      g.setCursor(x + (w - bw) / 2 - bx, y + (boxH + bh) / 2);
      g.print(buf);
      x += w + gap;
    }

    g.setTextColor(GxEPD_BLACK);
    g.setFont(&FreeSans12pt7b);
    g.setCursor(ui::MARGIN, y2 + boxH + 60);
    g.print(cal::weekdayLong(cal::dayOfWeek(d_.y, d_.m, d_.d)));

    g.setFont(&FreeSans9pt7b);
    g.setCursor(ui::MARGIN, g.height() - 40);
    g.print("Left/Right field   Up/Down change");
    g.setCursor(ui::MARGIN, g.height() - 16);
    g.print("Confirm save   Back cancel");
  }

 private:
  void adjust(int s) {
    switch (field_) {
      case 0: { int y = d_.y + s; d_.y = y < 2024 ? 2099 : (y > 2099 ? 2024 : y); break; }
      case 1: d_.m = (d_.m - 1 + s + 12) % 12 + 1; break;
      case 2: { int dim = cal::daysInMonth(d_.y, d_.m); d_.d = (d_.d - 1 + s + dim) % dim + 1; break; }
      case 3: hour_ = (hour_ + s + 24) % 24; break;
      case 4: minute_ = (minute_ + s + 60) % 60; break;
    }
    int dim = cal::daysInMonth(d_.y, d_.m);  // clamp after year/month change (Feb 29 etc.)
    if (d_.d > dim) d_.d = dim;
  }
  Date d_{2026, 1, 1};
  int hour_ = 0, minute_ = 0, field_ = 0;
  bool loaded_ = false;
};

ScreenPtr makeSetDateScreen() { return ScreenPtr(new SetDateScreen()); }
