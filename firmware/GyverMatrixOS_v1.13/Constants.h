// ************************ МАТРИЦА *************************
// если прошивка не лезет в Arduino NANO - отключай режимы! Строка 60 и ниже

#define BRIGHTNESS 150        // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 1000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 8             // высота матрицы
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 1         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 2    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 3     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настрйоке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

#define MCU_TYPE 0            // микроконтроллер: 
//                            0 - AVR (Arduino NANO/MEGA/UNO)
//                            1 - STM32 (Blue Pill)

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************
#define D_TEXT_SPEED 100      // скорость бегущего текста по умолчанию (мс)
#define D_EFFECT_SPEED 80     // скорость эффектов по умолчанию (мс)
#define D_GAME_SPEED 250      // скорость игр по умолчанию (мс)
#define D_GIF_SPEED 80        // скорость гифок (мс)
#define DEMO_GAME_SPEED 60    // скорость игр в демо режиме (мс)

boolean AUTOPLAY = 1;         // 0 выкл / 1 вкл автоматическую смену режимов (откл. можно со смартфона)
int AUTOPLAY_PERIOD = 10;     // время между авто сменой режимов (секунды)
#define IDLE_TIME 0          // время бездействия кнопок или Bluetooth (в секундах) после которого запускается автосмена режимов и демо в играх(0-откл(При первом запуске, он включится))

// о поддерживаемых цветах читай тут https://alexgyver.ru/gyvermatrixos-guide/
#define GLOBAL_COLOR_1 CRGB::Green    // основной цвет №1 для игр
#define GLOBAL_COLOR_2 CRGB::Orange   // основной цвет №2 для игр

#define SCORE_SIZE 0          // размер символов счёта в игре. 0 - маленький для 8х8 (шрифт 3х5), 1 - большой (шрифт 5х7)
#define FONT_TYPE 0           // (0 / 1) два вида маленького шрифта в выводе игрового счёта

// ************** ОТКЛЮЧЕНИЕ КОМПОНЕНТОВ СИСТЕМЫ (для экономии памяти) *************
// внимание! отключение модуля НЕ УБИРАЕТ его эффекты из списка воспроизведения!
// Это нужно сделать вручную во вкладке custom, удалив ненужные функции

#define USE_BUTTONS 0         // использовать физические кнопки управления играми (0 нет, 1 да)
#define BT_MODE 1             // использовать блютус (0 нет, 1 да)
#define USE_NOISE_EFFECTS 1   // крутые полноэкранные эффекты (0 нет, 1 да) СИЛЬНО ЖРУТ ПАМЯТЬ!!!11
#define USE_MODULE_EFFECTS 0  // крутые модульные эффекты (0 нет, 1 да) СИЛЬНО ЖРУТ ПАМЯТЬ!!!11
#define USE_ANIMATION 0       // анимация (0 нет, 1 да)
#define USE_FONTS 1           // использовать буквы (бегущая строка) (0 нет, 1 да)
#define USE_CLOCK 1           // использовать часы (0 нет, 1 да)
#define USE_RTC 0             // использовать RTC для часов

// игры
#define USE_SNAKE 0           // игра змейка (0 нет, 1 да)
#define USE_TETRIS 0          // игра тетрис (0 нет, 1 да)
#define USE_MAZE 0            // игра лабиринт (0 нет, 1 да)
#define USE_DINO 0            // игра дино (0 нет, 1 да)
#define USE_FLAPPY 0          // игра flappy bird (0 нет, 1 да)
#define USE_ARKAN 0           // игра арканоид (0 нет, 1 да)
#define USE_RUNNER 1          // игра бегалка (0 нет, 1 да)

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************
// Arduino (Nano, Mega)
#if (MCU_TYPE == 0)
#define LED_PIN 4           // пин ленты
#define BUTT_UP 3           // кнопка вверх
#define BUTT_DOWN 5         // кнопка вниз
#define BUTT_LEFT 2         // кнопка влево
#define BUTT_RIGHT 4        // кнопка вправо
#define BUTT_SET 7          // кнопка выбор/игра

// STM32 (BluePill) - плату выбирал STM32F103C
#elif (MCU_TYPE == 1)
#define LED_PIN PB12         // пин ленты
#define BUTT_UP PA1          // кнопка вверх
#define BUTT_DOWN PA3        // кнопка вниз
#define BUTT_LEFT PA0        // кнопка влево
#define BUTT_RIGHT PA2       // кнопка вправо
#define BUTT_SET PA4         // кнопка выбор/игра
#endif


#if(USE_ANIMATION == 1)
  #include "bitmap2.h"
#endif
// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

#define RUNNING_STRING 0
#define CLOCK_MODE 1
#define GAME_MODE 2
#define MADNESS_NOISE 3
#define CLOUD_NOISE 4
#define LAVA_NOISE 5
#define PLASMA_NOISE 6
#define RAINBOW_NOISE 7
#define RAINBOWSTRIPE_NOISE 8
#define ZEBRA_NOISE 9
#define FOREST_NOISE 10
#define OCEAN_NOISE 11
#define SNOW_ROUTINE 12
#define SPARKLES_ROUTINE 13
#define MATRIX_ROUTINE 14
#define STARFALL_ROUTINE 15
#define BALL_ROUTINE 16
#define BALLS_ROUTINE 17
#define RAINBOW_ROUTINE 18
#define RAINBOWDIAGONAL_ROUTINE 19
#define FIRE_ROUTINE 20
#define PATTERNS_ROUTINE 22
#define TLAND_ROUTINE 23
#define GAME_ROUTINE 24
#define IMAGE_MODE 21
