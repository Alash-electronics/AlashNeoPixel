// [ru] AlashNeoPixel: самый простой пример. Лента из 8 светодиодов по очереди загорается красным, зелёным и синим.
//
//      Подключение (лента WS2812 / NeoPixel):
//        5V  ленты к 5V  (для длинных лент нужен отдельный блок питания, общий GND с платой)
//        GND ленты к GND
//        DIN ленты к пину 6 (на ESP32 и ESP8266 к пину 4) через резистор 330 Ом
//      Если красный и зелёный перепутаны, замените ORDER_GRB на ORDER_RGB.
// [kk] AlashNeoPixel: ең қарапайым мысал. 8 жарықдиодты лента қызыл, жасыл және көк түспен кезекпен жанады.
//
//      Қосу (WS2812 / NeoPixel лентасы):
//        лента 5V  5V-қа  (ұзын лентаға бөлек қуат блогы керек, тақтамен ортақ GND)
//        лента GND GND-ға
//        лента DIN 6 пинге (ESP32 және ESP8266-да 4 пинге) 330 Ом резистор арқылы
//      Қызыл мен жасыл шатасса, ORDER_GRB орнына ORDER_RGB қойыңыз.
// [en] AlashNeoPixel: the simplest example. A strip of 8 LEDs lights up red, green and blue in turn.
//
//      Wiring (a WS2812 / NeoPixel strip):
//        strip 5V  to 5V  (long strips need a separate power supply, common GND with the board)
//        strip GND to GND
//        strip DIN to pin 6 (on ESP32 and ESP8266 to pin 4) through a 330 Ohm resistor
//      If red and green are swapped, replace ORDER_GRB with ORDER_RGB.
#include <AlashNeoPixel.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const uint8_t PIN = 4;
#else
const uint8_t PIN = 6;
#endif
const uint16_t COUNT = 8;

AlashNeoPixel strip(COUNT, PIN, AlashNeoPixel::ORDER_GRB);

void setup() {
  strip.begin();
  strip.setBrightness(80);              // 0..255; ярче, значит больше ток
}

void loop() {
  static const uint32_t colors[] = {AlashNeoPixel::RED, AlashNeoPixel::GREEN, AlashNeoPixel::BLUE};
  static uint8_t i = 0;
  strip.fill(colors[i]);                // залить всю ленту цветом
  strip.show();                         // отправить на ленту
  i = (i + 1) % 3;
  delay(700);
}
