// свой список режимов

// ************************ НАСТРОЙКИ ************************
#define SMOOTH_CHANGE 1     // плавная смена режимов через чёрный
#define SHOW_FULL_TEXT 1    // не переключать режим, пока текст не покажется весь

// подключаем внешние файлы с картинками
//#include "bitmap2.h"


/*
   Режимы:
    clockRoutine();       // часы на чёрном фоне

   Эффекты:
    sparklesRoutine();    // случайные цветные гаснущие вспышки
    snowRoutine();        // снег
    matrixRoutine();      // "матрица"
    starfallRoutine();    // звездопад (кометы)
    ballRoutine();        // квадратик
    ballsRoutine();       // шарики
    rainbowRoutine();     // радуга во всю матрицу горизонтальная
    rainbowDiagonalRoutine();   // радуга во всю матрицу диагональная
    fireRoutine();        // огонь

  Крутые эффекты "шума":
    madnessNoise();       // цветной шум во всю матрицу
    cloudNoise();         // облака
    lavaNoise();          // лава
    plasmaNoise();        // плазма
    rainbowNoise();       // радужные переливы
    rainbowStripeNoise(); // полосатые радужные переливы
    zebraNoise();         // зебра
    forestNoise();        // шумящий лес
    oceanNoise();         // морская вода

  Игры:
    snakeRoutine();     // змейка
    tetrisRoutine();    // тетрис
    mazeRoutine();      // лабиринт
    runnerRoutine();    // бегалка прыгалка
    flappyRoutine();    // flappy bird
    arkanoidRoutine();  // арканоид

  Бегущая строка:
    fillString("Ваш текст", цвет);    // цвет вида 0x00ff25 или CRGB::Red и проч. цвета
    fillString("Ваш текст", 1);       // радужный перелив текста
    fillString("Ваш текст", 2);       // каждая буква случайным цветом!

  Рисунки и анимации:
    loadImage(<название массива>);    // основная функция вывода картинки
    imageRoutine1();                  // пример использования
    animation1();                     // пример анимации

*/

// ************************* СВОЙ СПИСОК РЕЖИМОВ ************************
// количество кастомных режимов (которые сами переключаются или кнопкой)
#define MODES_AMOUNT 28

void customModes() {
  switch (thisMode) {
    case 0: fillString("КРАСНЫЙ", CRGB::Red);
      break;
    case 1: fillString("РАДУГА", 1);
      break;
    case 2: fillString("RGB LED", 2);
      break;
    case 3: madnessNoise();
      break;
    case 4: cloudNoise();
      break;
    case 5: lavaNoise();
      break;
    case 6: plasmaNoise();
      break;
    case 7: rainbowNoise();
      break;
    case 8: rainbowStripeNoise();
      break;
    case 9: zebraNoise();
      break;
    case 10: forestNoise();
      break;
    case 11: oceanNoise();
      break;
    case 12: snowRoutine();
      break;
    case 13: sparklesRoutine();
      break;
    case 14: matrixRoutine();
      break;
    case 15: starfallRoutine();
      break;
    case 16: ballRoutine();
      break;
    case 17: ballsRoutine();
      break;
    case 18: rainbowRoutine();
      break;
    case 19: rainbowDiagonalRoutine();
      break;
    case 20: fireRoutine();
      break;
    case 21: snakeRoutine();
      break;
    case 22: tetrisRoutine();
      break;
    case 23: mazeRoutine();
      break;
    case 24: runnerRoutine();
      break;
    case 25: flappyRoutine();
      break;
    case 26: arkanoidRoutine();
      break;
    case 27: clockRoutine();
      break;
  }

}

// функция загрузки картинки в матрицу. должна быть здесь, иначе не работает =)
void loadImage(uint16_t (*frame)[WIDTH]) {
  for (byte i = 0; i < WIDTH; i++)
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, gammaCorrection(expandColor((pgm_read_word(&(frame[HEIGHT - j - 1][i]))))));
  // да, тут происходит лютенький п@здец, а именно:
  // 1) pgm_read_word - восстанавливаем из PROGMEM (флэш памяти) цвет пикселя в 16 битном формате по его координатам
  // 2) expandColor - расширяем цвет до 24 бит (спасибо adafruit)
  // 3) gammaCorrection - проводим коррекцию цвета для более корректного отображения
}
timerMinim gifTimer(D_GIF_SPEED);

// ********************** ПРИМЕРЫ ВЫВОДА КАРТИНОК ***********************

// Внимание! Если размер матрицы не совпадает с исходным размером матрицы в скетче
// (если вы только что  его скачали), то нужно удалить/закомментировать данные функции!
//
/*
  // показать картинку
  void imageRoutine1() {
  if (loadingFlag) {
    loadingFlag = false;
    loadImage(frame00);
  }
  }

  void animation1() {
  if (gifTimer.isReady()) {
    frameNum++;
    if (frameNum >= 4) frameNum = 0;
    switch (frameNum) {
      case 0: loadImage(frame00);
        break;
      case 1: loadImage(frame01);
        break;
      case 2: loadImage(frame02);
        break;
      case 3: loadImage(frame03);
        break;
    }
  }
  }
*/

// ********************* ОСНОВНОЙ ЦИКЛ РЕЖИМОВ *******************
#if (SMOOTH_CHANGE == 1)
byte fadeMode = 4;
boolean modeDir;
#endif

void nextMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = true;
#else
  nextModeHandler();
#endif
}
void prevMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = false;
#else
  prevModeHandler();
#endif
}
void nextModeHandler() {
  thisMode++;
  if (thisMode >= MODES_AMOUNT) thisMode = 0;
  loadingFlag = true;
  gamemodeFlag = false;
  FastLED.clear();
  FastLED.show();
}
void prevModeHandler() {
  thisMode--;
  if (thisMode < 0) thisMode = MODES_AMOUNT - 1;
  loadingFlag = true;
  gamemodeFlag = false;
  FastLED.clear();
  FastLED.show();
}

int fadeBrightness;
#if (SMOOTH_CHANGE == 1)
void modeFader() {
  if (fadeMode == 0) {
    fadeMode = 1;
  } else if (fadeMode == 1) {
    if (changeTimer.isReady()) {
      fadeBrightness -= 40;
      if (fadeBrightness < 0) {
        fadeBrightness = 0;
        fadeMode = 2;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  } else if (fadeMode == 2) {
    fadeMode = 3;
    if (modeDir) nextModeHandler();
    else prevModeHandler();
  } else if (fadeMode == 3) {
    if (changeTimer.isReady()) {
      fadeBrightness += 40;
      if (fadeBrightness > globalBrightness) {
        fadeBrightness = globalBrightness;
        fadeMode = 4;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  }
}
#endif

boolean loadFlag2;
void customRoutine() {
  if (!BTcontrol) {
    if (!gamemodeFlag) {
      if (effectTimer.isReady()) {
        if (!loadingFlag && !gamemodeFlag && needUnwrap() && modeCode != 0) clockOverlayUnwrap(CLOCK_X, CLOCK_Y);
        if (loadingFlag) loadFlag2 = true;

        customModes();                // режимы крутятся, пиксели мутятся

        if (!gamemodeFlag && modeCode != 0) clockOverlayWrap(CLOCK_X, CLOCK_Y);
#if (OVERLAY_CLOCK == 1 && USE_CLOCK == 1)
        if (loadFlag2) {
          setOverlayColors();
          loadFlag2 = false;
        }
#endif
        loadingFlag = false;
        FastLED.show();
      }
    } else {
      customModes();
    }
    btnsModeChange();
#if (SMOOTH_CHANGE == 1)
    modeFader();
#endif
  }

  if (idleState) {
    if (millis() - autoplayTimer > autoplayTime && AUTOPLAY) {    // таймер смены режима
      if (modeCode == 0 && SHOW_FULL_TEXT) {    // режим текста
        if (fullTextFlag) {
          autoplayTimer = millis();
          nextMode();
        }
      } else {
        autoplayTimer = millis();
        nextMode();
      }
    }
  } else {
    if (idleTimer.isReady()) {      // таймер холостого режима
      idleState = true;
      autoplayTimer = millis();
      gameDemo = true;

      gameSpeed = DEMO_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);

      loadingFlag = true;
      BTcontrol = false;
      FastLED.clear();
      FastLED.show();
    }
  }
}


void btnsModeChange() {
#if (USE_BUTTONS == 1)
  if (bt_set.clicked(&commonBtnTimer)) {
    if (gamemodeFlag) gameDemo = !gameDemo;
    if (gameDemo) {
      gameSpeed = DEMO_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);
      AUTOPLAY = true;
    } else {
      gameSpeed = D_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);
      AUTOPLAY = false;
    }
  }
  if (bt_set.holded(&commonBtnTimer)) {
    mazeMode = !mazeMode;
  }
  if (gameDemo) {
    if (bt_right.clicked(&commonBtnTimer)) {
      autoplayTimer = millis();
      nextMode();
    }

    if (bt_left.clicked(&commonBtnTimer)) {
      autoplayTimer = millis();
      prevMode();
    }

    if (bt_up.clicked(&commonBtnTimer)) {
      AUTOPLAY = true;
      autoplayTimer = millis();
    }
    if (bt_down.clicked(&commonBtnTimer)) {
      AUTOPLAY = false;
    }

    if (bt_right.holded(&commonBtnTimer))
      if (changeTimer.isReady()) {
        effects_speed -= 2;
        if (effects_speed < 30) effects_speed = 30;
        effectTimer.setInterval(effects_speed);
      }
    if (bt_left.holded(&commonBtnTimer))
      if (changeTimer.isReady()) {
        effects_speed += 2;
        if (effects_speed > 300) effects_speed = 300;
        effectTimer.setInterval(effects_speed);
      }
    if (bt_up.holded(&commonBtnTimer))
      if (changeTimer.isReady()) {
        globalBrightness += 2;
        if (globalBrightness > 255) globalBrightness = 255;
        fadeBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
      }
    if (bt_down.holded(&commonBtnTimer))
      if (changeTimer.isReady()) {
        globalBrightness -= 2;
        if (globalBrightness < 0) globalBrightness = 0;
        fadeBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
      }
  }
#endif
}
