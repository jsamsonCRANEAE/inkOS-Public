#pragma once
#include "../core/Screen.h"

// Factories for every screen in the firmware. Add new apps here.
ScreenPtr makeHomeScreen();
ScreenPtr makeSettingsScreen();
ScreenPtr makeSetDateScreen();
ScreenPtr makeDiagnosticsScreen();
ScreenPtr makeCalendarScreen();
ScreenPtr makeSdUpdateScreen();
