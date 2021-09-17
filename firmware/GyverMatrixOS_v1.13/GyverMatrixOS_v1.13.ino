/*
  Скетч к проекту "Адресная матрица"
  Гайд по постройке матрицы: https://alexgyver.ru/matrix_guide/
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverMatrixBT/
  Подробное описание прошивки: https://alexgyver.ru/gyvermatrixos-guide/
  Исходники на GitHub: https://github.com/AlexGyver/GyverMatrixBT/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/
*/

// GyverMatrixOS
// Версия прошивки 1.13, совместима с приложением GyverMatrixBT версии 1.13 и выше
// Вариативность эффектов, +2 эффекта, правка и изминение 2 эффектов 

#include "Constants.h"
#include "FastLED.h"
CRGB leds[NUM_LEDS];
String runningText = "";

static const byte maxDim = max(WIDTH, HEIGHT);
byte buttons = 4;   // 0 - верх, 1 - право, 2 - низ, 3 - лево, 4 - не нажата
int globalBrightness = BRIGHTNESS;
byte globalSpeed = 200;
uint32_t globalColor = 0x00ff00;   // цвет при запуске зелёный
byte breathBrightness;
boolean loadingFlag = true;
byte variant;
byte frameNum;
int Score;
int gameSpeed = DEMO_GAME_SPEED;
boolean gameDemo = true;
boolean idleState = true;  // флаг холостого режима работы
boolean BTcontrol = false;  // флаг контроля с блютус. Если false - управление с кнопок
int8_t thisMode = 0;
boolean controlFlag = false;
boolean gamemodeFlag = false;
boolean mazeMode = false;
int effects_speed = D_EFFECT_SPEED;
uint8_t hrs = 10, mins = 25, secs;
boolean dotFlag;
byte modeCode;    // 0 бегущая, 1 часы, 2 игры, 3 нойс маднесс и далее, 21 гифка или картинка,
boolean fullTextFlag = false;
boolean clockSet = false;
uint8_t patternIdx = -1;
byte animation = 0;

#if (USE_FONTS == 1)
#include "fonts.h"
#endif

#if (USE_MODULE_EFFECTS == 1)
#include "Patterns.h"
#endif

uint32_t autoplayTime = ((long)AUTOPLAY_PERIOD * 1000);
uint32_t autoplayTimer;

#include "timerMinim.h"
timerMinim effectTimer(D_EFFECT_SPEED);
timerMinim gameTimer(DEMO_GAME_SPEED);
timerMinim scrollTimer(D_TEXT_SPEED);
timerMinim idleTimer((long)IDLE_TIME * 1000);
timerMinim changeTimer(70);
timerMinim halfsecTimer(500);

#if ((USE_RTC) && (USE_CLOCK) && (MCU_TYPE == 0))
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
// RTC_DS1307 rtc;
#endif

void setup() {
#if (BT_MODE == 1)
  Serial.begin(9600);
#endif

#if ((USE_RTC) && (USE_CLOCK == 1) && (MCU_TYPE == 0))
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
#endif

  // настройки ленты
  FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();
  randomSeed(analogRead(0) + analogRead(1));    // пинаем генератор случайных чисел
}

void loop() {
  customRoutine();
  bluetoothRoutine();
}
