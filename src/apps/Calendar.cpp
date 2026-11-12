#include "Screens.h"
#include "../core/App.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>

// Month calendar.
//   Left / Right        : previous / next day   (hold = previous / next month, repeating)
//   Up / Down (side)    : previous / next week  (hold = repeat)
//   Confirm             : jump back to today
//   Back                : exit
class CalendarScreen : public Screen {
 public:
  const char* title() const override { return "Calendar"; }

  void onEnter(App& app) override {
    if (!init_) { sel_ = app.clock.today(); init_ = true; }
  }

  bool handle(const Event& ev, App& app) override {
    if (ev.type == EvType::None) return false;
    bool tap = ev.type == EvType::Press;
    switch (ev.btn) {
      case Btn::Left:  sel_ = tap ? cal::addDays(sel_, -1) : cal::addMonths(sel_, -1); return true;
      case Btn::Right: sel_ = tap ? cal::addDays(sel_, +1) : cal::addMonths(sel_, +1); return true;
      case Btn::Up:    sel_ = cal::addDays(sel_, -7); return true;
      case Btn::Down:  sel_ = cal::addDays(sel_, +7); return true;
      case Btn::Confirm:
        if (!tap) return false;
        sel_ = app.clock.today();
        return true;
      default: return false;
    }
  }

  // Redraw at midnight so "today" moves.
  bool tick(App& app) override {
    Date t = app.clock.today();
    if (t != lastToday_) { lastToday_ = t; return true; }
    return false;
  }

  void draw(Panel& g, App& app) override {
    const Settings& s = app.settings;
    const Date today = app.clock.today();
    lastToday_ = today;
    const int W = g.width();

    // ---- Month header ----
    char buf[48];
    snprintf(buf, sizeof buf, "%s %d", cal::monthName(sel_.m), sel_.y);
    g.setFont(&FreeSansBold18pt7b);
    centerText(g, buf, W / 2, ui::STATUS_H + 52);
    g.setFont(&FreeSans12pt7b);
    g.setCursor(ui::MARGIN, ui::STATUS_H + 50);       g.print("<");
    g.setCursor(W - ui::MARGIN - 12, ui::STATUS_H + 50); g.print(">");

    // ---- Grid geometry ----
    const int wkCol = s.showWeekNumbers ? 40 : 0;
    const int cellW = (W - 2 * 12 - wkCol) / 7;
    const int x0 = (W - wkCol - 7 * cellW) / 2 + wkCol;
    const int headY = ui::STATUS_H + 92;
    const int gridY = headY + 14;
    const int cellH = 70;
    const int first = cal::firstColumn(sel_.y, sel_.m, s.weekStart);
    const int dim = cal::daysInMonth(sel_.y, sel_.m);
    const int rows = cal::rowsNeeded(sel_.y, sel_.m, s.weekStart);

    // Weekday labels
    g.setFont(&FreeSansBold12pt7b);
    for (int c = 0; c < 7; c++) {
      const char* wd = cal::weekdayShort((c + s.weekStart) % 7);
      char two[3] = {wd[0], wd[1], 0};
      centerText(g, two, x0 + c * cellW + cellW / 2, headY);
    }
    g.drawFastHLine(x0, gridY - 4, 7 * cellW, GxEPD_BLACK);

    // Days
    for (int day = 1; day <= dim; day++) {
      int idx = first + day - 1;
      int r = idx / 7, c = idx % 7;
      int cx = x0 + c * cellW, cy = gridY + r * cellH;
      Date d{sel_.y, sel_.m, (int8_t)day};
      bool isSel = d == sel_;
      bool isToday = d == today;

      if (isSel) g.fillRoundRect(cx + 3, cy + 3, cellW - 6, cellH - 6, 10, GxEPD_BLACK);
      if (isToday && !isSel) {
        g.drawRoundRect(cx + 3, cy + 3, cellW - 6, cellH - 6, 10, GxEPD_BLACK);
        g.drawRoundRect(cx + 4, cy + 4, cellW - 8, cellH - 8, 9, GxEPD_BLACK);
        g.drawRoundRect(cx + 5, cy + 5, cellW - 10, cellH - 10, 8, GxEPD_BLACK);
      }
      if (isToday && isSel) {  // inner white ring marks "today" on the inverted cell
        g.drawRoundRect(cx + 8, cy + 8, cellW - 16, cellH - 16, 7, GxEPD_WHITE);
      }

      snprintf(buf, sizeof buf, "%d", day);
      g.setFont(isToday ? &FreeSansBold18pt7b : &FreeSans12pt7b);
      g.setTextColor(isSel ? GxEPD_WHITE : GxEPD_BLACK);
      centerText(g, buf, cx + cellW / 2, cy + cellH / 2 + (isToday ? 12 : 9));
      g.setTextColor(GxEPD_BLACK);
    }

    // ISO week numbers in the left column
    if (s.showWeekNumbers) {
      g.setFont(&FreeSans9pt7b);
      for (int r = 0; r < rows; r++) {
        int dayInRow = r * 7 - first + 1;
        if (dayInRow < 1) dayInRow = 1;
        Date d{sel_.y, sel_.m, (int8_t)dayInRow};
        snprintf(buf, sizeof buf, "%d", cal::isoWeek(d));
        centerText(g, buf, 12 + wkCol / 2, gridY + r * cellH + cellH / 2 + 6);
      }
    }

    // ---- Selected-day detail panel ----
    int py = gridY + 6 * cellH + 16;  // fixed position so the panel doesn't jump between months
    g.drawFastHLine(ui::MARGIN, py, W - 2 * ui::MARGIN, GxEPD_BLACK);
    int wd = cal::dayOfWeek(sel_.y, sel_.m, sel_.d);
    g.setFont(&FreeSansBold18pt7b);
    g.setCursor(ui::MARGIN, py + 44);
    g.print(cal::weekdayLong(wd));
    g.setFont(&FreeSans12pt7b);
    snprintf(buf, sizeof buf, "%s %d, %d", cal::monthName(sel_.m), sel_.d, sel_.y);
    g.setCursor(ui::MARGIN, py + 78);
    g.print(buf);

    int delta = cal::toDays(sel_) - cal::toDays(today);
    if (delta == 0) snprintf(buf, sizeof buf, "Today");
    else if (delta == 1) snprintf(buf, sizeof buf, "Tomorrow");
    else if (delta == -1) snprintf(buf, sizeof buf, "Yesterday");
    else if (delta > 0) snprintf(buf, sizeof buf, "In %d days", delta);
    else snprintf(buf, sizeof buf, "%d days ago", -delta);
    g.setCursor(ui::MARGIN, py + 110);
    g.print(buf);

    int doy = cal::toDays(sel_) - cal::toDays(Date{sel_.y, 1, 1}) + 1;
    int diy = cal::isLeap(sel_.y) ? 366 : 365;
    snprintf(buf, sizeof buf, "Week %d  -  Day %d of %d", cal::isoWeek(sel_), doy, diy);
    g.setFont(&FreeSans9pt7b);
    g.setCursor(ui::MARGIN, py + 140);
    g.print(buf);

    g.setCursor(ui::MARGIN, g.height() - 16);
    g.print("L/R day  Up/Dn week  Hold month  OK today");
  }

 private:
  static void centerText(Panel& g, const char* s, int cx, int baseline) {
    int16_t bx, by; uint16_t bw, bh;
    g.getTextBounds(s, 0, 0, &bx, &by, &bw, &bh);
    g.setCursor(cx - (int)bw / 2 - bx, baseline);
    g.print(s);
  }

  Date sel_{2026, 1, 1};
  Date lastToday_{0, 1, 1};
  bool init_ = false;
};

ScreenPtr makeCalendarScreen() { return ScreenPtr(new CalendarScreen()); }
