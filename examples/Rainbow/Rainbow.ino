// [ru] AlashNeoPixel: бегущая радуга. Эффект сам считает кадры, loop() только вызывает update(), поэтому остаётся место для другого кода.
//      Подключение: DIN ленты к пину 6 (ESP32 и ESP8266: пин 4) через резистор 330 Ом, питание 5 В, общий GND.
// [kk] AlashNeoPixel: жүгіретін кемпірқосақ. Эффект кадрларды өзі санайды, loop() тек update() шақырады, сондықтан басқа код үшін орын қалады.
//      Қосу: лента DIN 6 пинге (ESP32 және ESP8266: 4 пин) 330 Ом резистор арқылы, қуат 5 В, ортақ GND.
// [en] AlashNeoPixel: a running rainbow. The effect counts the frames itself, loop() only calls update(), so there is room for other code.
//      Wiring: strip DIN to pin 6 (ESP32 and ESP8266: pin 4) through a 330 Ohm resistor, 5 V power, common GND.
#include <AlashNeoPixel.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
const uint8_t PIN = 4;
#else
const uint8_t PIN = 6;
#endif

AlashNeoPixel strip(30, PIN);

void setup() {
  strip.begin();
  strip.setBrightness(60);
  strip.setGamma(true);                         // плавные оттенки
  strip.setMaxMilliamps(500);                   // не перегружать USB: ток не больше 500 мА
  strip.animate(AlashNeoPixel::EFFECT_RAINBOW, 20);   // новый кадр каждые 20 мс
}

void loop() {
  strip.update();                               // рисует кадр, когда пора
  // здесь можно делать что угодно ещё: читать кнопки, датчики
}
