// Install update.bin from the microSD card — InkOS can always replace itself.
//
// Uses the ESP32 OTA mechanism: the new image is written to the *other* app partition,
// verified, and only then marked bootable. If anything fails before the end, the running
// firmware is untouched and stays the one that boots.
//
// Safety gates before writing:
//   - file exists at the card root, is a plausible size, and starts with the ESP image magic (0xE9)
//   - USB power connected, or battery >= 40 %
//   - explicit Confirm from the user
// Implementation: native esp_ota_begin/write/end. The Arduino Update wrapper failed
// its read-back check on the X4 ("Flash Read Failed").
// After success: /update.bin is renamed /update.done (so it isn't re-installed) and the device restarts.
#include <SD.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "Screens.h"
#include "../core/App.h"
#include "../hal/Power.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

namespace {
constexpr const char* kFile = "/update.bin";
constexpr const char* kDone = "/update.done";
constexpr size_t kMinSize = 64 * 1024;
constexpr uint8_t kImageMagic = 0xE9;  // first byte of every ESP32 app image
constexpr int kMinBattery = 40;

enum class State { Checking, Ready, Blocked, Installing, Failed };
}  // namespace

class SdUpdateScreen : public Screen {
 public:
  const char* title() const override { return "Install from SD"; }

  void onEnter(App& app) override {
    if (state_ != State::Checking) return;
    check(app);
  }

  bool handle(const Event& ev, App& app) override {
    if (ev.type != EvType::Press) return false;
    if (ev.btn == Btn::Confirm && state_ == State::Ready) {
      install(app);  // only returns on failure (success restarts)
      return true;
    }
    if (ev.btn == Btn::Confirm && (state_ == State::Blocked || state_ == State::Failed)) {
      state_ = State::Checking;
      check(app);  // re-check (e.g. after plugging in USB)
      return true;
    }
    return false;  // Back exits
  }

  void draw(Panel& g, App&) override {
    int y = ui::STATUS_H + 50;
    auto line = [&](const GFXfont* f, const char* s, int gap = 34) {
      g.setFont(f); g.setCursor(ui::MARGIN, y); g.print(s); y += gap;
    };
    switch (state_) {
      case State::Ready:
        line(&FreeSansBold12pt7b, "update.bin found");
        line(&FreeSans12pt7b, sizeText_);
        line(&FreeSans12pt7b, "Image header OK");
        line(&FreeSans12pt7b, slotText_, 50);
        line(&FreeSans9pt7b, "The current firmware stays bootable until", 26);
        line(&FreeSans9pt7b, "the new one is fully written and verified.", 40);
        line(&FreeSansBold12pt7b, "Confirm = install", 34);
        line(&FreeSans12pt7b, "Back = cancel");
        break;
      case State::Blocked:
      case State::Failed:
        line(&FreeSansBold12pt7b, state_ == State::Failed ? "Install failed" : "Cannot install");
        line(&FreeSans12pt7b, msg_);
        line(&FreeSans9pt7b, slotText_, 40);
        line(&FreeSans9pt7b, "Running firmware is unchanged.", 26);
        line(&FreeSans9pt7b, "Confirm = check again   Back = exit");
        break;
      default:
        line(&FreeSans12pt7b, "Checking card...");
    }
  }

 private:
  void check(App& app) {
    msg_[0] = 0;
    if (!SD.begin(board::SD_CS, SPI, 20000000)) return block("No SD card / not FAT32");
    File f = SD.open(kFile, FILE_READ);
    if (!f) return block("No /update.bin in card root");
    size_ = f.size();
    int magic = f.read();
    f.close();
    if (size_ < kMinSize) return block("File too small");
    if (magic != kImageMagic) return block("Not an ESP32 firmware image");
    bool usb = power::usbPowered();
    int batt = power::batteryPercent();
    if (!usb && batt < kMinBattery) {
      snprintf(msg_, sizeof msg_, "Battery %d%% - plug in USB", batt);
      return block(nullptr);
    }
    const esp_partition_t* tgt = esp_ota_get_next_update_partition(nullptr);
    if (!tgt) return block("No spare app slot");
    if (size_ > tgt->size) return block("File larger than app slot");
    snprintf(slotText_, sizeof slotText_, "Target: %s @0x%06lx", tgt->label, (unsigned long)tgt->address);
    snprintf(sizeText_, sizeof sizeText_, "%u KB, %s", (unsigned)(size_ / 1024), usb ? "USB power" : "on battery");
    state_ = State::Ready;
  }

  void block(const char* why) {
    if (why) snprintf(msg_, sizeof msg_, "%s", why);
    state_ = State::Blocked;
  }

  // Draw a progress frame directly (the App's render loop isn't running while we block).
  void progressFrame(App& app, int pct) {
    Panel& g = app.display.frame();
    g.setFont(&FreeSansBold12pt7b);
    g.setCursor(ui::MARGIN, 120);
    g.print("Installing update.bin");
    g.setFont(&FreeSans12pt7b);
    g.setCursor(ui::MARGIN, 170);
    g.print("Do not power off or remove the card");
    const int x = ui::MARGIN, y = 230, w = g.width() - 2 * ui::MARGIN, h = 40;
    g.drawRect(x, y, w, h, GxEPD_BLACK);
    g.drawRect(x + 1, y + 1, w - 2, h - 2, GxEPD_BLACK);
    g.fillRect(x + 4, y + 4, (w - 8) * pct / 100, h - 8, GxEPD_BLACK);
    char buf[16];
    snprintf(buf, sizeof buf, "%d %%", pct);
    g.setCursor(ui::MARGIN, y + h + 40);
    g.print(buf);
    app.display.show(0);  // partial refresh: fast, no flashing mid-install
  }

  // Native ESP-IDF OTA: begin (erases target) -> write chunks -> end (validates image)
  // -> set boot partition. The running slot is never touched; boot only switches
  // after the new image validates.
  void install(App& app) {
    File f = SD.open(kFile, FILE_READ);
    if (!f) return fail("Card read error");
    const esp_partition_t* tgt = esp_ota_get_next_update_partition(nullptr);
    if (!tgt) { f.close(); return fail("No spare app slot"); }
    progressFrame(app, 0);

    esp_ota_handle_t h = 0;
    esp_err_t err = esp_ota_begin(tgt, f.size(), &h);
    if (err != ESP_OK) { f.close(); return failErr("begin", err, tgt); }

    static uint8_t buf[4096];
    size_t done = 0, total = f.size();
    int shown = 0;
    while (done < total) {
      size_t n = f.read(buf, sizeof buf);
      if (n == 0) { f.close(); esp_ota_abort(h); return fail("SD read stopped early"); }
      err = esp_ota_write(h, buf, n);
      if (err != ESP_OK) { f.close(); esp_ota_abort(h); return failErr("write", err, tgt); }
      done += n;
      int pct = (int)(done * 100 / total);
      if (pct >= shown + 25 && pct < 100) { shown = pct - pct % 25; progressFrame(app, shown); }
    }
    f.close();

    err = esp_ota_end(h);  // validates the whole image (magic, segments, checksum, SHA)
    if (err != ESP_OK) return failErr("verify", err, tgt);
    err = esp_ota_set_boot_partition(tgt);
    if (err != ESP_OK) return failErr("activate", err, tgt);

    SD.remove(kDone);
    SD.rename(kFile, kDone);
    progressFrame(app, 100);
    app.clock.persist();
    delay(500);
    ESP.restart();
  }

  void failErr(const char* step, esp_err_t err, const esp_partition_t* p) {
    snprintf(msg_, sizeof msg_, "%s: %s", step, esp_err_to_name(err));
    snprintf(slotText_, sizeof slotText_, "slot %s @0x%06lx", p ? p->label : "?", p ? (unsigned long)p->address : 0UL);
    state_ = State::Failed;
  }

  void fail(const char* why) {
    snprintf(msg_, sizeof msg_, "%s", why ? why : "Unknown error");
    state_ = State::Failed;
  }

  State state_ = State::Checking;
  size_t size_ = 0;
  char msg_[48] = {0};
  char sizeText_[40] = {0};
  char slotText_[40] = {0};
};

ScreenPtr makeSdUpdateScreen() { return ScreenPtr(new SdUpdateScreen()); }
