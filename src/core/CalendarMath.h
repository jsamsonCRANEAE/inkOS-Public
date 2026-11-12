#pragma once
// Pure Gregorian date math. No Arduino dependencies — unit-tested on the host.
#include <stdint.h>

struct Date {
  int16_t y;
  int8_t m;  // 1..12
  int8_t d;  // 1..31
  bool operator==(const Date& o) const { return y == o.y && m == o.m && d == o.d; }
  bool operator!=(const Date& o) const { return !(*this == o); }
};

namespace cal {

inline bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0); }

inline int daysInMonth(int y, int m) {
  static const int8_t dim[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return (m == 2 && isLeap(y)) ? 29 : dim[m - 1];
}

// 0 = Sunday .. 6 = Saturday (Sakamoto's method)
inline int dayOfWeek(int y, int m, int d) {
  static const int8_t t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Days since 1970-01-01 (Howard Hinnant's algorithm), and back.
inline int32_t toDays(const Date& dt) {
  int y = dt.y - (dt.m <= 2);
  int era = (y >= 0 ? y : y - 399) / 400;
  int yoe = y - era * 400;
  int mp = (dt.m + 9) % 12;
  int doy = (153 * mp + 2) / 5 + dt.d - 1;
  int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

inline Date fromDays(int32_t z) {
  z += 719468;
  int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  int32_t doe = z - era * 146097;
  int32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int32_t y = yoe + era * 400;
  int32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  int32_t mp = (5 * doy + 2) / 153;
  int d = doy - (153 * mp + 2) / 5 + 1;
  int m = mp < 10 ? mp + 3 : mp - 9;
  return Date{(int16_t)(y + (m <= 2)), (int8_t)m, (int8_t)d};
}

inline Date addDays(const Date& dt, int n) { return fromDays(toDays(dt) + n); }

// Move by whole months, clamping the day (Jan 31 + 1 month -> Feb 28/29).
inline Date addMonths(const Date& dt, int n) {
  int idx = dt.y * 12 + (dt.m - 1) + n;
  int y = idx / 12, m = idx % 12 + 1;
  int dim = daysInMonth(y, m);
  return Date{(int16_t)y, (int8_t)m, (int8_t)(dt.d > dim ? dim : dt.d)};
}

// Column (0..6) of the 1st of the month, given which weekday starts the week (0=Sun, 1=Mon).
inline int firstColumn(int y, int m, int weekStart) {
  return (dayOfWeek(y, m, 1) - weekStart + 7) % 7;
}

// Rows needed for the month grid (4, 5 or 6).
inline int rowsNeeded(int y, int m, int weekStart) {
  int cells = firstColumn(y, m, weekStart) + daysInMonth(y, m);
  return (cells + 6) / 7;
}

// ISO-8601 week number (weeks start Monday; week 1 contains Jan 4).
inline int isoWeek(const Date& dt) {
  int wd = (dayOfWeek(dt.y, dt.m, dt.d) + 6) % 7;  // 0 = Monday
  Date thursday = addDays(dt, 3 - wd);
  Date jan1{thursday.y, 1, 1};
  return (toDays(thursday) - toDays(jan1)) / 7 + 1;
}

inline const char* monthName(int m) {
  static const char* n[12] = {"January", "February", "March",     "April",   "May",      "June",
                              "July",    "August",   "September", "October", "November", "December"};
  return n[m - 1];
}
inline const char* weekdayShort(int wd) {  // 0 = Sunday
  static const char* n[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return n[wd];
}
inline const char* weekdayLong(int wd) {
  static const char* n[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  return n[wd];
}

}  // namespace cal
