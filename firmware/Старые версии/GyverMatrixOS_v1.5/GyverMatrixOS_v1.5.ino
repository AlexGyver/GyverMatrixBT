/*
  Скетч к проекту "Адресная матрица"
  Гайд по постройке матрицы: https://alexgyver.ru/matrix_guide/
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverMatrixBT/
  Исходники на GitHub: https://github.com/AlexGyver/GyverMatrixBT/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/
*/

// GyverMatrixOS
// Версия прошивки 1.5, совместима с приложением GyverMatrixBT версии 1.8 и выше

// ******************************** НАСТРОЙКИ ********************************
// чем больше матрица и количество частиц (эффекты), тем выше шанс того, что всё зависнет!
// Данный код стабильно работает на матрице 16х16 (256 диодов). Больше - на ваш страх и риск

// ************************ МАТРИЦА *************************
#define BRIGHTNESS 150        // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 16             // высота матрицы

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - последовательная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настрйоке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************
#define D_TEXT_SPEED 100      // скорость бегущего текста по умолчанию (мс)
#define D_EFFECT_SPEED 100    // скорость эффектов по умолчанию (мс) 
#define D_GAME_SPEED 250      // скорость игр по умолчанию (мс)
#define D_GIF_SPEED 80        // скорость гифок (мс)
#define DEMO_GAME_SPEED 60    // скорость игр в демо режиме (мс)

boolean AUTOPLAY = 1;        // 0 выкл / 1 вкл автоматическую смену режимов
#define AUTOPLAY_PERIOD 10    // время между авто сменой режимов (секунды)
#define IDLE_TIME 10          // время бездействия кнопок или Bluetooth (в секундах) после которого запускается автосмена режимов и демо в играх

#define GLOBAL_COLOR_1 CRGB::Green    // основной цвет №1 для игр
#define GLOBAL_COLOR_2 CRGB::Orange   // основной цвет №2 для игр
// поддерживает 150 цветов, названия можно посмотреть в библиотеке FastLED, файл pixeltypes.h, строка 590
// также цвет можно задавать в формате HEX (0XFF25AB). Генерировать в любом онлайн колорпикере или в фотошопе!

#define SCORE_SIZE 0          // размер символов счёта в игре. 0 - маленький для 8х8 (шрифт 3х5), 1 - большой (шрифт 5х7)
#define FONT_TYPE 1           // (0 / 1) два вида маленького шрифта в выводе игрового счёта

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************
#define LED_PIN 6           // пин ленты

#define BUTT_UP 3           // кнопка вверх
#define BUTT_DOWN 5         // кнопка вниз
#define BUTT_LEFT 4         // кнопка влево
#define BUTT_RIGHT 2        // кнопка вправо
#define BUTT_SET 7          // кнопка выбор/игра

// ************** ОТКЛЮЧЕНИЕ КОМПОНЕНТОВ СИСТЕМЫ (для экономии памяти) *************
#define USE_BUTTONS 1       // использовать физические кнопки управления играми (0 нет, 1 да)
#define BT_MODE 0           // использовать блютус (0 нет, 1 да)
#define USE_NOISE_EFFECTS 1 // крутые полноэкранные эффекты (0 нет, 1 да) СИЛЬНО ЖРУТ ПАМЯТЬ!!!11
#define USE_FONTS 1         // использовать буквы (бегущая строка) (0 нет, 1 да)

#define USE_TETRIS 1        // тетрис (0 нет, 1 да)
#define USE_SNAKE 1         // змейка (0 нет, 1 да)
#define USE_MAZE 0          // лабиринт (0 нет, 1 да)

// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT

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
byte frameNum;
int gameSpeed = DEMO_GAME_SPEED;
boolean gameDemo = true;
boolean idleState = true;  // флаг холостого режима работы
boolean BTcontrol = false;  // флаг контроля с блютус. Если false - управление с кнопок
int8_t thisMode = 0;
boolean controlFlag = false;
boolean gamemodeFlag = false;
boolean mazeMode = false;
int effects_speed = D_EFFECT_SPEED;

#if (USE_FONTS == 1)
#include "fonts.h"
#endif

#include "timerMinim.h"
timerMinim autoplayTimer((long)AUTOPLAY_PERIOD * 1000);
timerMinim effectTimer(D_EFFECT_SPEED);
timerMinim gameTimer(DEMO_GAME_SPEED);
timerMinim scrollTimer(D_TEXT_SPEED);
timerMinim idleTimer((long)IDLE_TIME * 1000);
timerMinim changeTimer(80);

void setup() {
#if (BT_MODE == 1)
  Serial.begin(9600);
#endif
  // настройки ленты
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
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
