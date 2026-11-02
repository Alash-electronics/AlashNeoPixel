// [ru] AlashNeoPixel: все готовые эффекты по очереди, каждые 6 секунд новый. Название эффекта выводится в Serial Monitor (115200).
//      Подключение: DIN ленты к пину 6 (ESP32 и ESP8266: пин 4) через резистор 330 Ом, питание 5 В, общий GND.
// [kk] AlashNeoPixel: барлық дайын эффектілер кезекпен, әр 6 секунд сайын жаңасы. Эффект атауы Serial Monitor-ға (115200) шығады.
//      Қосу: лента DIN 6 пинге (ESP32 және ESP8266: 4 пин) 330 Ом резистор арқылы, қуат 5 В, ортақ GND.
// [en] AlashNeoPixel: all the ready effects in turn, a new one every 6 seconds. The effect name is printed to the Serial Monitor (115200).
//      Wiring: strip DIN to pin 6 (ESP32 and ESP8266: pin 4) through a 330 Ohm resistor, 5 V power, common GND.
#include <AlashNeoPixel.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const uint8_t PIN = 4;
#else
const uint8_t PIN = 6;
#endif

AlashNeoPixel strip(30, PIN);

struct Show { AlashNeoPixel::Effect effect; uint16_t stepMs; uint32_t color; const char *name; };
const Show shows[] = {
  {AlashNeoPixel::EFFECT_RAINBOW, 20, AlashNeoPixel::WHITE, "RAINBOW"},
  {AlashNeoPixel::EFFECT_CHASE, 40, AlashNeoPixel::RED, "CHASE"},
  {AlashNeoPixel::EFFECT_COMET, 30, AlashNeoPixel::CYAN, "COMET"},
  {AlashNeoPixel::EFFECT_BREATHE, 15, AlashNeoPixel::PURPLE, "BREATHE"},
  {AlashNeoPixel::EFFECT_BLINK, 300, AlashNeoPixel::ORANGE, "BLINK"},
  {AlashNeoPixel::EFFECT_TWINKLE, 40, AlashNeoPixel::WARM_WHITE, "TWINKLE"},
  {AlashNeoPixel::EFFECT_FIRE, 30, AlashNeoPixel::WHITE, "FIRE"},
  {AlashNeoPixel::EFFECT_WIPE, 40, AlashNeoPixel::GREEN, "WIPE"},
  {AlashNeoPixel::EFFECT_THEATER, 120, AlashNeoPixel::YELLOW, "THEATER"},
  {AlashNeoPixel::EFFECT_SCANNER, 30, AlashNeoPixel::RED, "SCANNER"},
};
const uint8_t SHOWS = sizeof(shows) / sizeof(shows[0]);

void startShow(uint8_t i) {
  Serial.println(shows[i].name);
  strip.clear();
  strip.animate(shows[i].effect, shows[i].stepMs, shows[i].color);
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(60);
  strip.setGamma(true);
  strip.setMaxMilliamps(500);
  startShow(0);
}

void loop() {
  static uint8_t current = 0;
  static uint32_t changed = 0;
  strip.update();
  if (millis() - changed > 6000) {      // каждые 6 секунд следующий эффект
    changed = millis();
    current = (current + 1) % SHOWS;
    startShow(current);
  }
}
