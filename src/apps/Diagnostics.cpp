#include "Screens.h"
#include "../core/App.h"
#include "../hal/Power.h"
#include <esp_ota_ops.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Live hardware readout. Use it to calibrate the button thresholds in board.h:
// press each button and note the raw ADC value. Hold Back to exit.
class DiagnosticsScreen : public Screen {
 public:
  const char* title() const override { return "Diagnostics"; }

  bool handle(const Event& ev, App& app) override {
    if (ev.btn == Btn::Back && ev.type == EvType::Long) { app.pop(); return false; }
    last_ = ev;
    return true;  // swallow everything else (incl. Back short press) so it can be tested
  }

  bool tick(App&) override { return true; }  // refresh once a second
  bool liveView() const override { return true; }

  void draw(Panel& g, App& app) override {
    char line[48];
    int y = ui::STATUS_H + 50;
    auto row = [&](const char* s) { g.setCursor(ui::MARGIN, y); g.print(s); y += 40; };
    g.setFont(&FreeMono12pt7b);

    snprintf(line, sizeof line, "Front ADC : %4u", app.buttons.rawFront()); row(line);
    snprintf(line, sizeof line, "Side ADC  : %4u", app.buttons.rawSide()); row(line);
    snprintf(line, sizeof line, "Held      : %s", btnName(app.buttons.held())); row(line);
    snprintf(line, sizeof line, "Last event: %s %s", btnName(last_.btn),
             last_.type == EvType::Long ? "(long)" : last_.type == EvType::Repeat ? "(rep)" : ""); row(line);
    y += 20;
    snprintf(line, sizeof line, "Battery   : %.2f V %d%%", power::batteryVolts(), power::batteryPercent()); row(line);
    snprintf(line, sizeof line, "USB power : %s", power::usbPowered() ? "yes" : "no"); row(line);
    snprintf(line, sizeof line, "Free heap : %u KB", (unsigned)(ESP.getFreeHeap() / 1024)); row(line);
    snprintf(line, sizeof line, "Min heap  : %u KB", (unsigned)(ESP.getMinFreeHeap() / 1024)); row(line);
    snprintf(line, sizeof line, "Uptime    : %lu s", (unsigned long)(millis() / 1000)); row(line);
    snprintf(line, sizeof line, "Panel     : %dx%d", g.width(), g.height()); row(line);
    const esp_partition_t* run = esp_ota_get_running_partition();
    const esp_partition_t* oth = esp_ota_get_next_update_partition(nullptr);
    esp_app_desc_t d;
    bool othOk = oth && esp_ota_get_partition_description(oth, &d) == ESP_OK;
    snprintf(line, sizeof line, "Running   : %s", run ? run->label : "?"); row(line);
    snprintf(line, sizeof line, "Other     : %s %s", oth ? oth->label : "-", othOk ? "valid" : "EMPTY/BAD"); row(line);

    g.setFont(&FreeSans9pt7b);
    g.setCursor(ui::MARGIN, g.height() - 40);
    g.print("Press buttons to read raw ADC values.");
    g.setCursor(ui::MARGIN, g.height() - 16);
    g.print("Hold Back to exit");
  }

 private:
  Event last_;
};

ScreenPtr makeDiagnosticsScreen() { return ScreenPtr(new DiagnosticsScreen()); }
