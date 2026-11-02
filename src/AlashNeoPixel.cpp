// MIT License. Alash Electronics.
#include "AlashNeoPixel.h"
#include <stdlib.h>
#include <string.h>

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// ---- Какой драйвер вывода есть для этой платы -----------------------------------------------------
#if defined(ARDUINO_ARCH_ESP32) && defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
#define ALASH_NP_RMT 1
#elif defined(ARDUINO_ARCH_ESP8266)
#define ALASH_NP_ESP8266 1
#elif defined(__AVR__) && defined(F_CPU) && F_CPU == 16000000UL
#define ALASH_NP_AVR16 1
#endif
#if defined(ALASH_NP_RMT) || defined(ALASH_NP_ESP8266) || defined(ALASH_NP_AVR16)
#define ALASH_NP_HAS_DRIVER 1
#endif

#ifndef IRAM_ATTR
#define IRAM_ATTR ICACHE_RAM_ATTR
#endif

static const uint8_t GAMMA8[256] PROGMEM = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6,
  6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12,
  12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
  20, 20, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29,
  30, 30, 31, 32, 33, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41,
  42, 43, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53, 54, 55,
  56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
  73, 74, 75, 76, 77, 78, 79, 81, 82, 83, 84, 85, 87, 88, 89, 90,
  91, 93, 94, 95, 97, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 111,
  113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
  137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161,
  163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190,
  192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
  223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253, 255,
};

// Порядок байтов в ленте: индексы красного, зелёного и синего (белый всегда четвёртый)
static const uint8_t ORDER_TABLE[8][3] PROGMEM = {
  {1, 0, 2}, {0, 1, 2}, {1, 2, 0}, {0, 2, 1}, {2, 0, 1}, {2, 1, 0}, {1, 0, 2}, {0, 1, 2}};

static inline uint8_t xf(uint8_t v, uint16_t scale, bool useGamma) {    // гамма и яркость одного байта
  if (useGamma) v = pgm_read_byte(&GAMMA8[v]);
  return (uint8_t)(((uint16_t)v * scale) >> 8);
}

bool AlashNeoPixel::hasDriver() {
#if defined(ALASH_NP_HAS_DRIVER)
  return true;
#else
  return false;
#endif
}

// ---- Цвет -------------------------------------------------------------------------------------
uint8_t AlashNeoPixel::gamma8(uint8_t v) { return pgm_read_byte(&GAMMA8[v]); }

uint32_t AlashNeoPixel::ColorHSV(uint16_t h, uint8_t s, uint8_t v) {
  h %= 360;
  uint8_t region = h / 60;
  uint16_t rem = (uint16_t)(h % 60) * 255 / 60;
  uint8_t p = ((uint16_t)v * (255 - s)) / 255;
  uint8_t q = ((uint16_t)v * (255 - ((uint16_t)s * rem) / 255)) / 255;
  uint8_t t = ((uint16_t)v * (255 - ((uint16_t)s * (255 - rem)) / 255)) / 255;
  switch (region) {
    case 0: return Color(v, t, p);
    case 1: return Color(q, v, p);
    case 2: return Color(p, v, t);
    case 3: return Color(p, q, v);
    case 4: return Color(t, p, v);
    default: return Color(v, p, q);
  }
}

uint32_t AlashNeoPixel::Wheel(uint8_t pos) { return ColorHSV((uint16_t)pos * 360 / 256); }

uint32_t AlashNeoPixel::Blend(uint32_t a, uint32_t b, uint8_t amt) {
  uint32_t out = 0;
  for (uint8_t sh = 0; sh < 32; sh += 8) {
    uint16_t x = (a >> sh) & 0xFF, y = (b >> sh) & 0xFF;
    out |= (uint32_t)((x * (255 - amt) + y * amt) / 255) << sh;
  }
  return out;
}

void AlashNeoPixel::orderIndex(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &w) const {
  r = pgm_read_byte(&ORDER_TABLE[_order][0]);
  g = pgm_read_byte(&ORDER_TABLE[_order][1]);
  b = pgm_read_byte(&ORDER_TABLE[_order][2]);
  w = 3;
}

// ---- Начало работы -----------------------------------------------------------------------------
bool AlashNeoPixel::begin() {
  if (_begun) return true;
  if (_n == 0 || (uint32_t)_n * bpp() > 65535UL) { _err = ERR_BAD_COUNT; return false; }
#if defined(ARDUINO_ARCH_ESP8266)
  if (_pin > 15) { _err = ERR_BAD_PIN; return false; }       // GPIO16 у ESP8266 устроен иначе
#endif
  if (_pin == 255) { _err = ERR_BAD_PIN; return false; }
  _buf = (uint8_t *)calloc((size_t)_n * bpp(), 1);
  if (!_buf) { _err = ERR_NO_MEMORY; return false; }
#if !defined(ALASH_NP_RMT)
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
#endif
  _driverOk = driverBegin();
  if (!_driverOk && hasDriver()) { _err = ERR_DRIVER; free(_buf); _buf = nullptr; return false; }
  _begun = true;
  _err = ERR_OK;
  return true;
}

void AlashNeoPixel::end() {
  if (!_begun) return;
  driverEnd();
  free(_buf); free(_heat);
  _buf = nullptr; _heat = nullptr;
  _begun = false; _driverOk = false; _effect = EFFECT_NONE;
}

// ---- Пиксели -------------------------------------------------------------------------------------
void AlashNeoPixel::setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  if (i >= _n) { _err = ERR_RANGE; return; }
  uint8_t ri, gi, bi, wi; orderIndex(ri, gi, bi, wi);
  uint8_t *p = _buf + (uint32_t)i * bpp();
  p[ri] = r; p[gi] = g; p[bi] = b;
  if (bpp() == 4) p[wi] = w;
}

void AlashNeoPixel::setPixel(uint16_t i, uint32_t c) { setPixel(i, (uint8_t)(c >> 16), (uint8_t)(c >> 8), (uint8_t)c, (uint8_t)(c >> 24)); }

uint32_t AlashNeoPixel::getPixel(uint16_t i) const {
  if (!_begun || i >= _n) return 0;
  uint8_t ri, gi, bi, wi; orderIndex(ri, gi, bi, wi);
  const uint8_t *p = _buf + (uint32_t)i * bpp();
  return Color(p[ri], p[gi], p[bi], bpp() == 4 ? p[wi] : 0);
}

void AlashNeoPixel::fill(uint32_t color, uint16_t first, uint16_t howMany) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  if (first >= _n) return;
  uint16_t last = (howMany == 0 || (uint32_t)first + howMany > _n) ? _n : first + howMany;
  for (uint16_t i = first; i < last; i++) setPixel(i, color);
}

void AlashNeoPixel::gradient(uint32_t from, uint32_t to, uint16_t first, uint16_t howMany) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  if (first >= _n) return;
  uint16_t last = (howMany == 0 || (uint32_t)first + howMany > _n) ? _n : first + howMany;
  uint16_t cnt = last - first;
  for (uint16_t i = 0; i < cnt; i++) setPixel(first + i, Blend(from, to, cnt > 1 ? (uint32_t)i * 255 / (cnt - 1) : 0));
}

void AlashNeoPixel::rainbow(uint16_t startHue, uint16_t hueSpan) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  for (uint16_t i = 0; i < _n; i++) setPixel(i, ColorHSV(startHue + (uint32_t)i * hueSpan / _n));
}

void AlashNeoPixel::fade(uint8_t keep) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  uint32_t len = (uint32_t)_n * bpp();
  for (uint32_t i = 0; i < len; i++) _buf[i] = (uint8_t)(((uint16_t)_buf[i] * ((uint16_t)keep + 1)) >> 8);
}

void AlashNeoPixel::rotate(bool forward) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  uint8_t k = bpp(), tmp[4];
  if (_n < 2) return;
  if (forward) {
    memcpy(tmp, _buf + (uint32_t)(_n - 1) * k, k);
    memmove(_buf + k, _buf, (uint32_t)(_n - 1) * k);
    memcpy(_buf, tmp, k);
  } else {
    memcpy(tmp, _buf, k);
    memmove(_buf, _buf + k, (uint32_t)(_n - 1) * k);
    memcpy(_buf + (uint32_t)(_n - 1) * k, tmp, k);
  }
}

// ---- Ток и яркость ------------------------------------------------------------------------------
uint32_t AlashNeoPixel::activeMilliamps(uint16_t bright) const {
  if (!_begun) return 0;
  uint32_t sum = 0, len = (uint32_t)_n * bpp();
  for (uint32_t i = 0; i < len; i++) sum += _gamma ? gamma8(_buf[i]) : _buf[i];
  return (uint32_t)((uint64_t)sum * _chmA * bright / (255UL * 255UL));
}

uint16_t AlashNeoPixel::effectiveBright() const {
  uint16_t b = _bright;
  if (_maxmA > 0 && _begun) {
    uint32_t idle = _n;                           // около 1 мА на светодиод в покое
    uint32_t allowed = _maxmA > idle ? _maxmA - idle : 0;
    uint32_t want = activeMilliamps(b);
    if (want > allowed) b = want ? (uint16_t)((uint32_t)b * allowed / want) : 0;
  }
  return b;
}

void AlashNeoPixel::setMaxMilliamps(uint16_t mA) { _maxmA = mA; }

uint32_t AlashNeoPixel::estimatedMilliamps() const { return activeMilliamps(effectiveBright()) + (_begun ? _n : 0); }

uint32_t AlashNeoPixel::frameTimeUs() const {      // 1,25 мкс на бит
  return (uint32_t)_n * bpp() * 8UL * 5UL / 4UL;
}

bool AlashNeoPixel::canShow() const { return !_shown || (uint32_t)(micros() - _lastShowUs) >= 300UL; }

// ---- Вывод ---------------------------------------------------------------------------------------
void AlashNeoPixel::show() {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  if (!_writer && !_driverOk) { _err = ERR_NO_DRIVER; return; }
  while (!canShow()) delayMicroseconds(10);       // лента должна «защёлкнуть» прошлый кадр
  uint16_t scale = effectiveBright() + 1;         // 1..256
  uint16_t len = (uint16_t)((uint32_t)_n * bpp());
  if (_writer) {
    uint8_t *tmp = (uint8_t *)malloc(len);
    if (!tmp) { _err = ERR_NO_MEMORY; return; }
    for (uint16_t i = 0; i < len; i++) tmp[i] = xf(_buf[i], scale, _gamma);
    _writer(_pin, tmp, len);
    free(tmp);
  } else {
    driverSend(_buf, len, scale, _gamma);
  }
  _lastShowUs = micros(); _shown = true;
}

#if defined(ALASH_NP_AVR16)
// AVR 16 МГц: бит = 20 циклов. "0": высокий уровень 5 циклов (312 нс), "1": 11 циклов (688 нс), период 1,19..1,25 мкс (допуск WS2812: ±150 нс).
bool AlashNeoPixel::driverBegin() { return true; }
void AlashNeoPixel::driverEnd() {}
void AlashNeoPixel::driverSend(const uint8_t *data, uint16_t length, uint16_t scale, bool useGamma) {
  volatile uint8_t *port = portOutputRegister(digitalPinToPort(_pin));
  uint8_t mask = digitalPinToBitMask(_pin);
  uint8_t sreg = SREG;
  cli();
  uint8_t hi = *port | mask, lo = *port & ~mask;
  *port = lo;
  while (length--) {
    uint8_t b = xf(*data++, scale, useGamma), n = 8;
    asm volatile(
        "1:\n\t"
        "st   %a[port], %[hi]\n\t"       // фронт: высокий уровень
        "nop\n\tnop\n\t"
        "sbrs %[b], 7\n\t"               // бит 1: пропустить следующую команду
        "st   %a[port], %[lo]\n\t"       // бит 0: спад через 5 циклов
        "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
        "st   %a[port], %[lo]\n\t"       // бит 1: спад через 11 циклов
        "lsl  %[b]\n\t"
        "nop\n\tnop\n\t"
        "dec  %[n]\n\t"
        "brne 1b\n\t"
        : [b] "+r"(b), [n] "+r"(n)
        : [port] "e"(port), [hi] "r"(hi), [lo] "r"(lo));
  }
  SREG = sreg;
}
#elif defined(ALASH_NP_ESP8266)
bool AlashNeoPixel::driverBegin() { return true; }
void AlashNeoPixel::driverEnd() {}
static inline uint32_t IRAM_ATTR ccount() { uint32_t c; __asm__ __volatile__("rsr %0,ccount" : "=a"(c)); return c; }
void IRAM_ATTR AlashNeoPixel::driverSend(const uint8_t *data, uint16_t length, uint16_t scale, bool useGamma) {
  const uint32_t mask = 1UL << _pin;
  const uint32_t tpu = F_CPU / 1000000UL;
  const uint32_t t0h = tpu * 350 / 1000, t1h = tpu * 700 / 1000, period = tpu * 1250 / 1000;
  noInterrupts();
  uint32_t start = ccount();
  while (length--) {
    uint8_t b = xf(*data++, scale, useGamma);
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
      while ((ccount() - start) < period) {}  // дождаться конца предыдущего бита
      GPOS = mask;
      start = ccount();
      uint32_t high = (b & bit) ? t1h : t0h;
      while ((ccount() - start) < high) {}
      GPOC = mask;
    }
  }
  while ((ccount() - start) < period) {}
  interrupts();
}
#elif defined(ALASH_NP_RMT)
// ESP32 (ядро 3.x): RMT с тиком 100 нс. "0": 400 нс высокий и 900 нс низкий, "1": 800 и 500 нс.
bool AlashNeoPixel::driverBegin() {
  if (!rmtInit(_pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000)) return false;
  _rmt = malloc((size_t)_n * bpp() * 8 * sizeof(rmt_data_t));
  if (!_rmt) { rmtDeinit(_pin); return false; }
  return true;
}
void AlashNeoPixel::driverEnd() { if (_rmt) { rmtDeinit(_pin); free(_rmt); _rmt = nullptr; } }
void AlashNeoPixel::driverSend(const uint8_t *data, uint16_t length, uint16_t scale, bool useGamma) {
  rmt_data_t *sym = (rmt_data_t *)_rmt;
  size_t k = 0;
  for (uint16_t i = 0; i < length; i++) {
    uint8_t b = xf(data[i], scale, useGamma);
    for (uint8_t bit = 0x80; bit; bit >>= 1, k++) {
      sym[k].level0 = 1; sym[k].level1 = 0;
      if (b & bit) { sym[k].duration0 = 8; sym[k].duration1 = 5; }
      else         { sym[k].duration0 = 4; sym[k].duration1 = 9; }
    }
  }
  if (!rmtWrite(_pin, sym, k, RMT_WAIT_FOR_EVER)) _err = ERR_DRIVER;
}
#else
bool AlashNeoPixel::driverBegin() { return false; }
void AlashNeoPixel::driverEnd() {}
void AlashNeoPixel::driverSend(const uint8_t *, uint16_t, uint16_t, bool) {}
#endif

// ---- Эффекты -------------------------------------------------------------------------------------
uint8_t AlashNeoPixel::rnd() {                       // xorshift 16 бит
  _rnd ^= _rnd << 7; _rnd ^= _rnd >> 9; _rnd ^= _rnd << 8;
  return (uint8_t)(_rnd ^ (_rnd >> 8));
}

void AlashNeoPixel::animate(Effect e, uint16_t stepMs, uint32_t color) {
  if (!_begun) { _err = ERR_NOT_BEGUN; return; }
  if (e == EFFECT_FIRE && !_heat) {
    _heat = (uint8_t *)calloc(_n, 1);
    if (!_heat) { _err = ERR_NO_MEMORY; _effect = EFFECT_NONE; return; }
  }
  _effect = e; _stepMs = stepMs ? stepMs : 1; _color = color; _step = 0;
  _lastTick = millis() - _stepMs;                    // первый кадр сразу
}

bool AlashNeoPixel::update() {
  if (!_begun || _effect == EFFECT_NONE) return false;
  uint32_t now = millis();
  if (now - _lastTick < _stepMs) return false;
  _lastTick = now;
  render(_step++);
  show();
  return true;
}

void AlashNeoPixel::render(uint32_t s) {
  switch (_effect) {
    case EFFECT_RAINBOW: rainbow((uint16_t)((s * 4) % 360), 360); break;
    case EFFECT_CHASE: clear(); setPixel(s % _n, _color); break;
    case EFFECT_COMET: fade(190); setPixel(s % _n, _color); break;
    case EFFECT_BREATHE: {                           // треугольная волна, возведённая в квадрат: глаз видит равномерное дыхание
      uint16_t t = (s * 8) & 0x1FF;
      uint16_t level = t < 256 ? t : 511 - t;
      level = (level * level) >> 8;
      fill(Blend(0, _color, (uint8_t)level));
      break;
    }
    case EFFECT_BLINK: fill((s & 1) ? 0 : _color); break;
    case EFFECT_TWINKLE: fade(200); setPixel(rnd() % _n, _color); break;
    case EFFECT_FIRE: {                              // идея Fire2012 (М. Кригсман, MIT): остывание, подъём тепла, искры, цвет по температуре
      uint8_t cool = (uint8_t)(550UL / _n + 2);
      for (uint16_t i = 0; i < _n; i++) { uint8_t c = rnd() % cool; _heat[i] = _heat[i] > c ? _heat[i] - c : 0; }
      for (uint16_t k = _n - 1; k >= 2; k--) _heat[k] = ((uint16_t)_heat[k - 1] + _heat[k - 2] + _heat[k - 2]) / 3;
      if (rnd() < 120) { uint16_t y = rnd() % (_n < 7 ? _n : 7); uint16_t h = (uint16_t)_heat[y] + 160 + rnd() % 95; _heat[y] = h > 255 ? 255 : h; }
      for (uint16_t i = 0; i < _n; i++) {
        uint8_t h = _heat[i];
        uint16_t r = (uint16_t)h * 3, g = h > 85 ? (uint16_t)(h - 85) * 3 : 0, b = h > 170 ? (uint16_t)(h - 170) * 3 : 0;
        setPixel(i, r > 255 ? 255 : r, g > 255 ? 255 : g, b > 255 ? 255 : b);
      }
      break;
    }
    case EFFECT_WIPE: {
      uint32_t pos = s % ((uint32_t)_n * 2);
      if (pos < _n) setPixel(pos, _color); else setPixel(pos - _n, 0);
      break;
    }
    case EFFECT_THEATER: for (uint16_t i = 0; i < _n; i++) setPixel(i, ((i + s) % 3 == 0) ? _color : 0); break;
    case EFFECT_SCANNER: {
      uint32_t period = _n > 1 ? (uint32_t)_n * 2 - 2 : 1, pos = s % period;
      uint16_t p = pos < _n ? pos : period - pos;
      fade(100); setPixel(p, _color);
      break;
    }
    default: break;
  }
}

// ---- Диагностика -------------------------------------------------------------------------------
const __FlashStringHelper *AlashNeoPixel::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NOT_BEGUN:
      return T("Лента не запущена: вызовите begin() в setup().", "Лента іске қосылмаған: setup() ішінде begin() шақырыңыз.", "The strip is not started: call begin() in setup().");
    case ERR_NO_MEMORY:
      return T("Не хватило памяти под пиксели (3 байта на светодиод, 4 у RGBW). На Uno всего 2 КБ: около 500 светодиодов максимум, а эффект FIRE требует ещё байт на светодиод.", "Пикселдерге жад жетпеді (светодиодқа 3 байт, RGBW-де 4). Uno-да барлығы 2 КБ: ең көбі шамамен 500 светодиод, FIRE эффектісіне тағы байт керек.", "Not enough memory for the pixels (3 bytes per LED, 4 for RGBW). Uno has only 2 KB: about 500 LEDs at most, and the FIRE effect needs one more byte per LED.");
    case ERR_BAD_COUNT:
      return T("Неверное число светодиодов: должно быть от 1, а байтов в ленте не больше 65535.", "Светодиод саны қате: 1-ден бастап, лентадағы байт саны 65535-тен аспауы керек.", "Wrong LED count: it must be at least 1 and the strip must not exceed 65535 bytes.");
    case ERR_BAD_PIN:
      return T("Неверный пин. Укажите пин, к которому подключён вход DIN ленты (у ESP8266 это GPIO 0..15, GPIO16 не подходит).", "Пин қате. Лента DIN кірісі қосылған пинді көрсетіңіз (ESP8266-да GPIO 0..15, GPIO16 жарамайды).", "Wrong pin. Give the pin wired to the DIN input of the strip (on ESP8266 GPIO 0..15, GPIO16 does not work).");
    case ERR_NO_DRIVER:
      return T("Для этой платы нет встроенного драйвера. Поддерживаются AVR на 16 МГц (Uno, Nano, Mega), ESP8266 и ESP32 с ядром 3.x. Для ESP32 ядра 2.x обновите ядро; для остальных плат (RP2040, STM32) задайте свой вывод через setWriter().", "Бұл тақтаға кіріктірілген драйвер жоқ. 16 МГц AVR (Uno, Nano, Mega), ESP8266 және 3.x ядролы ESP32 қолдау көрсетіледі. ESP32 2.x үшін ядроны жаңартыңыз; басқалары үшін setWriter().", "This board has no built-in driver. Supported: AVR at 16 MHz (Uno, Nano, Mega), ESP8266 and ESP32 with core 3.x. For ESP32 core 2.x update the core; for other boards (RP2040, STM32) provide your own output with setWriter().");
    case ERR_DRIVER:
      return T("Не удалось запустить вывод сигнала. На ESP32 канал RMT занят другой библиотекой (например, ИК-приёмником) или не хватило памяти.", "Сигнал шығаруды іске қосу мүмкін болмады. ESP32-де RMT арнасын басқа кітапхана (мысалы, ИК қабылдағыш) алған немесе жад жетпейді.", "Could not start the signal output. On ESP32 the RMT channel is used by another library (for example an IR receiver) or memory ran out.");
    case ERR_RANGE:
      return T("Номер пикселя за пределами ленты, команда пропущена. Нумерация с нуля до count()-1.", "Пиксель нөмірі лентадан тыс, команда өткізілді. Нөмірлеу нөлден count()-1 дейін.", "The pixel index is outside the strip, the command was skipped. Numbering is 0 to count()-1.");
  }
  return F("");
}

void AlashNeoPixel::diagnose(Print &out) {
  out.println(T("AlashNeoPixel: диагностика", "AlashNeoPixel: диагностика", "AlashNeoPixel: diagnostics"));
  out.print(T("Светодиодов: ", "Светодиодтар: ", "LEDs: ")); out.print((long)_n);
  out.print(T("   пин: ", "   пин: ", "   pin: ")); out.print((long)_pin);
  static const char *const names[8] = {"GRB", "RGB", "BRG", "RBG", "GBR", "BGR", "GRBW", "RGBW"};
  out.print(T("   порядок: ", "   реті: ", "   order: ")); out.println(names[_order]);
  out.print(T("Память под пиксели, байт: ", "Пикселдерге жад, байт: ", "Pixel memory, bytes: ")); out.print((long)((uint32_t)_n * bpp()));
  out.print(T("   кадр: ", "   кадр: ", "   frame: ")); out.print((long)(frameTimeUs() + 300)); out.println(T(" мкс", " мкс", " us"));
#if defined(ALASH_NP_RMT)
  out.println(F("Driver: ESP32 RMT"));
#elif defined(ALASH_NP_ESP8266)
  out.println(F("Driver: ESP8266 cycle counter"));
#elif defined(ALASH_NP_AVR16)
  out.println(F("Driver: AVR 16 MHz"));
#else
  out.println(_writer ? F("Driver: setWriter()") : F("Driver: none"));
#endif
  out.print(T("Яркость: ", "Жарықтық: ", "Brightness: ")); out.print((long)_bright);
  out.print(T("   гамма: ", "   гамма: ", "   gamma: ")); out.print(_gamma ? T("да", "иә", "yes") : T("нет", "жоқ", "no"));
  out.print(T("   ограничение тока, мА: ", "   ток шектеуі, мА: ", "   current limit, mA: ")); out.println((long)_maxmA);
  if (!_begun) { out.println(T("Подсказка: вызовите begin() в setup().", "Кеңес: setup() ішінде begin() шақырыңыз.", "Tip: call begin() in setup().")); return; }
  if (!_writer && !_driverOk) { out.println(errorText()); return; }
  out.println(T("Сейчас загорятся красный, зелёный, синий и белый, потом побежит точка. Если цвета перепутаны, смените порядок: ORDER_RGB вместо ORDER_GRB. Если светится не вся лента или мигает:\n  1) питание 5 В от отдельного блока: каждый светодиод до 60 мА, 30 штук это уже 1,8 А; общий GND с платой;\n  2) резистор 300..470 Ом в линию данных и конденсатор 1000 мкФ у питания ленты;\n  3) ESP32 и ESP8266 дают 3,3 В на данных: для ленты на 5 В нужен преобразователь уровня (74AHCT125) или короткий провод;\n  4) сигнал подаётся на вход DIN, а не DOUT.",
                "Қазір қызыл, жасыл, көк және ақ жанады, сосын нүкте жүгіреді. Түстер шатасса, ретін өзгертіңіз: ORDER_GRB орнына ORDER_RGB. Лента түгел жанбаса немесе жыпылықтаса:\n  1) қуат 5 В бөлек блоктан: әр светодиод 60 мА-ге дейін; GND ортақ;\n  2) деректер желісіне 300..470 Ом резистор және қуатқа 1000 мкФ конденсатор;\n  3) ESP32 мен ESP8266 деректерге 3,3 В береді: 5 В лентаға деңгей түрлендіргіш (74AHCT125) керек;\n  4) сигнал DIN кірісіне беріледі, DOUT-қа емес.",
                "Red, green, blue and white will light now, then a dot will run. If the colors are swapped, change the order: ORDER_RGB instead of ORDER_GRB. If the strip does not light fully or flickers:\n  1) power it with 5 V from a separate supply: each LED up to 60 mA, 30 of them is already 1.8 A; common GND with the board;\n  2) a 300..470 ohm resistor in the data line and a 1000 uF capacitor across the strip power;\n  3) ESP32 and ESP8266 output 3.3 V on data: a 5 V strip needs a level shifter (74AHCT125) or a very short wire;\n  4) the signal goes to DIN, not DOUT."));
  uint8_t saved = _bright;
  if (_bright > 64) _bright = 64;                    // тест на четверти яркости, чтобы не перегрузить блок питания
  static const uint32_t seq[4] = {RED, GREEN, BLUE, WHITE};
  for (uint8_t i = 0; i < 4; i++) { fill(seq[i]); show(); delay(600); }
  uint16_t run = _n < 60 ? _n : 60;
  for (uint16_t i = 0; i < run; i++) { clear(); setPixel(i, WHITE); show(); delay(30); }
  clear(); show();
  _bright = saved;
}
