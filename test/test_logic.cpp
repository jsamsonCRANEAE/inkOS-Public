// Host-side tests for the hardware-independent logic.
// Build: g++ -std=c++17 -Iinclude -Isrc test/test_logic.cpp -o /tmp/t && /tmp/t
// Emits a day-by-day table on stdout for cross-checking against Python's datetime.
#include <cassert>
#include <cstdio>
#include <cstring>
#include "core/CalendarMath.h"
#include "hal/ButtonDecode.h"

static int fails = 0;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static void testButtons() {
  // Decoding at the measured centres
  CHECK(decodeFront(4095) == Btn::None);
  CHECK(decodeFront(3470) == Btn::Back);
  CHECK(decodeFront(2655) == Btn::Confirm);
  CHECK(decodeFront(1470) == Btn::Left);
  CHECK(decodeFront(3) == Btn::Right);
  CHECK(decodeSide(4095) == Btn::None);
  CHECK(decodeSide(2205) == Btn::Up);
  CHECK(decodeSide(3) == Btn::Down);
  // +/-15% tolerance around each centre stays in its band
  CHECK(decodeFront(3470 * 0.9) == Btn::Back);
  CHECK(decodeFront(2655 * 1.12) == Btn::Confirm && decodeFront(2655 * 0.85) == Btn::Confirm);
  CHECK(decodeFront(1470 * 1.3) == Btn::Left && decodeFront(1470 * 0.6) == Btn::Left);
  CHECK(decodeSide(2205 * 1.3) == Btn::Up && decodeSide(2205 * 0.6) == Btn::Up);
  CHECK(decodeAll(3, 3, true) == Btn::Power);

  auto run = [](ButtonFsm& f, Btn b, uint32_t from, uint32_t to, Event* out, int& n) {
    for (uint32_t t = from; t < to; t += 5) { Event e = f.update(b, t); if (e) out[n++] = e; }
  };
  // Short press: 120 ms -> exactly one Press on release
  { ButtonFsm f; Event ev[16]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Confirm, 100, 220, ev, n); run(f, Btn::None, 220, 400, ev, n);
    CHECK(n == 1 && ev[0].type == EvType::Press && ev[0].btn == Btn::Confirm); }
  // Bounce shorter than debounce is ignored
  { ButtonFsm f; Event ev[16]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Left, 100, 115, ev, n); run(f, Btn::None, 115, 400, ev, n);
    CHECK(n == 0); }
  // Hold 1.5 s on Right -> one Long, then repeats, and no Press on release
  { ButtonFsm f; Event ev[64]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Right, 100, 1600, ev, n); run(f, Btn::None, 1600, 1800, ev, n);
    CHECK(n >= 4 && ev[0].type == EvType::Long);
    for (int i = 1; i < n; i++) CHECK(ev[i].type == EvType::Repeat);
    printf("hold-1.5s events: %d (1 long + %d repeats)\n", n, n - 1); }
  // PowerGuard: held at boot is ignored until released; stuck LOW disarms after 8 s
  { PowerGuard g;
    CHECK(!g.filter(true, 10));            // held from wake press: ignored
    CHECK(!g.filter(false, 20));           // released -> armed
    CHECK(g.filter(true, 30));             // real press counts
    CHECK(g.filter(true, 30 + 7000));      // still within 8 s
    CHECK(!g.filter(true, 30 + 9000));     // stuck: ignored
    CHECK(!g.filter(true, 30 + 20000));    // stays ignored
    CHECK(!g.filter(false, 30 + 20010));   // released -> re-armed
    CHECK(g.filter(true, 30 + 20020)); }
  // Power short hold (1 s) is a Press, not a Long (sleep needs 1.5 s)
  { ButtonFsm f; Event ev[16]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Power, 100, 1100, ev, n); run(f, Btn::None, 1100, 1300, ev, n);
    CHECK(n == 1 && ev[0].type == EvType::Press && ev[0].btn == Btn::Power); }
  // Long on Power does NOT repeat (so it can't trigger sleep twice)
  { ButtonFsm f; Event ev[64]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Power, 100, 3000, ev, n); run(f, Btn::None, 3000, 3200, ev, n);
    CHECK(n == 1 && ev[0].type == EvType::Long); }
  // Sliding from one button straight to another emits Press for the first
  { ButtonFsm f; Event ev[16]; int n = 0;
    run(f, Btn::None, 0, 100, ev, n); run(f, Btn::Up, 100, 300, ev, n); run(f, Btn::Down, 300, 500, ev, n); run(f, Btn::None, 500, 700, ev, n);
    CHECK(n == 2 && ev[0].btn == Btn::Up && ev[1].btn == Btn::Down); }
}

static void testCalendarSpot() {
  CHECK(cal::dayOfWeek(2026, 11, 11) == 3);  // Wednesday
  CHECK(cal::daysInMonth(2024, 2) == 29 && cal::daysInMonth(2100, 2) == 28 && cal::daysInMonth(2000, 2) == 29);
  Date d = cal::addMonths(Date{2026, 1, 31}, 1);  CHECK(d == (Date{2026, 2, 28}));
  d = cal::addMonths(Date{2024, 3, 31}, -1);       CHECK(d == (Date{2024, 2, 29}));
  d = cal::addMonths(Date{2026, 12, 15}, 1);       CHECK(d == (Date{2027, 1, 15}));
  d = cal::addMonths(Date{2026, 1, 15}, -1);       CHECK(d == (Date{2025, 12, 15}));
  CHECK(cal::rowsNeeded(2026, 2, 0) == 4);   // Feb 2026 starts Sunday: perfect 4 rows
  CHECK(cal::rowsNeeded(2026, 8, 0) == 6);   // Aug 2026 starts Saturday: 6 rows
  CHECK(cal::firstColumn(2026, 11, 0) == 0); // Nov 1 2026 is a Sunday
  CHECK(cal::firstColumn(2026, 11, 1) == 6); // ...last column when weeks start Monday
}

int main(int argc, char** argv) {
  testButtons();
  testCalendarSpot();
  // Dump table: days since epoch -> y m d weekday isoWeek, plus round-trip check
  if (argc > 1 && strcmp(argv[1], "--dump") == 0) {
    for (int32_t z = 0; z <= cal::toDays(Date{2100, 12, 31}); z++) {
      Date d = cal::fromDays(z);
      if (cal::toDays(d) != z) { fprintf(stderr, "roundtrip fail %d\n", z); fails++; }
      printf("%d %d %d %d %d %d\n", z, d.y, d.m, d.d, cal::dayOfWeek(d.y, d.m, d.d), cal::isoWeek(d));
    }
  }
  fprintf(stderr, fails ? "%d FAILURES\n" : "all checks passed\n", fails);
  return fails ? 1 : 0;
}
