#pragma once
#include <functional>
#include <string>
#include <vector>
#include "App.h"
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Generic vertical list menu.
//  Up/Left = previous, Down/Right = next (held = repeat), Confirm = activate, Back = exit.
// Items may carry a value string (e.g. "Monday") and an "adjust" callback so a setting can
// be cycled in place with Confirm.
struct MenuItem {
  std::string label;
  std::function<std::string(App&)> value; // optional: right-aligned value text
  std::function<void(App&)> activate;     // Confirm
};

class MenuScreen : public Screen {
 public:
  MenuScreen(const char* title, std::vector<MenuItem> items) : title_(title), items_(std::move(items)) {}
  const char* title() const override { return title_; }

  bool handle(const Event& ev, App& app) override {
    if (items_.empty()) return false;
    int n = (int)items_.size();
    bool prev = ev.btn == Btn::Up || ev.btn == Btn::Left;
    bool next = ev.btn == Btn::Down || ev.btn == Btn::Right;
    if (prev && ev.type != EvType::None) { sel_ = (sel_ - 1 + n) % n; return true; }
    if (next && ev.type != EvType::None) { sel_ = (sel_ + 1) % n; return true; }
    if (ev.btn == Btn::Confirm && ev.type == EvType::Press && items_[sel_].activate) {
      items_[sel_].activate(app);
      return true;
    }
    return false;
  }

  void draw(Panel& g, App& app) override {
    const int rowH = 64;
    int top = ui::STATUS_H + 16;
    int visible = (g.height() - top - 40) / rowH;
    int first = sel_ >= visible ? sel_ - visible + 1 : 0;

    for (int i = first; i < (int)items_.size() && i < first + visible; i++) {
      int y = top + (i - first) * rowH;
      bool on = i == sel_;
      if (on) g.fillRoundRect(ui::MARGIN - 8, y, g.width() - 2 * (ui::MARGIN - 8), rowH - 8, 8, GxEPD_BLACK);
      uint16_t fg = on ? GxEPD_WHITE : GxEPD_BLACK;
      g.setTextColor(fg);
      g.setFont(&FreeSans12pt7b);
      g.setCursor(ui::MARGIN + 8, y + 37);
      g.print(items_[i].label.c_str());
      if (items_[i].value) {
        std::string v = items_[i].value(app);
        g.setFont(&FreeSans9pt7b);
        int16_t bx, by; uint16_t bw, bh;
        g.getTextBounds(v.c_str(), 0, 0, &bx, &by, &bw, &bh);
        g.setCursor(g.width() - ui::MARGIN - 8 - bw, y + 36);
        g.print(v.c_str());
      }
    }
    g.setTextColor(GxEPD_BLACK);
    g.setFont(&FreeSans9pt7b);
    g.setCursor(ui::MARGIN, g.height() - 16);
    g.print("Up/Down move   Confirm select   Back exit");
  }

 private:
  const char* title_;
  std::vector<MenuItem> items_;
  int sel_ = 0;
};
