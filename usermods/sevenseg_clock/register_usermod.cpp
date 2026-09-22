#include "wled.h"
#include "usermod_v2_sevenseg_clock.h"

static SevenSegClockUsermod sevenseg_clock_instance;

struct SevenSegClockRegistrar {
  SevenSegClockRegistrar() { UsermodManager::add(&sevenseg_clock_instance); }
} sevenSegClockRegistrar;
