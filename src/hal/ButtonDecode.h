#pragma once
// Pure (hardware-free) button decoding + event state machine. Unit-tested on the host.
#include <stdint.h>
#include "board.h"

enum class Btn : uint8_t { None, Back, Confirm, Left, Right, Up, Down, Power };
enum class EvType : uint8_t { None, Press, Long, Repeat };

struct Event {
  EvType type = EvType::None;
  Btn btn = Btn::None;
  explicit operator bool() const { return type != EvType::None; }
};

inline Btn decodeFront(uint16_t raw) {
  using namespace board;
  if (raw >= FRONT_IDLE_MIN) return Btn::None;
  if (raw >= FRONT_BACK_MIN) return Btn::Back;
  if (raw >= FRONT_CONFIRM_MIN) return Btn::Confirm;
  if (raw >= FRONT_LEFT_MIN) return Btn::Left;
  return Btn::Right;
}

inline Btn decodeSide(uint16_t raw) {
  using namespace board;
  if (raw >= SIDE_IDLE_MIN) return Btn::None;
  if (raw >= SIDE_UP_MIN) return Btn::Up;
  return Btn::Down;
}

// Combine raw readings into one logical button. Power wins, then front, then side.
inline Btn decodeAll(uint16_t front, uint16_t side, bool powerLow) {
  if (powerLow) return Btn::Power;
  Btn f = decodeFront(front);
  if (f != Btn::None) return f;
  return decodeSide(side);
}

// Debounced press / long-press / auto-repeat state machine.
// Feed it the decoded button and a millisecond clock every few ms.
//  - Short press  -> Press on release (if released before LONG_MS)
//  - Held         -> Long once at LONG_MS, then Repeat every REPEAT_MS (Up/Down/Left/Right only)
class ButtonFsm {
 public:
  static constexpr uint32_t DEBOUNCE_MS = 30;
  static constexpr uint32_t LONG_MS = 600;
  static constexpr uint32_t REPEAT_MS = 180;

  Event update(Btn raw, uint32_t now) {
    // Debounce: the raw value must be stable for DEBOUNCE_MS before we accept it.
    if (raw != candidate_) {
      candidate_ = raw;
      candidateSince_ = now;
      return {};
    }
    if (now - candidateSince_ < DEBOUNCE_MS) return {};
    Btn stable = candidate_;

    Event ev;
    if (stable != held_) {
      // Edge. A release of a button that never went "long" is a Press.
      if (held_ != Btn::None && !longFired_) ev = {EvType::Press, held_};
      held_ = stable;
      heldSince_ = now;
      longFired_ = false;
      return ev;
    }
    if (held_ == Btn::None) return {};

    uint32_t heldFor = now - heldSince_;
    if (!longFired_ && heldFor >= LONG_MS) {
      longFired_ = true;
      lastRepeat_ = now;
      return {EvType::Long, held_};
    }
    if (longFired_ && repeats(held_) && now - lastRepeat_ >= REPEAT_MS) {
      lastRepeat_ = now;
      return {EvType::Repeat, held_};
    }
    return {};
  }

  Btn held() const { return held_; }

 private:
  static bool repeats(Btn b) {
    return b == Btn::Up || b == Btn::Down || b == Btn::Left || b == Btn::Right;
  }
  Btn candidate_ = Btn::None;
  uint32_t candidateSince_ = 0;
  Btn held_ = Btn::None;
  uint32_t heldSince_ = 0;
  uint32_t lastRepeat_ = 0;
  bool longFired_ = false;
};

inline const char* btnName(Btn b) {
  switch (b) {
    case Btn::Back: return "Back";
    case Btn::Confirm: return "Confirm";
    case Btn::Left: return "Left";
    case Btn::Right: return "Right";
    case Btn::Up: return "Up";
    case Btn::Down: return "Down";
    case Btn::Power: return "Power";
    default: return "-";
  }
}
