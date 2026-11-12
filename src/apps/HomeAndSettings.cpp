#include <string>
#include "Screens.h"
#include "../core/App.h"
#include "../core/MenuScreen.h"

ScreenPtr makeHomeScreen() {
  return ScreenPtr(new MenuScreen("InkOS", {
      {"Calendar", nullptr, [](App& a) { a.push(makeCalendarScreen()); }},
      {"Settings", nullptr, [](App& a) { a.push(makeSettingsScreen()); }},
      {"Sleep", nullptr, [](App& a) { a.sleepNow(); }},
  }));
}

// Cycle a value through a fixed list of options.
template <typename T, size_t N>
static T cycle(T cur, const T (&opts)[N]) {
  for (size_t i = 0; i < N; i++)
    if (opts[i] == cur) return opts[(i + 1) % N];
  return opts[0];
}

// Each setting: label, how to show its current value, what Confirm does.
// Changes are saved to flash immediately.
ScreenPtr makeSettingsScreen() {
  return ScreenPtr(new MenuScreen("Settings", {
      {"Orientation",
       [](App& a) { return std::string(a.settings.rotation == 3 ? "Portrait" : "Flipped"); },
       [](App& a) {
         a.settings.rotation = a.settings.rotation == 3 ? 1 : 3;
         a.display.setRotation(a.settings.rotation);
         a.settings.save();
       }},
      {"Week starts",
       [](App& a) { return std::string(a.settings.weekStart ? "Monday" : "Sunday"); },
       [](App& a) { a.settings.weekStart ^= 1; a.settings.save(); }},
      {"Clock format",
       [](App& a) { return std::string(a.settings.clock24h ? "24-hour" : "12-hour"); },
       [](App& a) { a.settings.clock24h = !a.settings.clock24h; a.settings.save(); }},
      {"Week numbers",
       [](App& a) { return std::string(a.settings.showWeekNumbers ? "On" : "Off"); },
       [](App& a) { a.settings.showWeekNumbers = !a.settings.showWeekNumbers; a.settings.save(); }},
      {"Full refresh",
       [](App& a) {
         return a.settings.fullEvery == 1 ? std::string("Every page")
                                          : "Every " + std::to_string(a.settings.fullEvery);
       },
       [](App& a) {
         static const uint8_t opts[] = {1, 5, 10, 20};
         a.settings.fullEvery = cycle(a.settings.fullEvery, opts);
         a.settings.save();
       }},
      {"Auto-sleep",
       [](App& a) {
         return a.settings.sleepMinutes ? std::to_string(a.settings.sleepMinutes) + " min" : std::string("Off");
       },
       [](App& a) {
         static const uint8_t opts[] = {2, 5, 10, 0};
         a.settings.sleepMinutes = cycle(a.settings.sleepMinutes, opts);
         a.settings.save();
       }},
      {"Set date & time", nullptr, [](App& a) { a.push(makeSetDateScreen()); }},
      {"Diagnostics", nullptr, [](App& a) { a.push(makeDiagnosticsScreen()); }},
  }));
}
