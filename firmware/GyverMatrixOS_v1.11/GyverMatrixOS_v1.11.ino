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
// Версия прошивки 1.12, совместима с приложением GyverMatrixBT версии 1.13 и выше

// ************************ МАТРИЦА *************************
// если прошивка не лезет в Arduino NANO - отключай режимы! Строка 60 и ниже

#define BRIGHTNESS 32         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 4000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 16             // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настрйоке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

#define MCU_TYPE 1            // микроконтроллер: 
//                            0 - AVR (Arduino NANO/MEGA/UNO)
//                            1 - ESP8266 (NodeMCU, Wemos D1)
//                            2 - STM32 (Blue Pill)

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************
#define D_TEXT_SPEED 100      // скорость бегущего текста по умолчанию (мс)
#define D_EFFECT_SPEED 80     // скорость эффектов по умолчанию (мс)
#define D_GAME_SPEED 250      // скорость игр по умолчанию (мс)
#define D_GIF_SPEED 100       // скорость гифок (мс)
#define DEMO_GAME_SPEED 60    // скорость игр в демо режиме (мс)

boolean AUTOPLAY = 1;         // 0 выкл / 1 вкл автоматическую смену режимов (откл. можно со смартфона)
int AUTOPLAY_PERIOD = 30;     // время между авто сменой режимов (секунды)
#define IDLE_TIME 300         // время бездействия кнопок или Bluetooth (в секундах) после которого запускается автосмена режимов и демо в играх

// о поддерживаемых цветах читай тут https://alexgyver.ru/gyvermatrixos-guide/
#define GLOBAL_COLOR_1 CRGB::Green    // основной цвет №1 для игр
#define GLOBAL_COLOR_2 CRGB::Orange   // основной цвет №2 для игр

#define SCORE_SIZE 0          // размер символов счёта в игре. 0 - маленький для 8х8 (шрифт 3х5), 1 - большой (шрифт 5х7)
#define FONT_TYPE 1           // (0 / 1) два вида маленького шрифта в выводе игрового счёта

// ************** ОТКЛЮЧЕНИЕ КОМПОНЕНТОВ СИСТЕМЫ (для экономии памяти) *************
// внимание! отключение модуля НЕ УБИРАЕТ его эффекты из списка воспроизведения!
// Это нужно сделать вручную во вкладке custom, удалив ненужные функции

#define USE_BUTTONS 0         // использовать физические кнопки управления играми (0 нет, 1 да)
#define BT_MODE 0             // использовать блютус (0 нет, 1 да)
#define WIFI_MODE 1           // использовать WiFi (0 нет, 1 да) - для MCU_TYPE == 1 
#define USE_NOISE_EFFECTS 1   // крутые полноэкранные эффекты (0 нет, 1 да) СИЛЬНО ЖРУТ ПАМЯТЬ!!!11
#define USE_FONTS 1           // использовать буквы (бегущая строка) (0 нет, 1 да)
#define USE_CLOCK 1           // использовать часы (0 нет, 1 да)
#define USE_RTC 0             // использовать часы реального времени DS3231 (0 нет, 1 да)
#define USE_ANIMATION 1       // использовать эффект анимации (0 нет, 1 да)

// игры
#define USE_SNAKE 1           // игра змейка (0 нет, 1 да)
#define USE_TETRIS 1          // игра тетрис (0 нет, 1 да)
#define USE_MAZE 1            // игра лабиринт (0 нет, 1 да)
#define USE_RUNNER 1          // игра бегалка-прыгалка (0 нет, 1 да)
#define USE_FLAPPY 1          // игра flappy bird
#define USE_ARKAN 1           // игра арканоид

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************
// Arduino (Nano, Mega)
#if (MCU_TYPE == 0)
#define LED_PIN 6           // пин ленты
#define BUTT_UP 3           // кнопка вверх
#define BUTT_DOWN 5         // кнопка вниз
#define BUTT_LEFT 2         // кнопка влево
#define BUTT_RIGHT 4        // кнопка вправо
#define BUTT_SET 7          // кнопка выбор/игра

// пины подписаны согласно pinout платы, а не надписям на пинах!
// esp8266 - плату выбирал Wemos D1 R1
#elif (MCU_TYPE == 1)
#define LED_PIN 2           // пин ленты
#define BUTT_UP 14          // кнопка вверх
#define BUTT_DOWN 13        // кнопка вниз
#define BUTT_LEFT 0         // кнопка влево
#define BUTT_RIGHT 12       // кнопка вправо
#define BUTT_SET 15         // кнопка выбор/игра

// STM32 (BluePill) - плату выбирал STM32F103C
#elif (MCU_TYPE == 2)
#define LED_PIN PB12         // пин ленты
#define BUTT_UP PA1          // кнопка вверх
#define BUTT_DOWN PA3        // кнопка вниз
#define BUTT_LEFT PA0        // кнопка влево
#define BUTT_RIGHT PA2       // кнопка вправо
#define BUTT_SET PA4         // кнопка выбор/игра
#endif

// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

#define MC_TEXT 0
#define MC_CLOCK 1
#define MC_GAME 2
#define MC_NOISE_MADNESS 3
#define MC_NOISE_CLOUD 4
#define MC_NOISE_LAVA 5
#define MC_NOISE_PLASMA 6
#define MC_NOISE_RAINBOW 7
#define MC_NOISE_RAINBOW_STRIP 8
#define MC_NOISE_ZEBRA 9
#define MC_NOISE_FOREST 10
#define MC_NOISE_OCEAN 11
#define MC_SNOW 12
#define MC_SPARKLES 13
#define MC_MATRIX 14
#define MC_STARFALL 15
#define MC_BALL 16
#define MC_BALLS 17
#define MC_RAINBOW 18
#define MC_RAINBOW_DIAG 19
#define MC_FIRE 20
#define MC_IMAGE 21

#if (MCU_TYPE == 1)
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OldTime.h>
#endif

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
int scrollSpeed = D_TEXT_SPEED;
int gameSpeed = DEMO_GAME_SPEED;
boolean gameDemo = true;
boolean idleState = true;  // флаг холостого режима работы
boolean BTcontrol = false;  // флаг контроля с блютус или WiFi. Если false - управление с кнопок
int8_t thisMode = 0;
boolean controlFlag = false;
boolean gamemodeFlag = false;
boolean mazeMode = false;
int effects_speed = D_EFFECT_SPEED;
int8_t hrs = 10, mins = 25, secs;
boolean dotFlag;
byte modeCode;    // 0 бегущая, 1 часы, 2 игры, 3 нойс маднесс и далее, 21 гифка или картинка,
boolean fullTextFlag = false;
boolean clockSet = false;

#if (USE_FONTS == 1)
#include "fonts.h"
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

// подключаем внешние файлы с картинками
#if (USE_ANIMATION == 1 && WIDTH == 16 && HEIGHT == 16)
#include "bitmap2.h"
#endif

#if (USE_CLOCK == 1 && (MCU_TYPE == 0 || MCU_TYPE == 1))

#include "RTClib.h"
#if (USE_RTC == 1)
#include <Wire.h>
RTC_DS3231 rtc;
// RTC_DS1307 rtc;
#endif
#endif

#if (MCU_TYPE == 1)

// Раскомментируйте следующую строку, если параметры подключения к WiFiсерверу задаются
// явным образом в блоке ниже. Если строка закомментирована - блок определения параметров подключения в
// точно таком же формате вынесен в отдельный файл 'WiFiNet.h' и переменные при сборке скетча будут браться из него.

// #define public
#ifndef public 
#include "WiFiNet.h"          // приватные данные и пароли доступа к WiFi сети
#else
  char ssid[] = "****";       // SSID (имя) вашего роутера
  char pass[] = "****";       // пароль роутера
#endif

  boolean connected = false;
  IPAddress timeServerIP; // time.nist.gov адрес сервера NTP
  const char* ntpServerName = "time.nist.gov";
  #define NTP_PACKET_SIZE 48 // NTP время в первых 48 байтах сообщения
  #define SYNC_TIME_PERIOD 60
  byte packetBuffer[ NTP_PACKET_SIZE]; // буффер для хранения входящих и исходящих пакетов

  WiFiUDP udp;
  long timeZoneOffset = 2L;            // set this to the offset in seconds to your local time;
  long daylight = 1;
  unsigned int localPort = 2390;       // local port to listen for UDP packets
  long ntp_t = 0;
  byte init_time = 0;
  timerMinim WifiTimer(500);
  
  timerMinim NTPCheck(1000 * 60 * SYNC_TIME_PERIOD); // Сверяем время через SYNC_TIME_PERIOD минут
#endif

void setup() {
#if (BT_MODE == 1)
  Serial.begin(9600);
#endif

#if (MCU_TYPE == 1 && WIFI_MODE == 1)
// Если BT не используется - в Serial выводятся диагностические сообщения.
// Если BT используется - Serial исполльзуется для коммуникации через BT
#if (BT_MODE == 0)
  Serial.begin(115200);
#endif
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif

#if (USE_CLOCK == 1 && USE_RTC == 1 && (MCU_TYPE == 0 || MCU_TYPE == 1))
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
  FastLED.setBrightness(globalBrightness);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();
  randomSeed(analogRead(0) + analogRead(1));    // пинаем генератор случайных чисел
}

void loop() {
#if (MCU_TYPE == 1 && WIFI_MODE == 1)
  checkWiFiConnection(); 
#endif
  bluetoothRoutine();
}

// -----------------------------------------

bool wifi_connected = false;
bool printed_1 = false;
bool printed_2 = false;

void checkWiFiConnection() {
  
  // Проверяем подключение к WiFi, при необходимости (пере)подключаемся к сети
  wifi_connected = WiFi.status() == WL_CONNECTED; 
  if (!wifi_connected) {
    if (!printed_1)
    {      
#if (BT_MODE == 0)
      Serial.print("Connecting to ");
      Serial.print(ssid);
      Serial.println("...");
#endif  
      WiFi.begin(ssid, pass);
      printed_1 = true;
      printed_2 = false;
    }
  }

  // Сразу после подключения - печатаем результат подключения
  if (wifi_connected && !printed_2) {
#if (BT_MODE == 0)
    Serial.print("WiFi подключен. IP адрес: ");
    Serial.println(WiFi.localIP());
    Serial.printf("UDP-сервер на порту %d\n", localPort);
#endif  
    printed_1 = false;
    printed_2 = true;
    udp.begin(localPort);
  }
}
