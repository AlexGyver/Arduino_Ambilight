/*
   Управление лентой на WS2812 с компьютера + динамическая яркость
   Создано не знаю кем, допилил и перевёл AlexGyver http://alexgyver.ru/
   2017
*/
//----------------------НАСТРОЙКИ-----------------------
#define NUM_LEDS 98          // число светодиодов в ленте
#define DI_PIN 13            // пин, к которому подключена лента
#define OFF_TIME 10          // время (секунд), через которое лента выключится при пропадаании сигнала

#define start_flashes 0      // проверка цветов при запуске (1 - включить, 0 - выключить)

#define auto_bright 1        // автоматическая подстройка яркости от уровня внешнего освещения (1 - включить, 0 - выключить)
#define max_bright 255       // максимальная яркость (0 - 255)
#define min_bright 50        // минимальная яркость (0 - 255)
#define bright_constant 500  // константа усиления от внешнего света (0 - 1023)
// чем МЕНЬШЕ константа, тем "резче" будет прибавляться яркость
#define coef 0.9             // коэффициент фильтра (0.0 - 1.0), чем больше - тем медленнее меняется яркость
//----------------------НАСТРОЙКИ-----------------------

int new_bright, new_bright_f;
unsigned long bright_timer, off_timer;

#define serialRate 115200  // скорость связи с ПК
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;  // кодовое слово Ada для связи
#include <FastLED.h>
CRGB leds[NUM_LEDS];  // создаём ленту
boolean led_state = true;  // флаг состояния ленты

void setup()
{
  FastLED.addLeds<WS2812, DI_PIN, GRB>(leds, NUM_LEDS);  // инициализация светодиодов

  // вспышки красным синим и зелёным при запуске (можно отключить)
  if (start_flashes) {
    LEDS.showColor(CRGB(255, 0, 0));
    delay(500);
    LEDS.showColor(CRGB(0, 255, 0));
    delay(500);
    LEDS.showColor(CRGB(0, 0, 255));
    delay(500);
    LEDS.showColor(CRGB(0, 0, 0));
  }

  Serial.begin(serialRate);
  Serial.print("Ada\n");     // Связаться с компом
}

void check_connection() {
  if (led_state) {
    if (millis() - off_timer > (OFF_TIME * 1000)) {
      led_state = !led_state;
      FastLED.clear();
      FastLED.show();
    }
  }
}

void loop() {
  if (auto_bright) {                         // если включена адаптивная яркость
    if (millis() - bright_timer > 100) {     // каждые 100 мс
      bright_timer = millis();               // сброить таймер
      new_bright = map(analogRead(6), 0, bright_constant, min_bright, max_bright);   // считать показания с фоторезистора, перевести диапазон
      new_bright = constrain(new_bright, min_bright, max_bright);
      new_bright_f = new_bright_f * coef + new_bright * (1 - coef);
      LEDS.setBrightness(new_bright_f);      // установить новую яркость
    }
  }
  if (!led_state) led_state = true;
  off_timer = millis();  

  for (i = 0; i < sizeof prefix; ++i) {
waitLoop: while (!Serial.available()) check_connection();;
    if (prefix[i] == Serial.read()) continue;
    i = 0;
    goto waitLoop;
  }

  while (!Serial.available()) check_connection();;
  hi = Serial.read();
  while (!Serial.available()) check_connection();;
  lo = Serial.read();
  while (!Serial.available()) check_connection();;
  chk = Serial.read();
  if (chk != (hi ^ lo ^ 0x55))
  {
    i = 0;
    goto waitLoop;
  }

  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    byte r, g, b;
    // читаем данные для каждого цвета
    while (!Serial.available()) check_connection();
    r = Serial.read();
    while (!Serial.available()) check_connection();
    g = Serial.read();
    while (!Serial.available()) check_connection();
    b = Serial.read();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
  FastLED.show();  // записываем цвета в ленту
}
