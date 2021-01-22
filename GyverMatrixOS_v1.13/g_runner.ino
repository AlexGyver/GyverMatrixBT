// игра "Бегалка"

// **************** НАСТРОЙКИ БЕГАЛКИ ****************
#define DENCE 210 //плотность блоков(меньше число - больше шанса)
#if (USE_RUNNER == 1)
byte Gpos=WIDTH/2;
int gScore;

void newGame() {
  gScore = 0;
  FastLED.clear();
}

void runRoutine() {
  if (loadingFlag) {
    FastLED.clear();
    loadingFlag = false;
    gamemodeFlag = true;
    modeCode = 24;
    newGame();
    FastLED.show();
  }

  if (gameDemo) {
      if (getPixColorXY(Gpos, 1) == GLOBAL_COLOR_1){
      if (getPixColorXY(Gpos-1, 1)== GLOBAL_COLOR_1) buttons = 3;
      else buttons = 1;}
  }

  if (checkButtons()) {
    if (buttons == 3) {   // кнопка нажата
      Gpos--;
      if(Gpos<0)Gpos=0;
    }
    if (buttons == 1) {   // кнопка нажата
      Gpos++;
      if(Gpos>WIDTH-1)Gpos=WIDTH-1;
    }
    buttons = 4;
  }

  if (gameTimer.isReady()) {        // главный таймер игры
    gScore ++;
    if(getPixColorXY(Gpos, 1) == GLOBAL_COLOR_1)gameOver();//если перед ним блок, ему 3.14зда
    shiftDown();//здвиг всего вниз
    drawPixelXY(Gpos,0,GLOBAL_COLOR_2);
    for (byte x = 0; x < WIDTH; x++) {
      // заполняем случайно верхнюю строку
      // а также не даём двум блокам по вертикали вместе быть
      if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0,DENSE) == 0))
        drawPixelXY(x, HEIGHT - 1, GLOBAL_COLOR_1);
      else
        drawPixelXY(x, HEIGHT - 1, 0x000000);
    }
    FastLED.show();
  }
}




void gameOver() {
  FastLED.clear();
  displayScore(gScore);
  delay(800);
  newGame();
}


#else
void gameRoutine() {
  return;
}
#endif
