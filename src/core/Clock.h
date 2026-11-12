#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <sys/time.h>
#include <time.h>
#include "CalendarMath.h"

// Timekeeping. The X4 has no battery-backed RTC chip, so:
//  - while awake or in deep sleep, the ESP32-C3's internal RTC timer keeps time;
//  - if the board loses power entirely, we restore the last saved time and flag it
//    as approximate ("~" on screen) until you set it again.
// Times are kept as local wall-clock time (TZ = UTC0). NTP + time zones are a later step.
class Clock {
 public:
  void begin() {
    setenv("TZ", "UTC0", 1);
    tzset();
    if (time(nullptr) < kMinValid) {
      Preferences p;
      p.begin("inkos", true);
      int64_t saved = p.getLong64("epoch", 0);
      p.end();
      if (saved >= kMinValid) {
        setEpoch(saved);
        approximate_ = true;
      } else {
        setEpoch(kDefault);
        approximate_ = true;
      }
    }
  }

  void set(const Date& d, int hour, int minute) {
    int64_t e = (int64_t)cal::toDays(d) * 86400 + hour * 3600 + minute * 60;
    setEpoch(e);
    approximate_ = false;
    persist();
  }

  void persist() const {
    Preferences p;
    p.begin("inkos", false);
    p.putLong64("epoch", (int64_t)time(nullptr));
    p.end();
  }

  Date today() const {
    time_t t = time(nullptr);
    return cal::fromDays((int32_t)(t / 86400));
  }
  int hour() const { return (int)((time(nullptr) % 86400) / 3600); }
  int minute() const { return (int)((time(nullptr) % 3600) / 60); }
  bool approximate() const { return approximate_; }

  // "9:41 PM" or "21:41"
  void format(char* out, size_t n, bool h24) const {
    int h = hour(), m = minute();
    if (h24) snprintf(out, n, "%02d:%02d", h, m);
    else snprintf(out, n, "%d:%02d %s", (h % 12) ? h % 12 : 12, m, h < 12 ? "AM" : "PM");
  }

 private:
  static constexpr int64_t kMinValid = 1704067200;  // 2024-01-01
  static constexpr int64_t kDefault = 1767225600;   // 2026-01-01
  static void setEpoch(int64_t e) {
    timeval tv{(time_t)e, 0};
    settimeofday(&tv, nullptr);
  }
  bool approximate_ = false;
};
