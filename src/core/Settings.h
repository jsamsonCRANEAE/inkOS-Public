#pragma once
#include <Arduino.h>
#include <Preferences.h>

// Persistent user settings, stored in NVS flash (survive power loss and reflashes of the app).
struct Settings {
  uint8_t rotation = 3;       // GxEPD2 rotation: 3 = portrait, 1 = portrait flipped
  uint8_t weekStart = 0;      // 0 = Sunday, 1 = Monday
  uint8_t fullEvery = 10;     // full refresh after N partial refreshes
  uint8_t sleepMinutes = 5;   // auto-sleep after inactivity; 0 = never
  bool clock24h = false;
  bool showWeekNumbers = false;

  void load() {
    Preferences p;
    p.begin("inkos", true);
    rotation = p.getUChar("rot", rotation);
    weekStart = p.getUChar("wk", weekStart);
    fullEvery = p.getUChar("full", fullEvery);
    sleepMinutes = p.getUChar("sleep", sleepMinutes);
    clock24h = p.getBool("24h", clock24h);
    showWeekNumbers = p.getBool("wkno", showWeekNumbers);
    p.end();
  }

  void save() const {
    Preferences p;
    p.begin("inkos", false);
    p.putUChar("rot", rotation);
    p.putUChar("wk", weekStart);
    p.putUChar("full", fullEvery);
    p.putUChar("sleep", sleepMinutes);
    p.putBool("24h", clock24h);
    p.putBool("wkno", showWeekNumbers);
    p.end();
  }
};
