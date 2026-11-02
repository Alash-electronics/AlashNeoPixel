// Имитация Arduino.h для тестов AlashNeoPixel: часы millis() и micros(), Print, PROGMEM как обычная память.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define OUTPUT 1
#define LOW 0
inline uint32_t g_ms = 1000, g_us = 5000000;
inline uint32_t millis() { return g_ms; }
inline uint32_t micros() { return g_us; }
inline void delay(uint32_t ms) { g_ms += ms; g_us += ms * 1000; }
inline void delayMicroseconds(uint32_t us) { g_us += us; }
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
class Print {
 public:
  std::string text;
  void put(const char *s) { printf("%s", s); text += s; }
  void print(const char *s) { put(s); }
  void print(const __FlashStringHelper *s) { put((const char *)s); }
  void println(const __FlashStringHelper *s) { put((const char *)s); put("\n"); }
  void print(long v) { char b[24]; snprintf(b, sizeof b, "%ld", v); put(b); }
  void println(long v) { print(v); put("\n"); }
  void println(const char *s = "") { put(s); put("\n"); }
};
