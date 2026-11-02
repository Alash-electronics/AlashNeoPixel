// Тесты AlashNeoPixel без железа: вместо драйвера ленты используется setWriter(), он записывает отправленные байты.
//   g++ -std=c++17 -Wall -fsanitize=address,undefined -I mock -I ../../src test_neopixel.cpp ../../src/AlashNeoPixel.cpp && ./a.out
#include <stdio.h>
#include <vector>
#include "Arduino.h"
#include "AlashNeoPixel.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
typedef AlashNeoPixel NP;

static std::vector<std::vector<uint8_t>> frames;
static uint8_t lastPin = 0;
static void writer(uint8_t pin, const uint8_t *d, uint16_t len) { lastPin = pin; frames.emplace_back(d, d + len); }
static void fresh() { frames.clear(); g_ms = 1000; g_us = 5000000; }
static const std::vector<uint8_t> &last() { return frames.back(); }
static int lit(const NP &s) { int n = 0; for (uint16_t i = 0; i < s.count(); i++) if (s.getPixel(i)) n++; return n; }
static int firstLit(const NP &s) { for (uint16_t i = 0; i < s.count(); i++) if (s.getPixel(i)) return i; return -1; }
static void tick(NP &s, uint16_t ms, int times = 1) { for (int i = 0; i < times; i++) { g_ms += ms; s.update(); } }

int main() {
  // 1) begin: проверка параметров, нет драйвера на «хосте» без writer
  {
    fresh();
    NP z(0, 6); CHECK(!z.begin() && z.error() == NP::ERR_BAD_COUNT);
    NP big(22000, 6); CHECK(!big.begin() && big.error() == NP::ERR_BAD_COUNT);       // 66000 байт не помещается в длину кадра
    NP p(8, 255); CHECK(!p.begin() && p.error() == NP::ERR_BAD_PIN);
    NP s(8, 6);
    s.setPixel(0, 1, 2, 3); CHECK(s.error() == NP::ERR_NOT_BEGUN);       // до begin()
    s.clearError(); s.show(); CHECK(s.error() == NP::ERR_NOT_BEGUN);
    CHECK(s.begin() && s.begin());                                       // повторный begin безопасен
    CHECK(!NP::hasDriver());
    s.clearError(); s.show(); CHECK(s.error() == NP::ERR_NO_DRIVER && frames.empty());
    s.setWriter(writer); s.clearError(); s.show();
    CHECK(s.error() == NP::ERR_OK && frames.size() == 1 && last().size() == 24 && lastPin == 6);
  }

  // 2) цвет: упаковка, HSV, колесо, смесь
  {
    CHECK(NP::Color(1, 2, 3) == 0x010203 && NP::Color(1, 2, 3, 4) == 0x04010203u);
    CHECK(NP::ColorHSV(0) == 0xFF0000 && NP::ColorHSV(120) == 0x00FF00 && NP::ColorHSV(240) == 0x0000FF);
    CHECK(NP::ColorHSV(60) == 0xFFFF00 && NP::ColorHSV(180) == 0x00FFFF && NP::ColorHSV(300) == 0xFF00FF);
    CHECK(NP::ColorHSV(360) == NP::ColorHSV(0) && NP::ColorHSV(720 + 120) == 0x00FF00);   // оттенок по кругу
    CHECK(NP::ColorHSV(77, 0, 200) == 0xC8C8C8);                                          // без насыщенности серый
    CHECK(NP::ColorHSV(77, 255, 0) == 0);
    uint32_t c = NP::ColorHSV(30);                                                        // оранжевый: R max, G около половины
    CHECK(((c >> 16) & 255) == 255 && ((c >> 8) & 255) >= 120 && ((c >> 8) & 255) <= 135 && (c & 255) == 0);
    CHECK(NP::Wheel(0) == 0xFF0000 && NP::Wheel(85) == NP::ColorHSV(85 * 360 / 256));
    CHECK(NP::Blend(0x000000, 0xFFFFFF, 0) == 0 && NP::Blend(0x000000, 0xFFFFFF, 255) == 0xFFFFFF);
    uint32_t m = NP::Blend(0x000000, 0xFF8000, 128);
    CHECK(((m >> 16) & 255) == 128 && ((m >> 8) & 255) == 64);
    CHECK(NP::Blend(0xFF0000, 0x0000FF, 255) == 0x0000FF && NP::Blend(0xFF0000, 0x0000FF, 0) == 0xFF0000);
  }

  // 3) порядок байтов и RGBW
  {
    struct { NP::Order o; uint8_t b0, b1, b2; } cases[] = {
      {NP::ORDER_GRB, 20, 10, 30}, {NP::ORDER_RGB, 10, 20, 30}, {NP::ORDER_BRG, 30, 10, 20},
      {NP::ORDER_RBG, 10, 30, 20}, {NP::ORDER_GBR, 20, 30, 10}, {NP::ORDER_BGR, 30, 20, 10}};
    for (auto &c : cases) {
      fresh(); NP s(2, 3, c.o); s.begin(); s.setWriter(writer);
      s.setPixel(1, 10, 20, 30); s.show();
      CHECK(last().size() == 6 && last()[3] == c.b0 && last()[4] == c.b1 && last()[5] == c.b2 && last()[0] == 0);
      CHECK(s.getPixel(1) == NP::Color(10, 20, 30));
    }
    fresh(); NP w(2, 3, NP::ORDER_GRBW); w.begin(); w.setWriter(writer);
    w.setPixel(0, NP::Color(10, 20, 30, 40)); w.show();
    CHECK(last().size() == 8 && last()[0] == 20 && last()[1] == 10 && last()[2] == 30 && last()[3] == 40);
    CHECK(w.getPixel(0) == NP::Color(10, 20, 30, 40));
    NP rw(1, 3, NP::ORDER_RGBW); rw.begin(); rw.setWriter(writer); rw.setPixel(0, 1, 2, 3, 4); rw.show();
    CHECK(last()[0] == 1 && last()[1] == 2 && last()[2] == 3 && last()[3] == 4);
    NP rgb(1, 3); rgb.begin(); rgb.setPixel(0, NP::Color(1, 2, 3, 99)); CHECK(rgb.getPixel(0) == NP::Color(1, 2, 3));   // белый в RGB-ленте игнорируется
  }

  // 4) fill, диапазоны, выход за пределы не портит соседей
  {
    fresh(); NP s(10, 3); s.begin();
    s.fill(0x112233); CHECK(lit(s) == 10);
    s.clear(); CHECK(lit(s) == 0);
    s.fill(NP::RED, 2, 3); CHECK(lit(s) == 3 && s.getPixel(2) == NP::RED && s.getPixel(4) == NP::RED && s.getPixel(5) == 0 && s.getPixel(1) == 0);
    s.clear(); s.fill(NP::BLUE, 8, 100); CHECK(lit(s) == 2);                // хвост обрезается
    s.clear(); s.fill(NP::BLUE, 20, 5); CHECK(lit(s) == 0);                 // старт за лентой: ничего
    s.clearError(); s.setPixel(10, NP::GREEN); CHECK(s.error() == NP::ERR_RANGE && lit(s) == 0);
    CHECK(s.getPixel(500) == 0);
  }

  // 5) градиент и радуга
  {
    fresh(); NP s(5, 3); s.begin();
    s.gradient(0x000000, 0xFF0000);
    CHECK(s.getPixel(0) == 0 && s.getPixel(4) == 0xFF0000 && ((s.getPixel(2) >> 16) & 255) == 127);
    s.clear(); s.gradient(NP::RED, NP::BLUE, 1, 3); CHECK(s.getPixel(0) == 0 && s.getPixel(1) == NP::RED && s.getPixel(3) == NP::BLUE && s.getPixel(4) == 0);
    NP one(1, 3); one.begin(); one.gradient(NP::RED, NP::BLUE); CHECK(one.getPixel(0) == NP::RED);
    NP r(6, 3); r.begin(); r.rainbow();
    CHECK(r.getPixel(0) == 0xFF0000 && r.getPixel(2) == 0x00FF00 && r.getPixel(4) == 0x0000FF);
    r.rainbow(120); CHECK(r.getPixel(0) == 0x00FF00);
    NP half(4, 3); half.begin(); half.rainbow(0, 180); CHECK(half.getPixel(0) == 0xFF0000 && half.getPixel(3) != half.getPixel(0));
  }

  // 6) затухание и сдвиг
  {
    fresh(); NP s(4, 3); s.begin(); s.fill(NP::Color(200, 100, 50));
    s.fade(255); CHECK(s.getPixel(0) == NP::Color(200, 100, 50));          // 255 = без изменений
    s.fade(127); CHECK(s.getPixel(0) == NP::Color(100, 50, 25));
    s.fade(0); CHECK(lit(s) == 0 || s.getPixel(0) == 0);
    NP r(4, 3); r.begin(); r.setPixel(0, NP::RED); r.setPixel(1, NP::GREEN); r.setPixel(2, NP::BLUE);
    r.rotate(true); CHECK(r.getPixel(1) == NP::RED && r.getPixel(2) == NP::GREEN && r.getPixel(3) == NP::BLUE && r.getPixel(0) == 0);
    r.rotate(true); CHECK(r.getPixel(0) == NP::BLUE && r.getPixel(1) == 0 && r.getPixel(3) == NP::GREEN);
    r.rotate(false); r.rotate(false); CHECK(r.getPixel(0) == NP::RED && r.getPixel(1) == NP::GREEN && r.getPixel(2) == NP::BLUE);
    NP w(3, 3, NP::ORDER_GRBW); w.begin(); w.setPixel(0, NP::Color(1, 2, 3, 4)); w.rotate(true);
    CHECK(w.getPixel(1) == NP::Color(1, 2, 3, 4) && w.getPixel(0) == 0);
  }

  // 7) яркость: данные пикселей не портятся, на ленту уходят масштабированные
  {
    fresh(); NP s(2, 3, NP::ORDER_RGB); s.begin(); s.setWriter(writer); s.fill(NP::Color(255, 128, 10));
    s.show(); CHECK(last()[0] == 255 && last()[1] == 128 && last()[2] == 10);   // 255 = точно как есть
    s.setBrightness(128); s.show();
    CHECK(last()[0] == 128 && last()[1] == 64 && last()[2] == 5);               // (v * 129) >> 8
    CHECK(s.getPixel(0) == NP::Color(255, 128, 10) && s.brightness() == 128);
    s.setBrightness(0); s.show(); CHECK(last()[0] == 0 && last()[1] == 0 && last()[2] == 0);
    s.setBrightness(1); s.show(); CHECK(last()[0] == 1 && last()[1] == 1 && last()[2] == 0);   // (v * 2) >> 8
  }

  // 8) гамма
  {
    CHECK(NP::gamma8(0) == 0 && NP::gamma8(255) == 255 && NP::gamma8(128) == 56);
    bool mono = true; for (int i = 1; i < 256; i++) if (NP::gamma8(i) < NP::gamma8(i - 1)) mono = false;
    CHECK(mono);
    fresh(); NP s(1, 3, NP::ORDER_RGB); s.begin(); s.setWriter(writer); s.setPixel(0, 128, 255, 10);
    s.setGamma(true); s.show(); CHECK(last()[0] == 56 && last()[1] == 255 && last()[2] == NP::gamma8(10));
    s.setBrightness(128); s.show(); CHECK(last()[0] == (uint8_t)((56 * 129) >> 8));   // сначала гамма, потом яркость
  }

  // 9) ток: оценка и ограничение
  {
    fresh(); NP s(10, 3); s.begin(); s.setWriter(writer);
    CHECK(s.estimatedMilliamps() == 10);                                    // чёрная лента: 1 мА на светодиод в покое
    s.fill(NP::WHITE);
    CHECK(s.estimatedMilliamps() == 610);                                   // 10 * 3 * 20 + 10
    s.setBrightness(128); CHECK(s.estimatedMilliamps() >= 300 && s.estimatedMilliamps() <= 312);
    s.setBrightness(255); s.setMaxMilliamps(310);
    CHECK(s.estimatedMilliamps() <= 310 && s.estimatedMilliamps() >= 295);
    s.show(); CHECK(last()[0] < 140 && last()[0] > 120);                    // примерно половина
    s.fill(NP::Color(0, 20, 0)); s.show(); CHECK(last()[0] == 20);     // зелёный идёт первым байтом (GRB)          // малая нагрузка: ограничение не вмешивается
    s.fill(NP::WHITE); s.setMaxMilliamps(5); s.show(); CHECK(last()[0] == 0);   // меньше тока покоя: яркость в ноль, без сбоев
    s.setMaxMilliamps(0); s.show(); CHECK(last()[0] == 255);
    s.setChannelMilliamps(10); CHECK(s.estimatedMilliamps() == 310);
  }

  // 10) пауза защёлки между кадрами
  {
    fresh(); NP s(4, 3); s.begin(); s.setWriter(writer);
    CHECK(s.canShow()); s.show(); CHECK(!s.canShow());
    uint32_t t0 = g_us; s.show(); CHECK(g_us - t0 >= 280 && frames.size() == 2);   // подождал защёлку
    g_us += 1000; CHECK(s.canShow());
    CHECK(s.frameTimeUs() == 4 * 24 * 5 / 4);
    NP t(30, 3); CHECK(t.frameTimeUs() == 900);
  }

  // 11) эффекты
  {
    fresh(); NP s(10, 3); s.begin(); s.setWriter(writer);
    s.stopEffect(); CHECK(!s.update());
    s.animate(NP::EFFECT_CHASE, 50, NP::RED);
    CHECK(s.update() && lit(s) == 1 && firstLit(s) == 0);                   // первый кадр сразу
    CHECK(!s.update());                                                     // пока не прошло 50 мс: нет
    for (int i = 1; i < 25; i++) { tick(s, 50); CHECK(lit(s) == 1 && firstLit(s) == i % 10); }

    s.animate(NP::EFFECT_COMET, 20, NP::BLUE); s.clear();
    tick(s, 20, 5); CHECK(s.getPixel(4) == NP::BLUE && lit(s) == 5);       // голова и хвост
    CHECK((s.getPixel(3) & 255) < 255 && (s.getPixel(3) & 255) > (s.getPixel(2) & 255));   // хвост затухает

    s.animate(NP::EFFECT_BLINK, 100, NP::GREEN);
    s.update(); bool a = lit(s) == 10; tick(s, 100); bool b = lit(s) == 0; tick(s, 100); bool c = lit(s) == 10;
    CHECK(a && b && c);                                                     // первый шаг (0) цвет, второй пусто

    s.animate(NP::EFFECT_THEATER, 100, NP::WHITE);
    s.update(); CHECK(lit(s) == 4 && s.getPixel(0) != 0 && s.getPixel(3) != 0 && s.getPixel(1) == 0);
    tick(s, 100); CHECK(s.getPixel(0) == 0 && s.getPixel(2) != 0);

    s.animate(NP::EFFECT_WIPE, 30, NP::RED);
    s.clear(); s.update(); tick(s, 30, 9); CHECK(lit(s) == 10);             // заполнили
    tick(s, 30, 5); CHECK(lit(s) == 5);                                     // стираем с начала
    tick(s, 30, 5); CHECK(lit(s) == 0);

    s.animate(NP::EFFECT_SCANNER, 30, NP::RED); s.clear();
    int maxp = -1, minp = 99; for (int i = 0; i < 40; i++) { tick(s, 30); int h = -1; uint32_t br = 0; for (int k = 0; k < 10; k++) if (s.getPixel(k) > br) { br = s.getPixel(k); h = k; } if (h > maxp) maxp = h; if (h < minp) minp = h; }
    CHECK(minp == 0 && maxp == 9);                                          // ходит от края до края и обратно

    s.animate(NP::EFFECT_RAINBOW, 20); s.update(); uint32_t p0 = s.getPixel(0); tick(s, 20, 3); CHECK(s.getPixel(0) != p0);
    s.animate(NP::EFFECT_BREATHE, 20, NP::Color(0, 200, 0)); s.update(); CHECK(lit(s) == 0);   // начинает с темноты
    uint32_t mx = 0; for (int i = 0; i < 70; i++) { tick(s, 20); if (s.getPixel(0) > mx) mx = s.getPixel(0); } CHECK(((mx >> 8) & 255) >= 195);
    s.animate(NP::EFFECT_TWINKLE, 20, NP::WHITE); s.setSeed(1234); tick(s, 20, 60); CHECK(lit(s) >= 3 && lit(s) <= 10);
    s.animate(NP::EFFECT_FIRE, 20); tick(s, 20, 80);
    bool warm = true; int bright = 0; for (int k = 0; k < 10; k++) { uint32_t c2 = s.getPixel(k); int r = (c2 >> 16) & 255, g = (c2 >> 8) & 255, bl = c2 & 255; if (!(r >= g && g >= bl)) warm = false; if (r > 100) bright++; }
    CHECK(warm && bright >= 1);                                             // оттенки огня: красный не меньше зелёного, зелёный не меньше синего
    s.stopEffect(); size_t n = frames.size(); tick(s, 100, 5); CHECK(frames.size() == n);
  }

  // 12) эффекты: до begin, переполнение millis(), малые ленты
  {
    fresh(); NP s(5, 3); s.animate(NP::EFFECT_CHASE); CHECK(s.error() == NP::ERR_NOT_BEGUN);
    s.begin(); s.setWriter(writer); s.clearError();
    g_ms = 4294967200u; s.animate(NP::EFFECT_CHASE, 50, NP::RED);
    int shown = 0; for (int i = 0; i < 40; i++) { g_ms += 10; if (s.update()) shown++; }
    CHECK(shown >= 7 && shown <= 9);                                        // 400 мс через переполнение: каждые 50 мс
    for (int e = NP::EFFECT_RAINBOW; e <= NP::EFFECT_SCANNER; e++) {        // крошечные ленты не ломаются
      for (uint16_t n = 1; n <= 3; n++) { NP t(n, 3); t.begin(); t.setWriter(writer); t.animate((NP::Effect)e, 10, NP::WHITE); tick(t, 10, 50); CHECK(t.error() == NP::ERR_OK); }
    }
  }

  // 13) end() и повторный begin
  {
    fresh(); NP s(4, 3); s.begin(); s.setWriter(writer); s.animate(NP::EFFECT_FIRE, 10); tick(s, 10, 3);
    s.end(); CHECK(s.effect() == NP::EFFECT_NONE && !s.update());
    s.clearError(); s.setPixel(0, NP::RED); CHECK(s.error() == NP::ERR_NOT_BEGUN);
    CHECK(s.begin()); s.setPixel(0, NP::RED); CHECK(s.getPixel(0) == NP::RED);
  }

  // 14) тексты ошибок и диагностика
  {
    fresh();
    // по одной ситуации на каждую ошибку
    NP e1(0, 3); e1.begin(); CHECK(e1.error() == NP::ERR_BAD_COUNT);
    NP e2(4, 255); e2.begin(); CHECK(e2.error() == NP::ERR_BAD_PIN);
    NP e3(4, 3); e3.setPixel(0, 1); CHECK(e3.error() == NP::ERR_NOT_BEGUN);
    NP e4(4, 3); e4.begin(); e4.show(); CHECK(e4.error() == NP::ERR_NO_DRIVER);
    NP e5(4, 3); e5.begin(); e5.setPixel(99, 1); CHECK(e5.error() == NP::ERR_RANGE);
    NP *list[] = {&e1, &e2, &e3, &e4, &e5};
    for (NP *p : list) {
      p->setLanguage(NP::LANG_RU); const char *ru = (const char *)p->errorText();
      p->setLanguage(NP::LANG_KK); const char *kk = (const char *)p->errorText();
      p->setLanguage(NP::LANG_EN); const char *en = (const char *)p->errorText();
      CHECK(strlen(ru) > 20 && strlen(kk) > 20 && strlen(en) > 20 && strcmp(ru, kk) && strcmp(kk, en) && strcmp(ru, en));
    }
    e1.clearError(); CHECK(strcmp((const char *)e1.errorText(), "OK") == 0);

    fresh(); NP d(8, 5, NP::ORDER_RGB); d.begin(); d.setWriter(writer); d.setBrightness(200); d.setLanguage(NP::LANG_EN);
    Print p; d.diagnose(p);
    CHECK(p.text.find("LEDs: 8") != std::string::npos && p.text.find("RGB") != std::string::npos && p.text.find("level shifter") != std::string::npos);
    CHECK(frames.size() == 4 + 8 + 1);                                      // красный, зелёный, синий, белый, бегущая точка, погасить
    CHECK(d.brightness() == 200);                                           // яркость возвращена
    CHECK(frames[0][0] == (uint8_t)((255 * 65) >> 8));                      // тест шёл на яркости 64
    d.setLanguage(NP::LANG_KK); Print pk; d.diagnose(pk); CHECK(pk.text.find("Светодиодтар: 8") != std::string::npos);
    NP nb(4, 3); Print pn; nb.diagnose(pn); CHECK(pn.text.find("begin()") != std::string::npos);
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
