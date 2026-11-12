#pragma once
#include <Arduino.h>
#include "ButtonDecode.h"

// Reads the two ADC ladders + power pin and turns them into events.
class Buttons {
 public:
  void begin() {
    analogReadResolution(12);
    analogSetPinAttenuation(board::BTN_ADC_FRONT, ADC_11db);
    analogSetPinAttenuation(board::BTN_ADC_SIDE, ADC_11db);
    pinMode(board::BTN_POWER, INPUT_PULLUP);
  }

  Event poll() {
    lastFront_ = analogRead(board::BTN_ADC_FRONT);
    lastSide_ = analogRead(board::BTN_ADC_SIDE);
    bool pwr = digitalRead(board::BTN_POWER) == LOW;
    return fsm_.update(decodeAll(lastFront_, lastSide_, pwr), millis());
  }

  bool powerHeld() const { return digitalRead(board::BTN_POWER) == LOW; }
  uint16_t rawFront() const { return lastFront_; }
  uint16_t rawSide() const { return lastSide_; }
  Btn held() const { return fsm_.held(); }

 private:
  ButtonFsm fsm_;
  uint16_t lastFront_ = 4095, lastSide_ = 4095;
};
