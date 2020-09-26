// эффекты

// **************** НАСТРОЙКИ ЭФФЕКТОВ ****************
// эффект "шарики"
#define BALLS_AMOUNT 3    // количество "шариков"
#define CLEAR_PATH 1      // очищать путь
#define TRACK_STEP 70     // длина хвоста шарика (чем больше цифра, тем хвост короче)

// эффект "квадратик"
#define BALL_SIZE 3       // размер шара

// эффект "огонь"
#define FSCALE 1+30       //масштаб
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
  modeCode=20;
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
{modeCode=14;
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // обрабатываем нашу матрицу снизу вверх до второй сверху строчки
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
      uint32_t thisColor = getPixColorXY(x, y);                                              // берём цвет нашего пикселя
      uint32_t upperColor = getPixColorXY(x, y + 1U);                                        // берём цвет пикселя над нашим
      if (upperColor >= 0x900000 && random(7 * HEIGHT) != 0U)                  // если выше нас максимальная яркость, игнорим этот факт с некой вероятностью или опускаем цепочку ниже
        drawPixelXY(x, y, upperColor);
      else if (thisColor == 0U && random((0,10) * HEIGHT) == 0U)  // если наш пиксель ещё не горит, иногда зажигаем новые цепочки
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
      if (random(0,10) == 0U)
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


// ============= ЭФФЕКТ ВОЛНЫ ===============
// https://github.com/pixelmatix/aurora/blob/master/PatternWave.h
// Адаптация от (c) SottNick

byte waveThetaUpdate = 0;
byte waveThetaUpdateFrequency = 0;
byte waveTheta = 0;

byte hueUpdate = 0;
byte hueUpdateFrequency = 0;
//    byte hue = 0; будем использовать сдвиг от эффектов Радуга

byte waveRotation = 0;
uint8_t waveScale = 256 / WIDTH;
uint8_t waveCount = 1;

void WaveRoutine() {
  if (loadingFlag)
  {
    loadingFlag = false;
    modeCode = 22;
    waveRotation = random(0, 4);// теперь вместо этого регулятор Масштаб
    //waveRotation = (modes[currentMode].Scale - 1) / 25U;
    waveCount = random(1, 3);// теперь вместо этого чётное/нечётное у регулятора Скорость
    //waveCount = modes[currentMode].Speed % 2;
    //waveThetaUpdateFrequency = random(1, 2);
    //hueUpdateFrequency = random(1, 6);
  }

  fader(200);

  float n = 0;

  switch (waveRotation) {
    case 0:
      for (float x = 0; x < WIDTH; x++) {
        n = (float)quadwave8(x * 2 + waveTheta) / waveScale;
        drawPixelXY(x, n, ColorFromPalette(RainbowColors_p, hue + x));
        if (waveCount != 1)
          drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(RainbowColors_p, hue + x));
      }
      break;

    case 1:
      for (float y = 0; y < HEIGHT; y++) {
        n = (float)quadwave8(y * 2 + waveTheta) / waveScale;
        drawPixelXY(n, y, ColorFromPalette(RainbowColors_p, hue + y));
        if (waveCount != 1)
          drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(RainbowColors_p, hue + y));
      }
      break;

    case 2:
      for (float x = 0; x < WIDTH; x++) {
        n = (float)quadwave8(x * 2 - waveTheta) / waveScale;
        drawPixelXY(x, n, ColorFromPalette(RainbowColors_p, hue + x));
        if (waveCount != 1)
          drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(RainbowColors_p, hue + x));
      }
      break;

    case 3:
      for (float y = 0; y < HEIGHT; y++) {
        n = (float)quadwave8(y * 2 - waveTheta) / waveScale;
        drawPixelXY(n, y, ColorFromPalette(RainbowColors_p, hue + y));
        if (waveCount != 1)
          drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(RainbowColors_p, hue + y));
      }
      break;
  }


  if (waveThetaUpdate >= waveThetaUpdateFrequency) {
    waveThetaUpdate = 0;
    waveTheta++;
  }
  else {
    waveThetaUpdate++;
  }

  if (hueUpdate >= hueUpdateFrequency) {
    hueUpdate = 0;
    hue++;
  }
  else {
    hueUpdate++;
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
uint8_t wrapX(int8_t x){
  return (x + WIDTH)%WIDTH;
}
uint8_t wrapY(int8_t y){
  return (y + HEIGHT)%HEIGHT;
}
// ********************* ЗВЕЗДОПАД ******************
void starfallRoutine()
{modeCode = 15;
  for (uint8_t x = 0U; x < WIDTH - 1U; x++) // fix error i != 0U
  {
    if (!random8(STAR_DENSE) &&
        !getPixColorXY(wrapX(x), HEIGHT - 1U) &&
        !getPixColorXY(wrapX(x + 1U), HEIGHT - 1U) &&
        !getPixColorXY(wrapX(x - 1U), HEIGHT - 1U))
    {
      drawPixelXY(x, HEIGHT - 1U, CHSV(random8(), SATURATION, random8(64U, 255U)));
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      drawPixelXY(wrapX(x + 1U), y, getPixColorXY(x, y + 1U));
    }
  }

  // уменьшаем яркость верхней линии, формируем "хвосты"
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    fadePixel(i, HEIGHT - 1U, TAIL_STEP);
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
