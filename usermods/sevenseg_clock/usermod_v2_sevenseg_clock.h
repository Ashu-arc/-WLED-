#pragma once
/*
 * usermod_v2_sevenseg_clock.h
 * 7-Segment WLED Digital Clock (HH:MM:SS) + 7-pixel weekday indicator
 * -------------------------------------------------------------------
 * Drop this file into: wled00/usermods/sevenseg_clock/
 * Then register it in: wled00/usermods_list.cpp  (see SETUP_INSTRUCTIONS.md)
 *
 * WIRING ORDER (single WS2812B chain, data-in to data-out):
 *   Digit0 (Hours tens)   -> segments a,b,c,d,e,f,g
 *   Digit1 (Hours units)  -> segments a,b,c,d,e,f,g
 *   Digit2 (Minutes tens) -> segments a,b,c,d,e,f,g
 *   Digit3 (Minutes units)-> segments a,b,c,d,e,f,g
 *   Digit4 (Seconds tens) -> segments a,b,c,d,e,f,g
 *   Digit5 (Seconds units)-> segments a,b,c,d,e,f,g
 *   Weekday0..6           -> Sun, Mon, Tue, Wed, Thu, Fri, Sat  (1 pixel each)
 *
 * Standard 7-segment layout reference:
 *      _a_
 *   f |   | b
 *     |_g_|
 *   e |   | c
 *     |_d_|
 */

#include "wled.h"

// ===================== USER CONFIGURATION =====================
#define LEDS_PER_SEGMENT   1     // how many physical LEDs make up ONE bar (increase if each
                                  // segment uses more than 1 LED, e.g. 2 or 3)
#define NUM_DIGITS         6     // H H : M M : S S
#define WEEKDAY_LEDS       7     // Sun..Sat, one pixel each
#define PIXEL_BASE_OFFSET  0     // index of the FIRST led used by this usermod
                                  // (0 unless you're sharing the strip with other effects)
// ================================================================

// Segment bit positions within a digit (bit0..bit6 = a..g)
#define SEG_A 0
#define SEG_B 1
#define SEG_C 2
#define SEG_D 3
#define SEG_E 4
#define SEG_F 5
#define SEG_G 6

// Which segments are lit for each digit 0-9. Bit order: (MSB) g f e d c b a (LSB)
static const uint8_t sevenSegDigitPatterns[10] = {
  0b0111111, // 0 -> a b c d e f
  0b0000110, // 1 -> b c
  0b1011011, // 2 -> a b g e d
  0b1001111, // 3 -> a b g c d
  0b1100110, // 4 -> f g b c
  0b1101101, // 5 -> a f g c d
  0b1111101, // 6 -> a f g e c d
  0b0000111, // 7 -> a b c
  0b1111111, // 8 -> all
  0b1101111  // 9 -> a b c d f g
};

class SevenSegClockUsermod : public Usermod {
  private:
    unsigned long lastUpdate = 0;

    // ---- Colors (edit these to taste) ----
    uint32_t colorOn     = RGBW32(255, 40, 0, 0);   // lit segment color
    uint32_t colorOff    = RGBW32(0, 0, 0, 0);      // unlit segment color
    uint32_t colorDayOn  = RGBW32(0, 150, 255, 0);  // today's weekday pixel
    uint32_t colorDayOff = RGBW32(0, 0, 0, 0);      // all other weekday pixels

    void setDigit(int digitIndex, uint8_t pattern) {
      int digitStart = PIXEL_BASE_OFFSET + digitIndex * 7 * LEDS_PER_SEGMENT;
      for (int seg = 0; seg < 7; seg++) {
        bool on = pattern & (1 << seg);
        uint32_t c = on ? colorOn : colorOff;
        for (int l = 0; l < LEDS_PER_SEGMENT; l++) {
          int idx = digitStart + seg * LEDS_PER_SEGMENT + l;
          strip.setPixelColor(idx, c);
        }
      }
    }

    void setWeekday(int wdayZeroIndexed /* 0=Sunday..6=Saturday */) {
      int base = PIXEL_BASE_OFFSET + NUM_DIGITS * 7 * LEDS_PER_SEGMENT;
      for (int i = 0; i < WEEKDAY_LEDS; i++) {
        strip.setPixelColor(base + i, (i == wdayZeroIndexed) ? colorDayOn : colorDayOff);
      }
    }

  public:
    // Total pixel count this usermod needs. Use this number as your
    // WLED "LED count" in Config -> LED Preferences.
    static const int TOTAL_PIXELS = NUM_DIGITS * 7 * LEDS_PER_SEGMENT + WEEKDAY_LEDS;

    void setup() override {
      // Nothing needed here. Make sure NTP + your timezone are enabled in
      // WLED's Config -> Time settings before flashing/using this.
    }

    void connected() override {}

    void loop() override {
      if (!ntpEnabled) return;              // Time sync must be ON (Config -> Time)
      if (millis() - lastUpdate < 500) return;
      lastUpdate = millis();

      updateLocalTime();
      uint8_t h  = hour(localTime);
      uint8_t mi = minute(localTime);
      uint8_t s  = second(localTime);
      uint8_t wd = weekday(localTime) - 1;  // TimeLib: 1=Sunday..7=Saturday -> 0..6

      setDigit(0, sevenSegDigitPatterns[h  / 10]);
      setDigit(1, sevenSegDigitPatterns[h  % 10]);
      setDigit(2, sevenSegDigitPatterns[mi / 10]);
      setDigit(3, sevenSegDigitPatterns[mi % 10]);
      setDigit(4, sevenSegDigitPatterns[s  / 10]);
      setDigit(5, sevenSegDigitPatterns[s  % 10]);
      setWeekday(wd);

      strip.trigger();  // push the frame out immediately
    }

    uint16_t getId() override { return USERMOD_ID_UNSPECIFIED; }
};
