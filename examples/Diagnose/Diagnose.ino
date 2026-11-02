// AlashNeoPixel: самопроверка. Печатает настройки, зажигает красный, зелёный, синий и белый, потом пускает бегущую точку.
// Подсказки (питание, порядок цветов, преобразователь уровня) выводятся в Serial Monitor (115200).
#include <AlashNeoPixel.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const uint8_t PIN = 4;
#else
const uint8_t PIN = 6;
#endif

AlashNeoPixel strip(30, PIN, AlashNeoPixel::ORDER_GRB);

void setup() {
  Serial.begin(115200);
  strip.setLanguage(AlashNeoPixel::LANG_RU);    // LANG_KK или LANG_EN
  if (!strip.begin()) Serial.println(strip.errorText());
  strip.diagnose(Serial);
}

void loop() {}
