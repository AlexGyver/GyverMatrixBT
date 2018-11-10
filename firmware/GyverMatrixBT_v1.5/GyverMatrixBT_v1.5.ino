/*
  Скетч к проекту "Bluetooth управляемая RGB матрица"
  Гайд по постройке матрицы: https://alexgyver.ru/matrix_guide/
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverMatrixBT/
  Исходники на GitHub: https://github.com/AlexGyver/GyverMatrixBT/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/
*/

// Версия прошивки 1.5, совместима с приложением версии 1.5

// **************** НАСТРОЙКИ ****************
// чем больше матрица и количество частиц (эффекты), тем выше шанс того, что всё зависнет!
// Данный код стабильно работает на матрице 16х16 (256 диодов). Больше - на ваш страх и риск

#define USE_BUTTONS 0       // использовать физические кнопки управления играми (0 нет, 1 да)
#define BUTT_UP 3           // кнопка вверх
#define BUTT_DOWN 5         // кнопка вниз
#define BUTT_LEFT 2         // кнопка влево
#define BUTT_RIGHT 4        // кнопка вправо

#define LED_PIN 6           // пин ленты
#define BRIGHTNESS 60       // стандартная маскимальная яркость (0-255)

#define WIDTH 16            // ширина матрицы
#define HEIGHT 16           // высота матрицы

#define MATRIX_TYPE 0       // тип матрицы: 0 - зигзаг, 1 - последовательная
#define CONNECTION_ANGLE 0  // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0   // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

#define SCORE_SIZE 0        // размер символов счёта в игре. 0 - маленький для 8х8 (шрифт 3х5), 1 - большой (шрифт 5х8)
#define FONT_TYPE 1			    // (0 / 1) два вида маленького шрифта

#define GLOBAL_COLOR_1 CRGB::Green    // основной цвет №1 для игр
#define GLOBAL_COLOR_2 CRGB::Orange   // основной цвет №2 для игр

// поддерживает 150 цветов, названия можно посмотреть
// в библиотеке FastLED, файл pixeltypes.h, строка 590
// также цвет можно задавать в формате HEX (0XFF25AB)
// в любом онлайн колорпикере или в фотошопе!

// **************** ДЛЯ РАЗРАБОТЧИКОВ ****************
#define DEBUG 0
#define NUM_LEDS WIDTH * HEIGHT

#include "FastLED.h"
CRGB leds[NUM_LEDS];

#define PARSE_AMOUNT 4    // максимальное количество значений в массиве, который хотим получить
#define header '$'        // стартовый символ
#define divider ' '       // разделительный символ
#define ending ';'        // завершающий символ

byte intData[PARSE_AMOUNT];     // массив численных значений после парсинга
uint32_t prevColor;
boolean recievedFlag;
String runningText = "";
byte prevY = 0;
byte prevX = 0;

boolean runningFlag;
boolean loadingFlag;
boolean gameFlag;
boolean effectsFlag;
byte game;
byte effect;
boolean buttons[4];
byte globalBrightness = BRIGHTNESS;
boolean parseStarted;
byte globalSpeed = 200;
uint32_t globalColor = 0x00ff00;   // цвет при запуске зелёный
byte breathBrightness;

#include "GyverTimer.h"
#include "fonts.h"

GTimer_ms effectTimer(100);
GTimer_ms gameTimer(800);
GTimer_ms scrollTimer(100);

#if (USE_BUTTONS == 1)
#include "GyverButton.h"
GButton bt_left(BUTT_RIGHT);
GButton bt_right(BUTT_LEFT);
GButton bt_up(BUTT_UP);
GButton bt_down(BUTT_DOWN);
#endif

void setup() {
  Serial.begin(9600);

  // настройки ленты
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  randomSeed(analogRead(0) + analogRead(1));    // пинаем генератор случайных чисел
}

void loop() {
  parsing();       				            // принимаем данные

  if (!parseStarted) {                // на время принятия данных матрицу не обновляем!
    
    if (runningFlag) fillString(runningText);   // бегущая строка
    if (gameFlag) games();                      // игры
    if (effectsFlag) effects();                 // эффекты
  }
}

boolean checkButtons() {
#if (USE_BUTTONS == 1)
  bt_left.tick();
  bt_right.tick();
  bt_up.tick();
  bt_down.tick();

  if (bt_left.isPress()) {
    buttons[3] = 1;
    return true;
  }
  if (bt_right.isPress()) {
    buttons[1] = 1;
    return true;
  }
  if (bt_up.isPress()) {
    buttons[0] = 1;
    return true;
  }
  if (bt_down.isPress()) {
    buttons[2] = 1;
    return true;
  }
#endif

  for (byte i = 0; i < 4; i++)
    if (buttons[i] != 0) return true;

  return false;
}

// блок эффектов, работают по общему таймеру
void effects() {
  if (effectTimer.isReady()) {
    switch (effect) {
      case 0: brightnessRoutine();
        break;
      case 1: colorsRoutine();
        break;
      case 2: snowRoutine();
        break;
      case 3: ballRoutine();
        break;
      case 4: rainbowRoutine();
        break;
      case 5: rainbowColorsRoutine();
        break;
      case 6: fireRoutine();
        break;
      case 7: matrixRoutine();
        break;
      case 8: ballsRoutine();
        break;
      case 9: wavesRoutine();
        break;
    }
    FastLED.show();
  }
}

// блок игр
void games() {
  switch (game) {
    case 0:
      if (loadingFlag) {
        FastLED.clear();
        loadingFlag = false;
        newGameSnake();
      }
      snakeRoutine();
      break;
    case 1:
      if (loadingFlag) {
        FastLED.clear();
        loadingFlag = false;
        newGameTetris();
      }
      tetrisRoutine();
      break;
    case 2:
      if (loadingFlag) {
        FastLED.clear();
        loadingFlag = false;
        newGameMaze();
      }
      mazeRoutine();
      break;
  }
}

byte index;
String string_convert = "";
enum modes {NORMAL, COLOR, TEXT} parseMode;

// ********************* ПРИНИМАЕМ ДАННЫЕ **********************
void parsing() {
  // ****************** ОБРАБОТКА *****************
  /*
    Протокол связи, посылка начинается с режима. Режимы:
    0 - отправка цвета $0 colorHEX;
    1 - отправка координат точки $1 X Y;
    2 - заливка - $2;
    3 - очистка - $3;
    4 - яркость - $4 value;
    5 - картинка $5 colorHEX X Y;
    6 - текст $6 some text
    7 - управление текстом: $7 1; пуск, $7 0; стоп
    8 - эффект
      - $8 0 номерЭффекта;
      - $8 1 старт/стоп;
    9 - игра
    10 - кнопка вверх
    11 - кнопка вправо
    12 - кнопка вниз
    13 - кнопка влево
    14 - пауза в игре
    15 - скорость $8 скорость;
  */
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;

    switch (intData[0]) {
      case 1:
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 2:
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 3:
        FastLED.clear();
        FastLED.show();
        break;
      case 4:
        globalBrightness = intData[1];
        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
        FastLED.show();
        break;
      case 5:
        drawPixelXY(intData[2], intData[3], gammaCorrection(globalColor));
        // делаем обновление матрицы каждую строчку, чтобы реже обновляться
        // и не пропускать пакеты данных (потому что отправка на большую матрицу занимает много времени)
        if (prevY != intData[3] || ( (intData[3] == 0) && (intData[2] == WIDTH - 1) ) ) {
          prevY = intData[3];
          FastLED.show();
        }
        break;
      case 6:
        loadingFlag = true;
        // строка принимается в переменную runningText
        break;
      case 7:
        if (intData[1] == 1) runningFlag = true;
        if (intData[1] == 0) runningFlag = false;
        break;
      case 8:
        if (intData[1] == 0) {
          effect = intData[2];
          gameFlag = false;
          loadingFlag = true;
          breathBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);    // возвращаем яркость
          globalSpeed = intData[3];
          gameTimer.setInterval(globalSpeed * 4);
        }
        else if (intData[1] == 1) effectsFlag = !effectsFlag;
        break;
      case 9:
        loadingFlag = true;
        effectsFlag = false;
        gameFlag = true;
        game = intData[1];
        globalSpeed = intData[2];
        gameTimer.setInterval(globalSpeed * 4);
        break;
      case 10:
        buttons[0] = 1;
        break;
      case 11:
        buttons[1] = 1;
        break;
      case 12:
        buttons[2] = 1;
        break;
      case 13:
        buttons[3] = 1;
        break;
      case 14:
        gameFlag = !gameFlag;
        break;
      case 15: globalSpeed = intData[1];
        if (gameFlag) gameTimer.setInterval(globalSpeed * 4);   // для игр скорость нужно меньше!
        if (effectsFlag) effectTimer.setInterval(globalSpeed);
        if (runningFlag) scrollTimer.setInterval(globalSpeed);
        break;
    }
  }

  // ****************** ПАРСИНГ *****************
  if (Serial.available() > 0) {
    char incomingByte;
    if (parseMode == TEXT) {     // если нужно принять строку
      runningText = Serial.readString();  // принимаем всю
      incomingByte = ending;              // сразу завершаем парс
      parseMode = NORMAL;
    } else {
      incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ
    }
    if (parseStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != divider && incomingByte != ending) {   // если это не пробел И не конец
        string_convert += incomingByte;       // складываем в строку
      } else {                                // если это пробел или ; конец пакета
        if (index == 0) {
          byte thisMode = string_convert.toInt();
          if (thisMode == 0 || thisMode == 5) parseMode = COLOR;    // передача цвета (в отдельную переменную)
          else if (thisMode == 6) parseMode = TEXT;
          else parseMode = NORMAL;
          //if (thisMode != 7 || thisMode != 0) runningFlag = false;
        }

        if (index == 1) {       // для второго (с нуля) символа в посылке
          if (parseMode == NORMAL) intData[index] = string_convert.toInt();             // преобразуем строку в int и кладём в массив}
          if (parseMode == COLOR) globalColor = strtol(&string_convert[0], NULL, 16);     // преобразуем строку HEX в цифру
        } else {
          intData[index] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
        }
        string_convert = "";                  // очищаем строку
        index++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == header) {             // если это $
      parseStarted = true;                      // поднимаем флаг, что можно парсить
      index = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == ending) {             // если таки приняли ; - конец парсинга
      parseMode == NORMAL;
      parseStarted = false;                     // сброс
      recievedFlag = true;                    // флаг на принятие
    }
  }
}

// ******************** СЛУЖЕБНЫЕ ФУНКЦИИ ********************
// отображаем счёт
void displayScore(byte score) {
#if (SCORE_SIZE == 0)
  if (score > 9) drawDigit3x5(score / 10, WIDTH / 2 - 4, HEIGHT / 2 - 3, GLOBAL_COLOR_1);
  drawDigit3x5(score % 10, WIDTH / 2, HEIGHT / 2 - 3, GLOBAL_COLOR_2);
#elif (SCORE_SIZE == 1)
  // получаем количество разрядов в числе
  byte digitAm;
  if (score > 10) digitAm = 1;
  else digitAm = 0;

  // для количества разрядов
  for (byte dig = 0; dig <= digitAm; dig++) {
    byte offsetX = WIDTH / 2;         // горизонт по центру
    byte offsetY = HEIGHT / 2 - 4;    // высота по центру
    if (dig) offsetX -= 5;            // это десятки, их рисуем левее

    for (byte x = 0; x < 5; x++) {    // цифра состоит из 5 столбцов
      byte thisByte;
      if (!dig) thisByte = pgm_read_byte(&(fontHEX[score % 10 + 16][x])); // получаем единицы счёта
      else thisByte = pgm_read_byte(&(fontHEX[score / 10 + 16][x]));     // получаем десятки счёта
      for (byte y = 0; y < 8; y++) {              // цифра состоит из 8 строк
        boolean thisBit = thisByte & (1 << (7 - y));    // получаем "вкл или выкл пиксель"
        if (thisBit) {
          if (dig) drawPixelXY(x + offsetX, y + offsetY, GLOBAL_COLOR_1);
          else drawPixelXY(x + offsetX, y + offsetY, GLOBAL_COLOR_2);
        }
      }
    }
  }
#endif
  FastLED.show();
}

void drawDigit3x5(byte digit, byte X, byte Y, uint32_t color) {
  if (digit > 9) return;
  for (byte x = 0; x < 3; x++) {
    byte thisByte;
    if (FONT_TYPE) thisByte = pgm_read_byte(&(font3x5[digit][x]));		// первый шрифт
    else thisByte = pgm_read_byte(&(font3x5_s[digit][x]));				// второй
    for (byte y = 0; y < 5; y++) {
      if (x + X > WIDTH || y + Y > HEIGHT) continue;
      if (thisByte & (1 << y)) drawPixelXY(x + X, y + Y, color);
    }
  }
}
