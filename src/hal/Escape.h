#pragma once
// Escape hatch: hold BACK + UP while powering on / waking to boot the OTHER app slot
// (e.g. CrossPoint, which stays there after it installs InkOS).
// Runs as the very first thing in setup() — before the display or anything else that
// could fail — so it works even if InkOS can't draw a screen.
#include <Arduino.h>
#include <esp_ota_ops.h>
#include "ButtonDecode.h"

namespace escape {

inline bool comboHeld() {
  analogReadResolution(12);
  analogSetPinAttenuation(board::BTN_ADC_FRONT, ADC_11db);
  analogSetPinAttenuation(board::BTN_ADC_SIDE, ADC_11db);
  // Require the combo to be held steadily for ~300 ms (15 samples) to avoid false triggers.
  for (int i = 0; i < 15; i++) {
    if (decodeFront(analogRead(board::BTN_ADC_FRONT)) != Btn::Back) return false;
    if (decodeSide(analogRead(board::BTN_ADC_SIDE)) != Btn::Up) return false;
    delay(20);
  }
  return true;
}

// Point the bootloader at the other OTA slot and restart. Returns false (and changes
// nothing) if the other slot doesn't hold a valid image.
inline bool bootOtherSlot() {
  const esp_partition_t* other = esp_ota_get_next_update_partition(nullptr);
  if (!other) return false;
  if (esp_ota_set_boot_partition(other) != ESP_OK) return false;
  delay(50);
  esp_restart();
  return true;
}

inline void checkAtBoot() {
  if (comboHeld()) bootOtherSlot();
}

}  // namespace escape
