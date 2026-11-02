// AlashNeoPixel: адресные светодиоды WS2812 / WS2812B / SK6812 (NeoPixel): цвета, HSV, радуга, градиент, гамма,
// ограничение тока и готовые эффекты без delay(). Свой вывод сигнала: AVR 16 МГц, ESP8266 и ESP32 (RMT, ядро 3.x), без зависимостей.
// Протокол WS2812 написан по даташиту, код не заимствован. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashNeoPixel
#ifndef ALASH_NEOPIXEL_H
#define ALASH_NEOPIXEL_H

#include <Arduino.h>

class AlashNeoPixel {
 public:
  // Порядок цветов в ленте: у большинства WS2812 это GRB. Если красный и зелёный перепутаны, возьмите ORDER_RGB. SK6812 RGBW имеет белый канал.
  enum Order : uint8_t { ORDER_GRB = 0, ORDER_RGB, ORDER_BRG, ORDER_RBG, ORDER_GBR, ORDER_BGR, ORDER_GRBW, ORDER_RGBW };
  enum Effect : uint8_t { EFFECT_NONE = 0, EFFECT_RAINBOW, EFFECT_CHASE, EFFECT_COMET, EFFECT_BREATHE, EFFECT_BLINK, EFFECT_TWINKLE, EFFECT_FIRE, EFFECT_WIPE, EFFECT_THEATER, EFFECT_SCANNER };
  enum Error : uint8_t { ERR_OK = 0, ERR_NOT_BEGUN, ERR_NO_MEMORY, ERR_BAD_COUNT, ERR_BAD_PIN, ERR_NO_DRIVER, ERR_DRIVER, ERR_RANGE };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };
  // Готовые цвета (0xRRGGBB)
  enum : uint32_t { BLACK = 0x000000, WHITE = 0xFFFFFF, RED = 0xFF0000, GREEN = 0x00FF00, BLUE = 0x0000FF, YELLOW = 0xFFFF00,
                    CYAN = 0x00FFFF, MAGENTA = 0xFF00FF, ORANGE = 0xFF5A00, PURPLE = 0x8000FF, ROSE = 0xFF3C78, WARM_WHITE = 0xFFB464 };

  // Функция вывода для плат без встроенного драйвера (RP2040, STM32 и др.): получает готовые байты ленты
  typedef void (*Writer)(uint8_t pin, const uint8_t *data, uint16_t length);

  AlashNeoPixel(uint16_t count, uint8_t pin, Order order = ORDER_GRB) : _n(count), _pin(pin), _order(order) {}
  ~AlashNeoPixel() { end(); }

  bool begin();                                   // выделить память и настроить пин
  void end();
  uint16_t count() const { return _n; }

  // ---- Цвет ----------------------------------------------------------------------------------
  // Цвет одним числом 0xWWRRGGBB (белый канал только для RGBW); яркость и гамма применяются при show()
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
  static uint32_t ColorHSV(uint16_t hueDegrees, uint8_t saturation = 255, uint8_t value = 255);   // оттенок 0..359
  static uint32_t Wheel(uint8_t position);        // цветовой круг 0..255
  static uint32_t Blend(uint32_t from, uint32_t to, uint8_t amount);   // смесь: 0 = from, 255 = to
  static uint8_t gamma8(uint8_t v);

  void setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
  void setPixel(uint16_t i, uint32_t color);
  void setPixelHSV(uint16_t i, uint16_t hueDegrees, uint8_t saturation = 255, uint8_t value = 255) { setPixel(i, ColorHSV(hueDegrees, saturation, value)); }
  uint32_t getPixel(uint16_t i) const;
  void fill(uint32_t color, uint16_t first = 0, uint16_t howMany = 0);   // howMany = 0: до конца
  void clear() { fill(0); }
  void gradient(uint32_t from, uint32_t to, uint16_t first = 0, uint16_t howMany = 0);
  void rainbow(uint16_t startHue = 0, uint16_t hueSpan = 360);           // радуга на всю ленту (hueSpan 360 = полный круг)
  void fade(uint8_t keep);                        // умножить все пиксели на keep/256 (затухание)
  void rotate(bool forward = true);               // сдвинуть картинку на один пиксель по кругу

  // ---- Вывод -------------------------------------------------------------------------------
  void show();                                    // отправить на ленту (прерывания на время отправки выключаются)
  bool canShow() const;                           // прошла ли пауза после прошлой отправки (около 300 мкс)
  void setBrightness(uint8_t b) { _bright = b; }  // общая яркость 0..255, данные пикселей не портятся
  uint8_t brightness() const { return _bright; }
  void setGamma(bool on) { _gamma = on; }         // гамма-коррекция: плавные оттенки при малой яркости
  void setMaxMilliamps(uint16_t mA);              // защита блока питания: яркость снижается так, чтобы ток не превысил mA (0 = без ограничения)
  void setChannelMilliamps(uint8_t mA) { _chmA = mA; }   // ток одного канала при полной яркости (по умолчанию 20)
  uint32_t estimatedMilliamps() const;            // оценка тока при текущем содержимом и яркости
  void setWriter(Writer w) { _writer = w; }       // свой вывод (если задан, встроенный драйвер не используется)
  static bool hasDriver();                        // есть ли встроенный драйвер для этой платы
  uint32_t frameTimeUs() const;                   // сколько занимает отправка кадра

  // ---- Эффекты без delay() ----------------------------------------------------------------------
  void animate(Effect effect, uint16_t stepMs = 40, uint32_t color = WHITE);   // запустить эффект: один шаг раз в stepMs
  void stopEffect() { _effect = EFFECT_NONE; }
  Effect effect() const { return _effect; }
  bool update();                                  // вызывайте в loop(): рисует следующий кадр, когда пора; true, если кадр показан
  void setSeed(uint16_t seed) { _rnd = seed ? seed : 1; }

  // ---- Диагностика -------------------------------------------------------------------
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);                      // печатает настройки и зажигает красный, зелёный, синий, белый (блокирующий тест)

 private:
  uint8_t bpp() const { return _order >= ORDER_GRBW ? 4 : 3; }
  void orderIndex(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &w) const;
  uint8_t scaleFor() const;
  uint8_t rnd();
  void render(uint32_t step);
  bool driverBegin();
  void driverEnd();
  void driverSend(const uint8_t *data, uint16_t length, uint16_t scale, bool useGamma);
  uint32_t activeMilliamps(uint16_t bright) const;
  uint16_t effectiveBright() const;

  uint16_t _n; uint8_t _pin; Order _order;
  uint8_t *_buf = nullptr, *_heat = nullptr;
  uint8_t _bright = 255, _chmA = 20;
  uint16_t _maxmA = 0, _rnd = 0xACE1, _stepMs = 40;
  bool _begun = false, _gamma = false, _driverOk = false;
  Effect _effect = EFFECT_NONE; uint32_t _color = WHITE, _step = 0, _lastTick = 0, _lastShowUs = 0;
  bool _shown = false;
  Writer _writer = nullptr;
  Error _err = ERR_OK; Language _lang = LANG_RU;
#if defined(ARDUINO_ARCH_ESP32) && defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  void *_rmt = nullptr;                           // буфер символов RMT
#endif
};

#endif
