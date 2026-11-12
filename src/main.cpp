// InkOS — a from-scratch firmware for the Xteink X4.
#include <Arduino.h>
#include "core/App.h"
#include "apps/Screens.h"

static App app;

void setup() {
  Serial.begin(115200);
  app.begin(makeHomeScreen());
  Serial.println("InkOS up");
}

void loop() { app.loop(); }
