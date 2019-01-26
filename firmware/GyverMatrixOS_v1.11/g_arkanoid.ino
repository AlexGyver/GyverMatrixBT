// игра "arkanoid"

// **************** НАСТРОЙКИ ARKAN ****************
#define SHELF_LENGTH 5    // длина полки
#define VELOCITY 5        // скорость шара
#define BALL_SPEED 50     // период интегрирования

#define BLOCKS_H 4        // высота кучи блоков
#define LINE_NUM 8        // количество "линий" с блоками других уровней
#define LINE_MAX 4        // макс. длина линии

// цвета блоков по крутости
#define BLOCK_COLOR_1 CRGB::Aqua
#define BLOCK_COLOR_2 CRGB::Amethyst
#define BLOCK_COLOR_3 CRGB::Blue

#if (USE_ARKAN == 1)
int posX_ark;
int posY_ark;
int8_t velX_ark = 3;
int8_t velY_ark = (long)sqrt(sq(VELOCITY) - sq(velX_ark));
int8_t shelf_x = WIDTH / 2 - SHELF_LENGTH / 2;
byte shelfMAX = WIDTH - SHELF_LENGTH;
int arkScore;
int lastSpeed;

timerMinim popTimeout(500);
timerMinim shelfTimer(150);

void newGameArkan() {
  arkScore = 0;
  generateBlocks();
  shelf_x = WIDTH / 2 - SHELF_LENGTH / 2;
  posX_ark = WIDTH / 2 * 10;
  posY_ark = 15;
  velX_ark = random(1, 4);
  velY_ark = (long)sqrt(sq(VELOCITY) - sq(velX_ark));
}

void arkanoidRoutine() {
  if (loadingFlag) {
    FastLED.clear();
    loadingFlag = false;
    gamemodeFlag = true;
    modeCode = MC_GAME;
    newGameArkan();
    FastLED.show();
  }

  if (gameDemo) {
    if (shelfTimer.isReady()) {
      if (floor(posX_ark / 10) > shelf_x - SHELF_LENGTH / 2 - 1) buttons = 1;
      if (floor(posX_ark / 10) < shelf_x + SHELF_LENGTH / 2 + 1) buttons = 3;
    }
  }

  if (checkButtons()) {
    if (buttons == 3) {   // кнопка нажата
      shelfLeft();
    }
    if (buttons == 1) {   // кнопка нажата
      shelfRight();
    }
    buttons = 4;
  }

  if (lastSpeed != gameSpeed) {
    lastSpeed == gameSpeed;
    gameTimer.setInterval(gameSpeed / 2);
  }

  if (gameTimer.isReady()) {        // главный таймер игры
    drawPixelXY(posX_ark / 10, posY_ark / 10, CRGB::Black);
    posX_ark = posX_ark + velX_ark;
    posY_ark = posY_ark + velY_ark;
    int8_t posX_arkm = posX_ark / 10;
    int8_t posY_arkm = posY_ark / 10;
    if (abs(velY_ark) <= 2) {
      velX_ark = 3;
      velY_ark = (long)sqrt(sq(VELOCITY) - sq(velX_ark));
    }

    // отскок левый край
    if (posX_arkm < 0) {
      posX_ark = 0;
      velX_ark = -velX_ark;
    }

    // отскок правый край
    if (posX_arkm > WIDTH - 1) {
      posX_ark = (WIDTH - 1) * 10;
      velX_ark = -velX_ark;
    }

    // проверка на пробитие дна
    if (posY_ark < 9) {
      gameOverArkan();
      //posY_ark = 0;
      //velY_ark = -velY_ark;
    }

    // проверка на ударение с площадкой
    if (velY_ark < 0 && posY_ark > 10 && posY_ark <= 20 && posX_arkm >= shelf_x && posX_arkm < (shelf_x + SHELF_LENGTH)) {
      // таймаут, чтолбы исключить дрочку у полки
      if (popTimeout.isReady()) {

        // тут короч если длина полки больше двух, то её края "подкручивают"
        // шарик, т.е. при отскоке меняют скорость по оси Х
        if (SHELF_LENGTH > 2) {
          if (posX_arkm == shelf_x) {
            velX_ark -= 2;  // уменьшаем скорость по Х
            // расчёт скорости по У с учётом общего заданного вектора скорости
            velY_ark = (long)sqrt(sq(VELOCITY) - sq(velX_ark));
          } else if (posX_arkm == (shelf_x + SHELF_LENGTH - 1)) {
            velX_ark += 2;  // увеличиваем скорость по Х
            velY_ark = (long)sqrt(sq(VELOCITY) - sq(velX_ark));
          } else {
            velY_ark = -velY_ark;
          }
        } else {
          velY_ark = -velY_ark;
        }
      }
    }

    // пробитие верха
    if (posY_arkm > HEIGHT - 1) {
      posY_ark = (HEIGHT - 1) * 10;
      velY_ark = -velY_ark;
    }
    byte ballX = floor(posX_ark / 10);
    byte ballY = floor(posY_ark / 10);

    if (ballY > 2) {
      if (ballX < WIDTH - 1 && velX_ark > 0 && getPixColorXY(ballX + 1, ballY) != 0) {
        redrawBlock(ballX + 1, ballY);
        velX_ark = -velX_ark;
      }
      if (ballX > 1 && velX_ark < 0 && getPixColorXY(ballX - 1, ballY) != 0) {
        redrawBlock(ballX - 1, ballY);
        velX_ark = -velX_ark;
      }
      if (ballY < HEIGHT - 1 && velY_ark > 0 && getPixColorXY(ballX, ballY + 1) != 0) {
        redrawBlock(ballX, ballY + 1);
        velY_ark = -velY_ark;
      }
      if (velY_ark < 0 && getPixColorXY(ballX, ballY - 1) != 0) {
        redrawBlock(ballX, ballY - 1);
        velY_ark = -velY_ark;
      }
    }

    if (checkBlocks()) gameOverArkan();    

    drawPixelXY(ballX, ballY, GLOBAL_COLOR_1);
    for (byte i = shelf_x; i < shelf_x + SHELF_LENGTH; i++) {
      drawPixelXY(i, 0, GLOBAL_COLOR_2);
    }
    FastLED.show();
  }
}

boolean checkBlocks() {   // возвр ДА если блоков нет
  for (byte j = HEIGHT - 1; j > HEIGHT - 1 - BLOCKS_H; j--) {
    for (byte i = 0; i < WIDTH; i++) {
      if (getPixColorXY(i, j) != BLOCK_COLOR_1) return false;
      else if (getPixColorXY(i, j) != BLOCK_COLOR_2) return false;
      else if (getPixColorXY(i, j) != BLOCK_COLOR_3) return false;
    }
  }
  return true;
}

void redrawBlock(byte blockX, byte blockY) {
  arkScore++;
  if (getPixColorXY(blockX, blockY) == BLOCK_COLOR_1) drawPixelXY(blockX, blockY, 0);
  else if (getPixColorXY(blockX, blockY) == BLOCK_COLOR_2) drawPixelXY(blockX, blockY, BLOCK_COLOR_1);
  else if (getPixColorXY(blockX, blockY) == BLOCK_COLOR_3) drawPixelXY(blockX, blockY, BLOCK_COLOR_2);
}

void generateBlocks() {
  for (byte j = HEIGHT - 1; j > HEIGHT - 1 - BLOCKS_H; j--) {
    for (byte i = 0; i < WIDTH; i++) {
      drawPixelXY(i, j, BLOCK_COLOR_1);
    }
  }
  for (byte k = 0; k < LINE_NUM; k++) {
    byte newPosX = random(0, WIDTH - 1 - LINE_MAX);
    byte newPosY = random(HEIGHT - BLOCKS_H, HEIGHT);
    byte newColor = random(0, 3);
    for (byte i = newPosX; i < newPosX + LINE_MAX; i++) {
      switch (newColor) {
        case 0: drawPixelXY(i, newPosY, 0);
          break;
        case 1: drawPixelXY(i, newPosY, BLOCK_COLOR_2);
          break;
        case 2: drawPixelXY(i, newPosY, BLOCK_COLOR_3);
          break;
      }
    }
  }
}

void gameOverArkan() {
  displayScore(arkScore);
  delay(800);
  FastLED.clear();
  newGameArkan();
}

void shelfRight() {
  shelf_x++;            // прибавить координату полки
  if (shelf_x > shelfMAX) { // если полка пробила правый край
    shelf_x = shelfMAX;
  } else {
    drawPixelXY(shelf_x - 1, 0, CRGB::Black);   // стереть последнюю точку
    drawPixelXY(shelf_x + SHELF_LENGTH - 1, 0, GLOBAL_COLOR_2);  // нарисовать первую
    FastLED.show();
  }
}

void shelfLeft() {
  shelf_x--;
  if (shelf_x < 0) { // если полка пробила левый край
    shelf_x = 0;
  } else {
    drawPixelXY(shelf_x, 0, GLOBAL_COLOR_2);   // стереть последнюю точку
    drawPixelXY(shelf_x + SHELF_LENGTH, 0, CRGB::Black);  // нарисовать первую
    FastLED.show();
  }
}

#else
void arkanoidRoutine() {
  return;
}
#endif
