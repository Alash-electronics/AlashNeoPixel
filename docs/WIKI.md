# AlashNeoPixel: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Адресная лента **WS2812 / NeoPixel** в три строки: `strip.fill(RED); strip.show();`. Цвета по HSV, радуга, градиент, гамма, **защита блока питания по току** и **10 готовых эффектов** (радуга, огонь, комета, дыхание и другие) без `delay()`. Свой вывод сигнала для Arduino Uno, ESP8266 и ESP32. Без зависимостей.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashNeoPixel** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

Лента на 5 В от ESP32 и ESP8266 (3,3 В на выходе) надёжнее работает через преобразователь уровня 74AHCT125. Конденсатор 1000 мкФ у питания ленты защищает от скачка при включении.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `AlashNeoPixel(count, pin, order)` | Лента: число светодиодов, пин DIN, порядок цветов |
| `begin() / end()` | Запустить (память, пин) / освободить |
| `fill(color, first, count) / clear()` | Залить цветом / погасить |
| `setPixel(i, r, g, b) / setPixel(i, color) / getPixel(i)` | Один пиксель |
| `Color(r, g, b, w) / ColorHSV(h, s, v) / Wheel(pos) / Blend(a, b, amt)` | Цвета: RGB, HSV, круг, смесь |
| `rainbow(start, span) / gradient(a, b) / fade(keep) / rotate()` | Радуга, градиент, затухание, сдвиг |
| `show() / canShow()` | Отправить на ленту / можно ли |
| `setBrightness(b) / setGamma(on)` | Общая яркость и гамма |
| `setMaxMilliamps(mA) / estimatedMilliamps()` | Ограничение тока и оценка |
| `animate(effect, stepMs, color) / update() / stopEffect()` | Эффекты без delay() |
| `setWriter(fn)` | Свой вывод для платы без драйвера |
| `diagnose(Serial) / errorText() / setLanguage()` | Самопроверка и сообщения на ru/kk/en |

### Если что-то не работает
**Лента не светится**

Проверьте, что сигнал идёт на вход DIN, а не DOUT, что GND ленты соединён с GND платы и что питание 5 В. Запустите пример `Diagnose`: он зажигает цвета и подсказывает. Не забудьте `strip.begin()` и `strip.show()`.

**Цвета перепутаны (красный вместо зелёного)**

У разных лент разный порядок байтов. Замените `ORDER_GRB` на `ORDER_RGB` (или `ORDER_BRG`). Для SK6812 с белым каналом используйте `ORDER_GRBW`.

**Мерцает или первый светодиод показывает мусор**

Добавьте резистор 330 Ом в линию данных и конденсатор 1000 мкФ у питания. Провод данных делайте коротким, общий GND обязателен. На ESP32 и ESP8266 (3,3 В на выходе) для ленты на 5 В поставьте преобразователь уровня 74AHCT125.

**Плата перезагружается или лента тускнеет к концу**

Не хватает питания: каждый светодиод до 60 мА на полной белой. Поставьте отдельный блок 5 В, подключите питание с обоих концов длинной ленты, а в коде задайте `setMaxMilliamps()` и меньшую `setBrightness()`.

**На ESP8266 или ESP32 лента сбоит при работе Wi-Fi**

ESP8266 отправляет сигнал программно и отключает прерывания на время кадра: Wi-Fi может сбить бит на длинной ленте. Уменьшите число светодиодов, отправляйте реже или возьмите ESP32: он шлёт сигнал аппаратно (RMT).

**Ошибка «нет драйвера» или не хватает памяти**

Встроенный драйвер есть для AVR на 16 МГц, ESP8266 и ESP32 с ядром 3.x. Для других плат задайте `setWriter()`. На Uno всего 2 КБ памяти: около 500 светодиодов максимум, а эффект FIRE требует ещё байт на светодиод.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Rainbow`, `Effects`, `Diagnose`

---

## Қазақша

**WS2812 / NeoPixel** адрестелетін лентасы үш жолмен: `strip.fill(RED); strip.show();`. HSV түстері, кемпірқосақ, градиент, гамма, **қуат көзін ток бойынша қорғау** және `delay()` жоқ **10 дайын эффект** (кемпірқосақ, от, комета, тыныс және т.б.). Arduino Uno, ESP8266 және ESP32 үшін өз сигнал шығаруымыз. Тәуелділіксіз.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashNeoPixel** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

5 В лента ESP32 және ESP8266 (шығыста 3,3 В) арқылы 74AHCT125 деңгей түрлендіргішімен сенімдірек жұмыс істейді. Лента қуатына қосылған 1000 мкФ конденсатор қосылу кезіндегі секіруден қорғайды.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `AlashNeoPixel(count, pin, order)` | Лента: саны, пин, түс реті |
| `begin() / end()` | Іске қосу / босату |
| `fill(color, first, count) / clear()` | Түспен толтыру / өшіру |
| `setPixel(i, r, g, b) / setPixel(i, color) / getPixel(i)` | Бір пиксель |
| `Color(r, g, b, w) / ColorHSV(h, s, v) / Wheel(pos) / Blend(a, b, amt)` | Түстер |
| `rainbow(start, span) / gradient(a, b) / fade(keep) / rotate()` | Кемпірқосақ, градиент, өшу, жылжыту |
| `show() / canShow()` | Лентаға жіберу |
| `setBrightness(b) / setGamma(on)` | Жарықтық және гамма |
| `setMaxMilliamps(mA) / estimatedMilliamps()` | Ток шектеуі |
| `animate(effect, stepMs, color) / update() / stopEffect()` | delay() жоқ эффекттер |
| `setWriter(fn)` | Драйвері жоқ тақта үшін |
| `diagnose(Serial) / errorText() / setLanguage()` | Өзін-өзі тексеру |

### Бірдеңе жұмыс істемесе
**Лента жанбайды**

Сигнал DOUT-қа емес, DIN кірісіне берілетінін, лента GND-сы тақта GND-мен қосылғанын және қуат 5 В екенін тексеріңіз. `Diagnose` мысалын іске қосыңыз. `strip.begin()` және `strip.show()` ұмытпаңыз.

**Түстер шатасқан (жасыл орнына қызыл)**

Әртүрлі лентада байт реті әртүрлі. `ORDER_GRB` орнына `ORDER_RGB` қойыңыз. Ақ арнасы бар SK6812 үшін `ORDER_GRBW`.

**Жыпылықтайды немесе бірінші светодиод қоқыс көрсетеді**

Деректер желісіне 330 Ом резистор және қуатқа 1000 мкФ конденсатор қосыңыз. Деректер сымы қысқа, GND ортақ. ESP32 мен ESP8266 үшін 5 В лентаға 74AHCT125 деңгей түрлендіргіші.

**Тақта қайта қосылады немесе лента соңына қарай күңгірттенеді**

Қуат жетпейді: әр светодиод толық ақта 60 мА-ге дейін. Бөлек 5 В блок қойыңыз, ұзын лентаның екі шетінен қуат беріңіз, кодта `setMaxMilliamps()` және аздау `setBrightness()` беріңіз.

**ESP8266 немесе ESP32-де Wi-Fi кезінде лента ақаулайды**

ESP8266 сигналды бағдарламалық жібереді де, кадр кезінде үзілістерді өшіреді: Wi-Fi ұзын лентада битті бұзуы мүмкін. Светодиодтарды азайтыңыз, сирек жіберіңіз немесе ESP32 алыңыз: ол сигналды аппараттық (RMT) жібереді.

**«Драйвер жоқ» қатесі немесе жад жетпейді**

Кіріктірілген драйвер 16 МГц AVR, ESP8266 және 3.x ядролы ESP32 үшін бар. Басқа тақталар үшін `setWriter()`. Uno-да 2 КБ ғана: ең көбі 500 светодиод, FIRE эффектісіне тағы байт керек.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Rainbow`, `Effects`, `Diagnose`

---

## English

**WS2812 / NeoPixel** addressable strips in three lines: `strip.fill(RED); strip.show();`. HSV colors, rainbow, gradient, gamma, **power-supply current protection** and **10 ready effects** (rainbow, fire, comet, breathe and more) without `delay()`. Its own signal output for Arduino Uno, ESP8266 and ESP32. No dependencies.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashNeoPixel** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| Лента / Strip | Плата / Board |
|---|---|
| 5V | 5V (для длинной ленты отдельный блок питания) |
| GND | GND (общий с платой) |
| DIN | пин 6 (ESP32 / ESP8266: пин 4) через резистор 300..470 Ом |

A 5 V strip driven from ESP32 and ESP8266 (3.3 V output) works more reliably through a 74AHCT125 level shifter. A 1000 uF capacitor across the strip power protects from the switch-on surge.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `AlashNeoPixel(count, pin, order)` | Strip: LED count, DIN pin, color order |
| `begin() / end()` | Start (memory, pin) / release |
| `fill(color, first, count) / clear()` | Fill with a color / clear |
| `setPixel(i, r, g, b) / setPixel(i, color) / getPixel(i)` | One pixel |
| `Color(r, g, b, w) / ColorHSV(h, s, v) / Wheel(pos) / Blend(a, b, amt)` | Colors: RGB, HSV, wheel, blend |
| `rainbow(start, span) / gradient(a, b) / fade(keep) / rotate()` | Rainbow, gradient, fade, rotate |
| `show() / canShow()` | Send to the strip / is it allowed |
| `setBrightness(b) / setGamma(on)` | Global brightness and gamma |
| `setMaxMilliamps(mA) / estimatedMilliamps()` | Current limit and estimate |
| `animate(effect, stepMs, color) / update() / stopEffect()` | Effects without delay() |
| `setWriter(fn)` | Custom output for a board without a driver |
| `diagnose(Serial) / errorText() / setLanguage()` | Self-check and messages in ru/kk/en |

### If something does not work
**The strip does not light**

Check that the signal goes to DIN, not DOUT, that the strip GND is joined with the board GND and that power is 5 V. Run the `Diagnose` example: it lights colors and gives hints. Do not forget `strip.begin()` and `strip.show()`.

**Colors are swapped (red instead of green)**

Strips differ in byte order. Replace `ORDER_GRB` with `ORDER_RGB` (or `ORDER_BRG`). For SK6812 with a white channel use `ORDER_GRBW`.

**It flickers or the first LED shows garbage**

Add a 330 ohm resistor in the data line and a 1000 uF capacitor at the power. Keep the data wire short with a common GND. For ESP32 and ESP8266 (3.3 V output) and a 5 V strip use a 74AHCT125 level shifter.

**The board resets or the strip dims towards the end**

Not enough power: each LED draws up to 60 mA at full white. Use a separate 5 V supply, feed long strips from both ends, and in code set `setMaxMilliamps()` and a lower `setBrightness()`.

**On ESP8266 or ESP32 the strip glitches while Wi-Fi works**

ESP8266 sends the signal in software and disables interrupts for the frame: Wi-Fi can break a bit on a long strip. Use fewer LEDs, send less often or take an ESP32: it sends the signal in hardware (RMT).

**A "no driver" error or not enough memory**

The built-in driver exists for AVR at 16 MHz, ESP8266 and ESP32 with core 3.x. For other boards provide `setWriter()`. Uno has only 2 KB: about 500 LEDs at most, and the FIRE effect needs one more byte per LED.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Rainbow`, `Effects`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashNeoPixel
