GTimer_ms autoplayTimer(AUTOPLAY_PERIOD * 1000);
byte thisMode = 0;

#define MODES_AMOUNT 11     // количество кастомных режимов (которые сами переключаются)

/*
   Эффекты:
    snowRoutine();      // снег
    matrixRoutine();    // "матрица"
    starfallRoutine();  // звездопад (кометы)
    ballRoutine();      // квадратик
    ballsRoutine();     // шарики
    wavesRoutine();     // синусоиды
    rainbowRoutine();   // радуга во всю матрицу
    fireRoutine();      // огонь

  Игры:
    snakeRoutine();     // змейка
    tetrisRoutine();    // тетрис
    mazeRoutine();      // лабиринт

  Бегущая строка:
    fillString("Ваш текст");

*/

void customModes() {
  switch (thisMode) {
    case 0: snowRoutine();
      break;
    case 1: matrixRoutine();
      break;
    case 2: starfallRoutine();
      break;
    case 3: ballRoutine();
      break;
    case 4: ballsRoutine();
      break;
    case 5: wavesRoutine();
      break;
    case 6: rainbowRoutine();
      break;
    case 7: fireRoutine();
      break;
    case 8: snakeRoutine();
      break;
    case 9: tetrisRoutine();
      break;
    case 10: mazeRoutine();
      break;
    case 11: fillString("Ваш текст");
      break;
  }
  FastLED.show();
}

void customRoutine() {
  if (!BTcontrol) customModes();    // режимы крутятся, пиксели мутятся. Если контроль не с блютус

  if (idleState) {
    if (autoplayTimer.isReady()) {    // таймер смены режима
      thisMode++;
      if (thisMode >= MODES_AMOUNT) thisMode = 0;
      loadingFlag = true;
    }
  } else {
    if (idleTimer.isReady()) {      // таймер холостого режима
      idleState = true;
      autoplayTimer.reset();
      gameDemo = true;
      BTcontrol = false;    // даём добро на запуск своего набор режимов
    }
  }
}
