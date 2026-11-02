# AlashNeoPixel

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Адресная лента **WS2812 / NeoPixel** в три строки: `strip.fill(RED); strip.show();`. Цвета по HSV, радуга, градиент, гамма, **защита блока питания по току** и **10 готовых эффектов** (радуга, огонь, комета, дыхание и другие) без `delay()`. Свой вывод сигнала для Arduino Uno, ESP8266 и ESP32. Без зависимостей.

```cpp
#include <AlashNeoPixel.h>
AlashNeoPixel strip(30, 6);           // 30 светодиодов, DIN на пин 6

void setup() {
  strip.begin();
  strip.setBrightness(80);            // общая яркость 0..255
  strip.animate(AlashNeoPixel::EFFECT_RAINBOW, 20);   // эффект: новый кадр каждые 20 мс
}
void loop() {
  strip.update();                     // рисует кадр, когда пора
}
```

### Что умеет
- **`fill()`, `setPixel()`, `show()`:** залить, закрасить пиксель, отправить. Цвет `0xRRGGBB` или `Color(r, g, b)`.
- **HSV, цветовой круг, радуга, градиент:** `ColorHSV(оттенок, насыщенность, яркость)`, `rainbow()`, `gradient()`.
- **10 эффектов без `delay()`:** радуга, бег, комета, дыхание, мигание, мерцание, огонь, заливка, театральный бег, сканер. `animate()` и `update()`.
- **Защита по току:** `setMaxMilliamps(500)` снижает яркость так, чтобы блок питания или USB не перегрузился; `estimatedMilliamps()` показывает оценку.
- **Яркость без потерь:** `setBrightness()` применяется при отправке, данные пикселей остаются точными. `setGamma(true)` даёт плавные оттенки.
- **RGB и RGBW, любой порядок цветов:** WS2812, WS2812B, SK6812; `ORDER_GRB`, `ORDER_RGB`, `ORDER_GRBW` и другие.
- **Свой вывод сигнала:** AVR 16 МГц, ESP8266, ESP32 с ядром 3.x (RMT). Для других плат `setWriter()`.
- **Затухание и сдвиг:** `fade()`, `rotate()` для своих эффектов.
- **Видно, что происходит:** `diagnose(Serial)` зажигает цвета и подсказывает про питание, порядок цветов и уровни на трёх языках.
- Работает на **Arduino Uno / Nano / Mega, ESP8266, ESP32**.

### Подключение
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

Лента на 5 В от ESP32 и ESP8266 (3,3 В на выходе) надёжнее работает через преобразователь уровня 74AHCT125. Конденсатор 1000 мкФ у питания ленты защищает от скачка при включении.

### Чем отличается от других библиотек
Мы изучили Adafruit NeoPixel, FastLED, NeoPixelBus (Makuna), NeoPatterns и tinyNeoPixel:
- Adafruit NeoPixel (LGPL-3.0), NeoPixelBus (LGPL-3.0) и NeoPatterns (GPL-3.0) нельзя форкнуть в MIT, поэтому драйвер написан заново по даташиту WS2812; FastLED (MIT) огромна и тянет сотни килобайт.
- Эффекты у Adafruit нужно писать самим, у NeoPatterns они есть, но без защиты по току и без ESP32 RMT на ядре 3.x; у нас 10 эффектов и `setMaxMilliamps()`.
- Яркость у Adafruit портит данные пикселей; у нас она применяется при отправке.
- У всех только английский; у нас `diagnose()` на ru/kk/en.

### Примеры
`Basic`, `Rainbow`, `Effects`, `Diagnose`

### Тесты
Без железа проверены (с AddressSanitizer): порядок байтов для всех форматов и RGBW, HSV, радуга, градиент, затухание, сдвиг, яркость и гамма на выходе, оценка тока и ограничение, пауза защёлки, все 10 эффектов, переполнение `millis()`, ошибки и диагностика. Сам импульсный сигнал (время бита) проверен только по ассемблерному листингу AVR и сборкой для ESP; требует проверки на ленте: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**WS2812 / NeoPixel** адрестелетін лентасы үш жолмен: `strip.fill(RED); strip.show();`. HSV түстері, кемпірқосақ, градиент, гамма, **қуат көзін ток бойынша қорғау** және `delay()` жоқ **10 дайын эффект** (кемпірқосақ, от, комета, тыныс және т.б.). Arduino Uno, ESP8266 және ESP32 үшін өз сигнал шығаруымыз. Тәуелділіксіз.

```cpp
#include <AlashNeoPixel.h>
AlashNeoPixel strip(30, 6);           // 30 светодиодов, DIN на пин 6

void setup() {
  strip.begin();
  strip.setBrightness(80);            // общая яркость 0..255
  strip.animate(AlashNeoPixel::EFFECT_RAINBOW, 20);   // эффект: новый кадр каждые 20 мс
}
void loop() {
  strip.update();                     // рисует кадр, когда пора
}
```

### Мүмкіндіктері
- **`fill()`, `setPixel()`, `show()`:** толтыру, пиксельді бояу, жіберу. Түс `0xRRGGBB` немесе `Color(r, g, b)`.
- **HSV, түс шеңбері, кемпірқосақ, градиент:** `ColorHSV(реңк, қанықтық, жарықтық)`, `rainbow()`, `gradient()`.
- **`delay()` жоқ 10 эффект:** кемпірқосақ, жүгіру, комета, тыныс, жыпылық, жылтыр, от, толтыру, театр жүгіруі, сканер. `animate()` және `update()`.
- **Ток бойынша қорғау:** `setMaxMilliamps(500)` қуат көзі немесе USB артық жүктелмеуі үшін жарықтықты азайтады; `estimatedMilliamps()` бағалайды.
- **Шығынсыз жарықтық:** `setBrightness()` жіберу кезінде қолданылады, пиксель деректері дәл қалады. `setGamma(true)` реңктерді тегістейді.
- **RGB және RGBW, кез келген түс реті:** WS2812, WS2812B, SK6812; `ORDER_GRB`, `ORDER_RGB`, `ORDER_GRBW`.
- **Өз сигнал шығаруымыз:** AVR 16 МГц, ESP8266, 3.x ядролы ESP32 (RMT). Басқа тақталар үшін `setWriter()`.
- **Өшіру және жылжыту:** `fade()`, `rotate()` өз эффектілеріңіз үшін.
- **Не болып жатқаны көрінеді:** `diagnose(Serial)` түстерді жағып, қуат, түс реті мен деңгей туралы үш тілде кеңес береді.
- **Arduino Uno / Nano / Mega, ESP8266, ESP32** тақталарында жұмыс істейді.

### Қосу
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

5 В лента ESP32 және ESP8266 (шығыста 3,3 В) арқылы 74AHCT125 деңгей түрлендіргішімен сенімдірек жұмыс істейді. Лента қуатына қосылған 1000 мкФ конденсатор қосылу кезіндегі секіруден қорғайды.

### Басқа кітапханалардан айырмашылығы
Adafruit NeoPixel, FastLED, NeoPixelBus (Makuna), NeoPatterns және tinyNeoPixel қарадық:
- Adafruit NeoPixel (LGPL-3.0), NeoPixelBus (LGPL-3.0) және NeoPatterns (GPL-3.0) MIT-ке форк жасалмайды, сондықтан драйвер WS2812 даташиті бойынша қайта жазылды; FastLED (MIT) өте үлкен.
- Adafruit-те эффекттерді өзіңіз жазасыз; бізде 10 эффект және `setMaxMilliamps()` бар.
- Adafruit-те жарықтық пиксель деректерін бүлдіреді; бізде жіберу кезінде қолданылады.
- Барлығында тек ағылшынша; бізде `diagnose()` ru/kk/en.

### Мысалдар
`Basic`, `Rainbow`, `Effects`, `Diagnose`

### Тесттер
Құрылғысыз тексерілген (AddressSanitizer-мен): барлық пішім мен RGBW үшін байт реті, HSV, кемпірқосақ, градиент, өшу, жылжыту, шығыстағы жарықтық пен гамма, ток бағасы мен шектеу, 10 эффект, `millis()` толуы, қателер. Импульстік сигналдың өзі тек ассемблер тізімі бойынша тексерілген, лентада тексеру керек: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

**WS2812 / NeoPixel** addressable strips in three lines: `strip.fill(RED); strip.show();`. HSV colors, rainbow, gradient, gamma, **power-supply current protection** and **10 ready effects** (rainbow, fire, comet, breathe and more) without `delay()`. Its own signal output for Arduino Uno, ESP8266 and ESP32. No dependencies.

```cpp
#include <AlashNeoPixel.h>
AlashNeoPixel strip(30, 6);           // 30 светодиодов, DIN на пин 6

void setup() {
  strip.begin();
  strip.setBrightness(80);            // общая яркость 0..255
  strip.animate(AlashNeoPixel::EFFECT_RAINBOW, 20);   // эффект: новый кадр каждые 20 мс
}
void loop() {
  strip.update();                     // рисует кадр, когда пора
}
```

### Features
- **`fill()`, `setPixel()`, `show()`:** fill, color a pixel, send. A color is `0xRRGGBB` or `Color(r, g, b)`.
- **HSV, color wheel, rainbow, gradient:** `ColorHSV(hue, saturation, value)`, `rainbow()`, `gradient()`.
- **10 effects without `delay()`:** rainbow, chase, comet, breathe, blink, twinkle, fire, wipe, theater chase, scanner. `animate()` and `update()`.
- **Current protection:** `setMaxMilliamps(500)` lowers the brightness so the supply or USB is not overloaded; `estimatedMilliamps()` shows the estimate.
- **Lossless brightness:** `setBrightness()` is applied on output, the pixel data stays exact. `setGamma(true)` gives smooth shades.
- **RGB and RGBW, any color order:** WS2812, WS2812B, SK6812; `ORDER_GRB`, `ORDER_RGB`, `ORDER_GRBW` and more.
- **Its own signal output:** AVR 16 MHz, ESP8266, ESP32 with core 3.x (RMT). `setWriter()` for other boards.
- **Fade and rotate:** `fade()`, `rotate()` for your own effects.
- **See what happens:** `diagnose(Serial)` lights the colors and gives tips on power, color order and levels in three languages.
- Works on **Arduino Uno / Nano / Mega, ESP8266, ESP32**.

### Wiring
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

A 5 V strip driven from ESP32 and ESP8266 (3.3 V output) works more reliably through a 74AHCT125 level shifter. A 1000 uF capacitor across the strip power protects from the switch-on surge.

### How it differs from other libraries
We studied Adafruit NeoPixel, FastLED, NeoPixelBus (Makuna), NeoPatterns and tinyNeoPixel:
- Adafruit NeoPixel (LGPL-3.0), NeoPixelBus (LGPL-3.0) and NeoPatterns (GPL-3.0) cannot be forked into MIT, so the driver is written from the WS2812 datasheet; FastLED (MIT) is huge and costs hundreds of kilobytes.
- With Adafruit you write the effects yourself; NeoPatterns has them but no current limit; ours has 10 effects and `setMaxMilliamps()`.
- Adafruit brightness destroys pixel data; ours is applied on output.
- All are English only; ours has `diagnose()` in ru/kk/en.

### Examples
`Basic`, `Rainbow`, `Effects`, `Diagnose`

### Tests
Checked without hardware (with AddressSanitizer): byte order for all formats and RGBW, HSV, rainbow, gradient, fade, rotate, brightness and gamma on output, current estimate and limit, latch pause, all 10 effects, `millis()` overflow, errors and diagnostics. The pulse timing itself is checked only from the AVR assembly listing and by building for ESP, and needs a check on a real strip: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
