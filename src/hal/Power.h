#pragma once
#include <Arduino.h>
#include <esp_sleep.h>
#include "board.h"

namespace power {

// Battery voltage in volts (pin sees half the cell voltage).
inline float batteryVolts() {
  analogSetPinAttenuation(board::BATTERY_ADC, ADC_11db);
  uint32_t mv = 0;
  for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(board::BATTERY_ADC);
  return (mv / 8) * 2 / 1000.0f;
}

// Rough LiPo curve: 3.30 V = 0 %, 4.15 V = 100 %. Good enough for an icon.
inline int batteryPercent() {
  float v = batteryVolts();
  int p = (int)((v - 3.30f) / (4.15f - 3.30f) * 100.0f);
  return p < 0 ? 0 : (p > 100 ? 100 : p);
}

inline bool usbPowered() {
  pinMode(board::USB_DETECT, INPUT);
  return digitalRead(board::USB_DETECT) == HIGH;
}

// Deep sleep; the power button (active LOW) wakes us.
[[noreturn]] inline void deepSleep() {
  // Wait for release so we don't wake immediately on the same press.
  uint32_t t0 = millis();  // wait for release, but never hang on a stuck input
  while (digitalRead(board::BTN_POWER) == LOW && millis() - t0 < 3000) delay(10);
  delay(50);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << board::BTN_POWER, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
  while (true) {}
}

inline bool wokeFromButton() {
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO;
}

}  // namespace power
