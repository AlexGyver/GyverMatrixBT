// эффекты

// **************** НАСТРОЙКИ ЭФФЕКТОВ ****************
// эффект "синусоиды" - ОТКЛЮЧЕН
#define WAVES_AMOUNT 2    // количество синусоид

// эффект "шарики"
#define BALLS_AMOUNT 3    // количество "шариков"
#define CLEAR_PATH 1      // очищать путь
#define TRACK_STEP 70     // длина хвоста шарика (чем больше цифра, тем хвост короче)

// эффект "квадратик"
#define BALL_SIZE 2       // размер шара

// эффект "огонь"
#define FSCALE 100+30       //масштаб
// эффект "кометы"
#define TAIL_STEP 100     // длина хвоста кометы
#define SATURATION 150    // насыщенность кометы (от 0 до 255) 0=Метель
#define STAR_DENSE 60     // количество (шанс появления) комет

// эффект "конфетти"
#define DENSE 3           // плотность конфетти
#define BRIGHT_STEP 70    // шаг уменьшения яркости

// эффект "снег"
#define SNOW_DENSE 10     // плотность снегопада

// --------------------- ДЛЯ РАЗРАБОТЧИКОВ ----------------------

// *********** "дыхание" яркостью ***********
boolean brightnessDirection;
void brightnessRoutine() {
  if (brightnessDirection) {
    breathBrightness += 2;
    if (breathBrightness > globalBrightness - 1) {
      brightnessDirection = false;
    }
  } else {
    breathBrightness -= 2;
    if (breathBrightness < 1) {
      brightnessDirection = true;
    }
  }
  FastLED.setBrightness(breathBrightness);
}

// *********** смена цвета активных светодиодов (рисунка) ***********
byte hue;
void colorsRoutine() {
  hue += 4;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (getPixColor(i) > 0) leds[i] = CHSV(hue, 255, 255);
  }
}

// *********** снегопад 2.0 ***********
void snowRoutine() {
  modeCode = 12;
  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, SNOW_DENSE) == 0))
      drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1, 0x000000);
  }
}

// ***************************** БЛУДНЫЙ КУБИК *****************************
int coordB[2];
int8_t vectorB[2];
CRGB ballColor;

void ballRoutine() {
  if (loadingFlag) {
    for (byte i = 0; i < 2; i++) {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255, 255);
    }
    modeCode = 16;
    loadingFlag = false;
  }
  for (byte i = 0; i < 2; i++) {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0) {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      ballColor = CHSV(random(0, 9) * 28, 255, 255);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0] > (WIDTH - BALL_SIZE) * 10) {
    coordB[0] = (WIDTH - BALL_SIZE) * 10;
    vectorB[0] = -vectorB[0];
    ballColor = CHSV(random(0, 9) * 28, 255, 255);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1] > (HEIGHT - BALL_SIZE) * 10) {
    coordB[1] = (HEIGHT - BALL_SIZE) * 10;
    vectorB[1] = -vectorB[1];
    ballColor = CHSV(random(0, 9) * 28, 255, 255);
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (byte i = 0; i < BALL_SIZE; i++)
    for (byte j = 0; j < BALL_SIZE; j++)
      leds[getPixelNumber(coordB[0] / 10 + i, coordB[1] / 10 + j)] = ballColor;
}

// *********** радуга заливка ***********
void rainbowRoutine() {
  modeCode = 18;
  hue += 3;
  for (byte i = 0; i < WIDTH; i++) {
    CHSV thisColor = CHSV((byte)(hue + i * float(255 / WIDTH)), 255, 255);
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, thisColor);   //leds[getPixelNumber(i, j)] = thisColor;
  }
}

// *********** радуга дигональная ***********
void rainbowDiagonalRoutine() {
  modeCode = 19;
  hue += 3;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      CHSV thisColor = CHSV((byte)(hue + (float)(WIDTH / HEIGHT * x + y) * (float)(255 / maxDim)), 255, 255);
      drawPixelXY(x, y, thisColor); //leds[getPixelNumber(i, j)] = thisColor;
    }
  }
}


// *********** радуга активных светодиодов (рисунка) ***********
void rainbowColorsRoutine() {
  hue++;
  for (byte i = 0; i < WIDTH; i++) {
    CHSV thisColor = CHSV((byte)(hue + i * float(255 / WIDTH)), 255, 255);
    for (byte j = 0; j < HEIGHT; j++)
      if (getPixColor(getPixelNumber(i, j)) > 0) drawPixelXY(i, j, thisColor);
  }
}


// ********************** огонь **********************
void fireRoutine() {
  modeCode = 20;
  //CRGBPalette16 myPal = firepal;



  uint32_t a = millis();
  for (byte i = 0U; i < WIDTH; i++) {
    for (float j = 0.; j < HEIGHT; j++) {
      //      if(curPalette!=palettes.at(10))
      drawPixelXY((WIDTH - 1) - i, (HEIGHT - 1) - j, ColorFromPalette(HeatColors_p, qsub8(inoise8(i * FSCALE, j * FSCALE + a, a / 255), abs8(j - (HEIGHT - 1)) * 255 / (HEIGHT - 1)), 255));
    }
  }
}
// **************** МАТРИЦА *****************
void matrixRoutine()
{ modeCode = 14;
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // обрабатываем нашу матрицу снизу вверх до второй сверху строчки
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
      uint32_t thisColor = getPixColorXY(x, y);                                              // берём цвет нашего пикселя
      uint32_t upperColor = getPixColorXY(x, y + 1U);                                        // берём цвет пикселя над нашим
      if (upperColor >= 0x900000 && random(7 * HEIGHT) != 0U)                  // если выше нас максимальная яркость, игнорим этот факт с некой вероятностью или опускаем цепочку ниже
        drawPixelXY(x, y, upperColor);
      else if (thisColor == 0U && random((0, 10) * HEIGHT) == 0U) // если наш пиксель ещё не горит, иногда зажигаем новые цепочки
        //else if (thisColor == 0U && random((100 - modes[currentMode].Scale) * HEIGHT*3) == 0U)  // для длинных хвостов
        drawPixelXY(x, y, 0x9bf800);
      else if (thisColor <= 0x050800)                                                        // если наш пиксель почти погас, стараемся сделать затухание медленней
      {
        if (thisColor >= 0x030000)
          drawPixelXY(x, y, 0x020300);
        else if (thisColor != 0U)
          drawPixelXY(x, y, 0U);
      }
      else if (thisColor >= 0x900000)                                                        // если наш пиксель максимальной яркости, резко снижаем яркость
        drawPixelXY(x, y, 0x558800);
      else
        drawPixelXY(x, y, thisColor - 0x0a1000);                                             // в остальных случаях снижаем яркость на 1 уровень
      //drawPixelXY(x, y, thisColor - 0x050800);                                             // для длинных хвостов
    }
    // аналогично обрабатываем верхний ряд пикселей матрицы
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1U);
    if (thisColor == 0U)                                                                     // если наш верхний пиксель не горит, заполняем его с вероятностью .Scale
    {
      if (random(0, 10) == 0U)
        drawPixelXY(x, HEIGHT - 1U, 0x9bf800);
    }
    else if (thisColor <= 0x050800)                                                          // если наш верхний пиксель почти погас, стараемся сделать затухание медленней
    {
      if (thisColor >= 0x030000)
        drawPixelXY(x, HEIGHT - 1U, 0x020300);
      else
        drawPixelXY(x, HEIGHT - 1U, 0U);
    }
    else if (thisColor >= 0x900000)                                                          // если наш верхний пиксель максимальной яркости, резко снижаем яркость
      drawPixelXY(x, HEIGHT - 1U, 0x558800);
    else
      drawPixelXY(x, HEIGHT - 1U, thisColor - 0x0a1000);                                     // в остальных случаях снижаем яркость на 1 уровень
    //drawPixelXY(x, HEIGHT - 1U, thisColor - 0x050800);                                     // для длинных хвостов
  }
}

// ********************************* ШАРИКИ *********************************
int coord[BALLS_AMOUNT][2];
int8_t vector[BALLS_AMOUNT][2];
CRGB ballColors[BALLS_AMOUNT];

void ballsRoutine() {
  if (loadingFlag) {
    modeCode = 17;
    loadingFlag = false;
    for (byte j = 0; j < BALLS_AMOUNT; j++) {
      int sign;

      // забиваем случайными данными
      coord[j][0] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0] = random(4, 15) * sign;
      coord[j][1] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1] = random(4, 15) * sign;
      ballColors[j] = CHSV(random(0, 9) * 28, 255, 255);
    }
  }



  fader(TRACK_STEP);


  // движение шариков
  for (byte j = 0; j < BALLS_AMOUNT; j++) {

    // движение шариков
    for (byte i = 0; i < 2; i++) {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0) {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0] > (WIDTH - 1) * 10) {
      coord[j][0] = (WIDTH - 1) * 10;
      vector[j][0] = -vector[j][0];
    }
    if (coord[j][1] > (HEIGHT - 1) * 10) {
      coord[j][1] = (HEIGHT - 1) * 10;
      vector[j][1] = -vector[j][1];
    }
    leds[getPixelNumber(coord[j][0] / 10, coord[j][1] / 10)] =  ballColors[j];
  }
}


// ******************************** СИНУСОИДЫ *******************************
#define DEG_TO_RAD 0.017453
int t;
byte w[WAVES_AMOUNT];
byte phi[WAVES_AMOUNT];
byte A[WAVES_AMOUNT];
byte waveColors[WAVES_AMOUNT];

void wavesRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = 22;
    for (byte j = 0; j < WAVES_AMOUNT; j++) {
      // забиваем случайными данными
      w[j] = random(17, 25);
      phi[j] = random(0, 360);
      A[j] = HEIGHT / 2 * random(4, 11) / 10;
      waveColors[j] = random(0, 9);
    }
  }
  // сдвигаем все пиксели вправо
  for (int i = WIDTH - 1; i > 0; i--)
    for (int j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, getPixColorXY(i - 1, j));

  // увеличиваем "угол"
  t++;
  if (t > 360) t = 0;

  // заливаем чёрным левую линию
  for (byte i = 0; i < HEIGHT; i++) {
    drawPixelXY(0, i, 0x000000);
  }

  // генерируем позицию точки через синус
  for (byte j = 0; j < WAVES_AMOUNT; j++) {
    waveColors[j] ++;
    float value = HEIGHT / 2 + (float)A[j] * sin((float)w[j] * t * DEG_TO_RAD + (float)phi[j] * DEG_TO_RAD);
    drawPixelXY(0, (uint8_t)value,CHSV(waveColors[j],255,255));
  }
}

// функция плавного угасания цвета для всех пикселей
void fader(byte step) {
  for (byte i = 0; i < WIDTH; i++) {
    for (byte j = 0; j < HEIGHT; j++) {
      fadePixel(i, j, step);
    }
  }
}
void fadePixel(byte i, byte j, byte step) {     // новый фейдер
  int pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0) return;

  if (leds[pixelNum].r >= 30 ||
      leds[pixelNum].g >= 30 ||
      leds[pixelNum].b >= 30) {
    leds[pixelNum].fadeToBlackBy(step);
  } else {
    leds[pixelNum] = 0;
  }
}

/*
  void fadePixel(byte i, byte j, byte step) {     // старый фейдер
  // измеряяем цвет текущего пикселя
  uint32_t thisColor = getPixColorXY(i, j);

  // если 0, то пропускаем действия и переходим к следующему
  if (thisColor == 0) return;

  // разбиваем цвет на составляющие RGB
  byte rgb[3];
  rgb[0] = (thisColor >> 16) & 0xff;
  rgb[1] = (thisColor >> 8) & 0xff;
  rgb[2] = thisColor & 0xff;

  // ищем максимум
  byte maximum = max(max(rgb[0], rgb[1]), rgb[2]);
  float coef = 0;

  // если есть возможность уменьшить
  if (maximum >= step)
    // уменьшаем и находим коэффициент уменьшения
    coef = (float)(maximum - step) / maximum;

  // далее все цвета умножаем на этот коэффициент
  for (byte i = 0; i < 3; i++) {
    if (rgb[i] > 0) rgb[i] = (float)rgb[i] * coef;
    else rgb[i] = 0;
  }
  leds[getPixelNumber(i, j)] = CRGB(rgb[0], rgb[1], rgb[2]);
  }
*/

// ********************* ЗВЕЗДОПАД ******************
void starfallRoutine() {
  modeCode = 15;
  // заполняем головами комет левую и верхнюю линию
  for (byte i = HEIGHT / 2; i < HEIGHT; i++) {
    if (getPixColorXY(0, i) == 0
        && (random(0, STAR_DENSE) == 0)
        && getPixColorXY(0, i + 1) == 0
        && getPixColorXY(0, i - 1) == 0)
      leds[getPixelNumber(0, i)] = CHSV(random(0, 200), SATURATION, 255);
  }
  for (byte i = 0; i < WIDTH / 2; i++) {
    if (getPixColorXY(i, HEIGHT - 1) == 0
        && (random(0, STAR_DENSE) == 0)
        && getPixColorXY(i + 1, HEIGHT - 1) == 0
        && getPixColorXY(i - 1, HEIGHT - 1) == 0)
      leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(random(0, 200), SATURATION, 255);
  }

  // сдвигаем по диагонали
  for (byte y = 0; y < HEIGHT - 1; y++) {
    for (byte x = WIDTH - 1; x > 0; x--) {
      drawPixelXY(x, y, getPixColorXY(x - 1, y + 1));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (byte i = HEIGHT / 2; i < HEIGHT; i++) {
    fadePixel(0, i, TAIL_STEP);
  }
  for (byte i = 0; i < WIDTH / 2; i++) {
    fadePixel(i, HEIGHT - 1, TAIL_STEP);
  }
}


// рандомные гаснущие вспышки
void sparklesRoutine() {
  modeCode = 13;
  for (byte i = 0; i < DENSE; i++) {
    byte x = random(0, WIDTH);
    byte y = random(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
      leds[XY(x, y)] = CHSV(random(0, 255), 255, 255);
  }
  fader(BRIGHT_STEP);
}
