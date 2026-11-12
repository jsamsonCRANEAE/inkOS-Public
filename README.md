# InkOS — from-scratch firmware for the Xteink X4

A small, readable firmware you own end to end: hardware layer → button events → screen stack → apps.
v0.1 ships a home menu, persistent settings, date/time setting, a hardware diagnostics screen, and a month calendar.

## Build
```
pip install platformio
pio run                      # -> .pio/build/x4/firmware.bin
pio test -e native           # optional; or the one-liner below
g++ -std=c++17 -Iinclude -Isrc test/test_logic.cpp -o /tmp/t && /tmp/t
```

## Flash — safely
1. **Check your unit isn't USB-locked.** If it is, run the Xteink Unlocker (crosspointreader.com) first.
   Flashing a locked unit can brick it.
2. **Back up the stock flash** (16 MB) before anything else:
   `esptool.py --chip esp32c3 --port <PORT> read_flash 0 0x1000000 x4_stock_backup.bin`
3. Flash `firmware.bin` to the app slot with the web tool's **Custom .bin / OTA** option
   (xteink.dve.al or crosspointreader.com/#flash-tools). This keeps the existing bootloader and
   partition table, so "Swap boot partition" can still take you back to the other firmware.
   Avoid `pio run -t upload` until you have the backup — it rewrites the bootloader + partitions.
4. Serial log: `pio device monitor` (USB CDC, 115200).

## Phone-only route (no computer)
1. Push this project to a GitHub repo → `.github/workflows/build.yml` builds it in the cloud and
   attaches `update.bin` to a release named **latest**.
2. Download `update.bin` on your phone; copy it to the **root** of the X4's microSD card
   (USB-C microSD reader → Files app).
3. Card back in the X4, plug in USB power, hold **Power + Up** — the bootloader installs it.
4. To undo: download stock firmware from crosspointreader.com/debug, rename to `update.bin`, repeat.

## Controls
| Button | Menus | Calendar | Set date |
|---|---|---|---|
| Up / Down (side) | move (hold = repeat) | ±1 week | change value |
| Left / Right | move | ±1 day (hold = ±1 month) | choose field |
| Confirm | select / cycle setting | jump to today | save |
| Back | exit | exit | cancel |
| Power short | full refresh (clears ghosting) | | |
| Power hold 0.6 s | sleep | | |

## First boot checklist
1. **Settings → Diagnostics.** Press each button; the raw ADC values should land near
   Back 3470 / Confirm 2655 / Left 1470 / Right ~0 / Up 2205 / Down ~0.
   If a button misreads, adjust the thresholds in `include/board.h`.
2. If the screen is mirrored/upside-down, toggle **Settings → Orientation**.
   If the image is garbled or blank, your unit may have a UC8179/UC8279 controller instead of the
   SSD1677 — swap the driver class in `src/hal/Display.h` (the only file that knows the panel).
3. **Settings → Set date & time.** A `~` before the clock means the time was restored from flash
   after a full power loss and may be stale.

## Layout
```
include/board.h            pin map + button ADC thresholds (single source of truth)
src/hal/                   ButtonDecode (pure, tested) · Buttons · Display · Power
src/core/                  App (screen stack, status bar, sleep) · MenuScreen · Settings (NVS)
                           Clock · CalendarMath (pure, tested)
src/apps/                  Home+Settings · SetDate · Diagnostics · Calendar
test/test_logic.cpp        host tests: button FSM + calendar math (verified 1970–2100 vs Python)
```

## Adding an app
1. Subclass `Screen` (`title`, `handle`, `draw`; optional `tick`, `liveView`).
2. Add a `makeXxxScreen()` factory to `src/apps/Screens.h`.
3. Add a `MenuItem` for it in `makeHomeScreen()`.

## Known limits / next steps
- Time is manual. Next: WiFi NTP sync + time zone setting.
- After deep sleep the app restarts at the home menu (state isn't restored yet).
- No SD card use yet — natural next step is events from a `calendar.ics` / CSV on the card.
- Hardware facts come from community sources (Adafruit CircuitPython guide, FreeInk SDK,
  quireink_x4, Idered/xteink-x4); not from Xteink.
