GTimer_ms gifTimer(D_GIF_SPEED);

// подключаем внешние файлы с картинками
//#include "bitmap2.h"

/*
   Эффекты:
    sparklesRoutine();  // случайные цветные гаснущие вспышки
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
    fillString("Ваш текст", цвет);    // цвет вида 0x00ff25 или CRGB::Red и проч. цвета
    fillString("Ваш текст", 1);       // радужный перелив текста
    fillString("Ваш текст", 2);       // каждая буква случайным цветом!

  Рисунки и анимации:
    loadImage(<название массива>);     // основная функция вывода картинки
    imageRoutine1();                   // пример использования
    animation1();                      // пример анимации

*/

// ************** СВОЙ СПИСОК РЕЖИМОВ **************
// количество кастомных режимов (которые сами переключаются или кнопкой)
#define MODES_AMOUNT 15

void customModes() {
  switch (thisMode) {
    case 0: fillString("МАТРИЦА НА ОКНЕ", 1);
      break;
    case 1: snowRoutine();
      break;
    case 2: sparklesRoutine();
      break;
    case 3: matrixRoutine();
      break;
    case 4: starfallRoutine();
      break;
    case 5: ballRoutine();
      break;
    case 6: ballsRoutine();
      break;
    case 7: wavesRoutine();
      break;
    case 8: rainbowRoutine();
      break;
    case 9: fireRoutine();
      break;
    case 10: snakeRoutine();
      break;
    case 11: tetrisRoutine();
      break;
    case 12: mazeRoutine();
      break;
    case 13: fillString("С НОВЫМ ГОДОМ!", CRGB::Blue);
      break;
    case 14: fillString("2к19 ЛОЛ КЕК", 2);
      break;
  }
  FastLED.show();
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
void customRoutine() {
  if (!BTcontrol) customModes();                    // режимы крутятся, пиксели мутятся

  if (idleState) {
    if (autoplayTimer.isReady() && AUTOPLAY) {    // таймер смены режима
      thisMode++;
      if (thisMode >= MODES_AMOUNT) thisMode = 0;
      loadingFlag = true;
      FastLED.clear();
    }
  } else {
    if (idleTimer.isReady()) {      // таймер холостого режима
      idleState = true;
      autoplayTimer.reset();
      gameDemo = true;

      gameSpeed = DEMO_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);

      loadingFlag = true;
      BTcontrol = false;
      FastLED.clear();
    }
  }
}
