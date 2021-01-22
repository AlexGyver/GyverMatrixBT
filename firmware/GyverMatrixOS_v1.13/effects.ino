// эффекты

// **************** НАСТРОЙКИ ЭФФЕКТОВ ****************
// эффект "светлячки"
#define BALLS_AMOUNT 3    // количество "светлячков"
#define CLEAR_PATH 1      // очищать путь
#define TRACK_STEP 120    // длина хвоста светлячка (чем больше цифра, тем хвост короче)

// эффект "квадратик"
#define BALL_SIZE 2       // размер шара

// эффект "огонь"
#define FSCALE 20         //масштаб
#define SPARKLES_NUM  1   //кол-во летающих углей (WIDTH / 8U)

// эффект "кометы"
#define TAIL_STEP 120     // длина хвоста кометы
#define SATURATION 150    // насыщенность кометы (от 0 до 255)
#define STAR_DENSE 32     // количество (шанс появления) комет

// эффект "конфетти"
#define DENSE 3           // плотность конфетти
#define BRIGHT_STEP 70    // шаг уменьшения яркости

// эффект "снег"
#define SNOW_DENSE 10     // плотность снегопада

// эффекты "узоры" и "тикси ленд"
#define NEW_TIME 0       // время обновления эфффекта(в секундах)(0-отключить)

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
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = 12;
  }
  // сдвигаем всё вниз

  if (variant) {// заполняем головами комет левую и верхнюю линию
    for (byte i = 0; i < HEIGHT; i++) {
      if (getPixColorXY(0, i) == 0
          && (random(0, SNOW_DENSE) == 0)
          && getPixColorXY(0, i + 1) == 0
          && getPixColorXY(0, i - 1) == 0)
        leds[getPixelNumber(0, i)] = CHSV(0, 0, 255);
    }
    for (byte i = 0; i < WIDTH; i++) {
      if (getPixColorXY(i, HEIGHT - 1) == 0
          && (random(0, SNOW_DENSE) == 0)
          && getPixColorXY(i + 1, HEIGHT - 1) == 0
          && getPixColorXY(i - 1, HEIGHT - 1) == 0)
        leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(0, 0, 255);
    }
    // сдвигаем по диагонали
    for (byte y = 0; y < HEIGHT - 1; y++) {
      for (byte x = WIDTH - 1; x > 0; x--) {
        drawPixelXY(x, y, getPixColorXY(x - 1, y + 1));
      }
    }


    // уменьшаем яркость левой и верхней линии, формируем "хвосты"
    for (byte i = 0; i < HEIGHT; i++) {
      fadePixel(0, i, TAIL_STEP);
    }
    for (byte i = 0; i < WIDTH; i++) {
      fadePixel(i, HEIGHT - 1, TAIL_STEP / 2 + 10);
    }
  }
  else {
    shiftDown();
    for (byte x = 0; x < WIDTH; x++) {
      // заполняем случайно верхнюю строку
      // а также не даём двум блокам по вертикали вместе быть
      if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, SNOW_DENSE) == 0))
        drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
      else
        drawPixelXY(x, HEIGHT - 1, 0x000000);
    }
  }
}

// ***************************** БЛУДНЫЙ КУБИК *****************************
int coord[BALLS_AMOUNT][2];
int8_t vector[BALLS_AMOUNT][2];
byte ballColors[BALLS_AMOUNT];


void ballRoutine() {
  if (loadingFlag) {
    for (byte i = 0; i < 2; i++) {
      coord[0][i] = WIDTH / 2 * 10;
      vector[0][i] = random(8, 20);
      ballColors[0] = random(0, 9) * 28;
    }
    modeCode = 16;
    loadingFlag = false;
  }
  for (byte i = 0; i < 2; i++) {
    coord[0][i] += vector[0][i];
    if (coord[0][i] < 0) {
      coord[0][i] = 0;
      vector[0][i] = -vector[0][i];
      ballColors[0] = random(0, 9) * 28;
      //vector[0][i] += random(0, 6) - 3;
    }
  }
  if (coord[0][0] > (WIDTH - BALL_SIZE) * 10) {
    coord[0][0] = (WIDTH - BALL_SIZE) * 10;
    vector[0][0] = -vector[0][0];
    ballColors[0] = random(0, 9) * 28;
    //vector[0][0] += random(0, 6) - 3;
  }
  if (coord[0][1] > (HEIGHT - BALL_SIZE) * 10) {
    coord[0][1] = (HEIGHT - BALL_SIZE) * 10;
    vector[0][1] = -vector[0][1];
    ballColors[0] = random(0, 9) * 28;
    //vector[0][1] += random(0, 6) - 3;
  }
  if (variant)fader(75);
  else FastLED.clear();
  for (byte i = 0; i < BALL_SIZE; i++)
    for (byte j = 0; j < BALL_SIZE; j++)
      leds[getPixelNumber(coord[0][0] / 10 + i, coord[0][1] / 10 + j)] = CHSV(ballColors[0], 255, 255);
}


// *********** радуга заливка ***********
void rainbowRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = 18;
  }

  hue += 3;
  if (variant) {
    for (byte i = 0; i < WIDTH; i++) {
      CHSV thisColor = CHSV((byte)(hue + i * float(255 / WIDTH)), 255, 255);
      for (byte j = 0; j < HEIGHT; j++)
        drawPixelXY(i, j, thisColor);
    }
  }
  else {
    for (byte j = 0; j < HEIGHT; j++) {
      CHSV thisColor = CHSV((byte)(hue + j * float(255 / WIDTH)), 255, 255);
      for (byte i = 0; i < WIDTH; i++)
        drawPixelXY(i, j, thisColor);
    }
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
  if (loadingFlag) {
    loadingFlag = false;
  }
  hue++;
  if (variant) {
    for (byte i = 0; i < WIDTH; i++) {
      CHSV thisColor = CHSV((byte)(hue + i * float(255 / WIDTH)), 255, 255);
      for (byte j = 0; j < HEIGHT; j++)
        if (getPixColor(getPixelNumber(i, j)) > 0) drawPixelXY(i, j, thisColor);
    }
  }
  else {
    for (byte j = 0; j < HEIGHT; j++) {
      CHSV thisColor = CHSV((byte)(hue + j * float(255 / WIDTH)), 255, 255);
      for (byte i = 0; i < WIDTH; i++)
        if (getPixColor(getPixelNumber(i, j)) > 0) drawPixelXY(i, j, thisColor);
    }
  }
}


// ********************** ОГОНЬ **********************
// (c) SottNick
//сильно по мотивам https://pastebin.com/RG0QGzfK
//Perlin noise fire procedure by Yaroslaw Turbin
//https://www.reddit.com/r/FastLED/comments/hgu16i/my_fire_effect_implementation_based_on_perlin/
uint16_t ff_x, ff_y, ff_z;
uint8_t deltaHue, deltaHue2; // ещё пара таких же, когда нужно много
uint8_t spark[2][SPARKLES_NUM];
uint8_t shiftHue[HEIGHT];
uint8_t deltaValue;
uint8_t step;

void fireRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    deltaValue = FSCALE * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
    deltaValue = (((FSCALE - 1U) % 11U + 1U) << 4U) - 8U; // ширина языков пламени (масштаб шума Перлина)
    deltaHue = map(deltaValue, 8U, 168U, 8U, 84U); // высота языков пламени должна уменьшаться не так быстро, как ширина
    step = map(255U - deltaValue, 87U, 247U, 4U, 32U); // вероятность смещения искорки по оси ИКС
    for (uint8_t j = 0; j < HEIGHT; j++) {
      shiftHue[j] = (HEIGHT - 1 - j) * 255 / (HEIGHT - 1); // init colorfade table
    }

    for (uint8_t i = 0; i < SPARKLES_NUM; i++) {
      spark[1][i] = random8(HEIGHT);
      spark[0][i] = random8(WIDTH);
    }
  }
  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      leds[XY(i, HEIGHT - 1U - j)] = ColorFromPalette(HeatColors_p, qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U);
    }
  }

  //вставляем искорки из отдельного массива
  for (uint8_t i = 0; i < SPARKLES_NUM; i++) {
    //leds[XY(trackingObjectPosX[i],trackingObjectPosY[i])] += ColorFromPalette(*curPalette, random(156, 255));   //trackingObjectHue[i]
    if (spark[1][i] > 3U) {
      leds[XY(spark[0][i], spark[1][i])] = leds[XY(spark[0][i], 3U)];
      leds[XY(spark[0][i], spark[1][i])].fadeToBlackBy( spark[1][i] * 2U );
    }
    spark[1][i]++;
    if (spark[1][i] >= HEIGHT) {
      spark[1][i] = random8(4U);
      spark[0][i] = random8(WIDTH);
    }
    if (!random8(step))
      spark[0][i] = (WIDTH + (uint8_t)spark[0][i] + 1U - random8(3U)) % WIDTH;
  }
  ff_y++;
  if (ff_y & 0x01)
    ff_z++;
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

// ********************************* СВЕТЛЯЧКИ *********************************
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
      ballColors[j] = random(0, 9) * 28;
    }
  }



  if (variant)  FastLED.clear();
  else fader(256U - TRACK_STEP);


  // движение шариков
  for (byte j = 0; j < BALLS_AMOUNT; j++) {
    ballColors[j] ++;
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
    leds[getPixelNumber(coord[j][0] / 10, coord[j][1] / 10)] =  CHSV(ballColors[j], 255, 255);
  }
}



void fader(uint8_t step)                                   // плавное угасание всех пикселей
{
  fadeToBlackBy (leds, NUM_LEDS, 255U - step);
}
void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = XY(i, j);
  if (getPixColor(pixelNum) == 0U) return;

  if (leds[pixelNum].r >= 30U ||
      leds[pixelNum].g >= 30U ||
      leds[pixelNum].b >= 30U)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0U;
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
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = 15;
  }
  // заполняем головами комет левую и верхнюю линию
  for (byte i = 0; i < HEIGHT; i++) {
    if (getPixColorXY(0, i) == 0
        && (random(0, STAR_DENSE) == 0)
        && getPixColorXY(0, i + 1) == 0
        && getPixColorXY(0, i - 1) == 0)
      leds[getPixelNumber(0, i)] = CHSV(random(0, 200), SATURATION, 255);
  }
  for (byte i = 0; i < WIDTH; i++) {
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
  for (byte i = 0; i < HEIGHT; i++) {
    fadePixel(0, i, TAIL_STEP);
  }
  for (byte i = 0; i < WIDTH; i++) {
    fadePixel(i, HEIGHT - 1, TAIL_STEP / 2 + 10);
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
  fader(256U - BRIGHT_STEP);
}

#if (USE_MODULE_EFFECTS == 1)
// ------------------Узоры-----------------------
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/patterns.ino
int8_t lineIdx = 0;


CHSV colorMR[5] = {
  CHSV(0, 0, 0),              // 0 - Black
  CHSV(HUE_RED, 255, 255),    // 1 - Red
  CHSV(0, 0, 220),            // 2 - White
  CHSV(0, 255, 255),              // 3 - плавно меняеться в цикле (фон)
  CHSV(0, 255, 255),              // 4 - цвет равен 6 но +64
};


void drawPattern(uint8_t ptrn, uint8_t X, uint8_t Y, uint8_t W, uint8_t H, bool dir) {


  if (dir)
    shiftDown();
  else
    shiftUp();


  uint8_t y = dir ? (HEIGHT - 1) : 0;

  // Если ширина паттерна не кратна ширине матрицы - отрисовывать со сдвигом? чтобы рисунок был максимально по центру
  int8_t offset_x = -((WIDTH % W) / 2) + 3;

  for (uint8_t x = 0; x < WIDTH + W; x++) {
    int8_t xx = offset_x + x;
    if (xx >= 0 && xx < (int8_t)WIDTH) {
      uint8_t in = (uint8_t)pgm_read_byte(&(patterns[ptrn][lineIdx][x % 10]));
      CHSV color = colorMR[in];
      CHSV color2 = color.v != 0 ? CHSV(color.h, color.s, 255) : color;
      drawPixelXY(xx, y, color2);
    }
  }
  if (dir) {
    lineIdx = (lineIdx > 0) ? (lineIdx - 1) : (H - 1);
  } else {
    lineIdx = (lineIdx < H - 1) ? (lineIdx + 1) : 0;
  }
}



// Отрисовка указанной картинки с размерами WxH в позиции XY
void drawPicture_XY(uint8_t iconIdx, uint8_t X, uint8_t Y, uint8_t W, uint8_t H) {
  if (loadingFlag) {
    loadingFlag = false;
  }

  for (byte x = 0; x < W; x++) {
    for (byte y = 0; y < H; y++) {
      uint8_t in = (uint8_t)pgm_read_byte(&(patterns[iconIdx][y][x]));
      if (in != 0) {
        CHSV color = colorMR[in];
        CHSV color2 = color.v != 0 ? CHSV(color.h, color.s, 255) : color;
        drawPixelXY(X + x, Y + H - y, color2);
      }
    }
  }
}
void patternsRoutine() {


  if (loadingFlag) {
    loadingFlag = false;
    patternIdx = random8(0U, MAX_PATTERN);
    if (variant)
      lineIdx = 9;         // Картинка спускается сверху вниз - отрисовка с нижней строки паттерна (паттерн 10x10)
    else
      lineIdx = 0;         // Картинка поднимается сверху вниз - отрисовка с верхней строки паттерна
    // Цвета с индексом 6 и 7 - случайные, определяются в момент настройки эффекта
    colorMR[3] = CHSV(random8(), 255U, 255U);
    colorMR[4].hue = colorMR[3].hue + 96; //(beatsin8(1, 0, 255, 0, 127), 255U, 255U);
  }
  drawPattern(patternIdx, 0, 0, 10, 10, variant);
  if (!NEW_TIME == 0) {
    EVERY_N_SECONDS(NEW_TIME) {
      patternIdx ++;
      if (patternIdx >= MAX_PATTERN) patternIdx = 0;
    }
  }
}

// ---------- Эффект "Тикси Ленд"
// (c)  Martin Kleppe @aemkei, https://github.com/owenmcateer/tixy.land-display
void TLandRoutine() {
  modeCode = 23;
  double t = map(globalSpeed, 1, 255, 1200, 128);
  hue++;
  for ( double x = 0; x < WIDTH; x++) {
    for ( double y = 0; y < HEIGHT; y++) {
      processFrame(leds, t, x, y);
    }
  }
  if (!NEW_TIME == 0) {
    EVERY_N_SECONDS(NEW_TIME) {
      animation++;
    }
  }
}
void processFrame(CRGB *leds, double t, double x, double y) {
  double i = (y * 16) + x;
  double frame = constrain(code(t, i, x, y), -1, 1) * 255;

  if (frame >= 0) {
    leds[getPixelNumber(x, HEIGHT - 1 - y)].setRGB(frame, frame, frame);
  }
  else {
    leds[getPixelNumber(x, HEIGHT - 1 - y)].setRGB(-frame, 0, 0);
  }
}

float code(double t, double i, double x, double y) {
  //float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

  switch (animation) {
    case 0:
      //Plasma
      return sin(x + t) + sin(y + t) + sin(x + y + t) / 3;
      break;
    case 1:
      // Up&Down
      return sin(cos(x) * y / 8 + t);
      break;

    case 2:
      return sin(y * (t / 4.)) * cos(x * (t / 4.));
      break;

    case 22:
      return sin(i / 5 + (t));
      break;

    case 23:
      return sin(cos(y) * t) * cos(sin(x) * t);
      break;


    /**
       tixy.land website
    */
    case 3:
      // Emitting rings
      return sin(t - sqrt(pow((x - 7.5), 2) + pow((y - 6), 2)));
      break;

    case 4:
      // Rotation
      return sin(PI * 2 * atan((y - 8) / (x - 8)) + 5 * t);
      break;

    case 5:
      // Vertical fade
      return sin(y / 8 + t);
      break;

    case 6:
      // Smooth noise
      return cos(t + i + x * y);
      break;

    case 7:
      // Waves
      return sin(x / 2) - sin(x - t * 4) - y + 6;
      break;

    case 8:
      // Drop
      return fmod(8 * t, 13) - hypot(x - 7.5, y - 7.5);
      break;



    case 9:
      // Ripples @thespite
      return sin(t - sqrt(x * x + y * y));
      break;

    case 10:
      // Bloop bloop bloop @v21
      return  (x - 8) * (y - 8) - sin(t / 2) * 64;;
      break;


    /**
       Reddit
    */
    case 11:
      // lurkerurke https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgcwsn/
      return sin((x - 7.5) * (y - 7.5) / 5 * t + t);
      break;

    case 12:
      // SN0WFAKER https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk7c0/
      return sin(atan((y - 7.5) / (x - 7.5)) + t);
      break;

    case 13:
      // LeadingNegotiation9 https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbjcoho/
      return pow(cos(((int)y ^ (int)x) + t), cos((x > y) + t));
      break;

    case 14:
      // detunized https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk30l/
      return sin(y / 8 + t * 0.5) + x / 16 - 0.5;
      break;

    case 15:
      // Andres_A https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgzdnj/
      return 1 - hypot(sin(t) * 9 - x, cos(t) * 9 - y) / 9;
      break;


    /**
       @akella
       https://twitter.com/akella/status/1323549082552619008
    */
    case 16:
      return sin(6 * atan2(y - 8, x) + t);
      break;


    /**
       Paul Malin
       https://twitter.com/P_Malin/
    */
    case 17:
      // Parallax Scrolling Checkerboard https://twitter.com/P_Malin/status/1323609539648905218
      return ((int)((x - 8) / y + t * 2) & 1 ^ (int)(1 / y * 8) & 1) * y / 5;
      break;

    case 18:
      // Matrix Rain https://twitter.com/P_Malin/status/1323583013880553472
      return 1 - fmod((x * x - y + t * (fmod(1 + x * x, 5.0)) * 3.0), 16.0) / 16.0;
      break;

    case 19:
      // Burst https://twitter.com/P_Malin/status/1323605999274594304
      return -10. / ((x - 8.) * (x - 8.) + (y - 8.) * (y - 8.) - fmod(t * 0.3, 0.7) * 200.);
      break;

    case 20:
      // Rays
      return sin(atan2(x, y) * 5 + t * 2);
      break;

    case 21:
      // Starfield https://twitter.com/P_Malin/status/1323702220320313346
      return !((int)(x + (t / 2.) * 50. / (fmod(y * y, 5.9) + 1.)) & 15) / (fmod(y * y, 5.9) + 1.);
      break;

    default:
      animation = 0;
      break;
  }
}

#else
void patternsRoutine() {
  return;
}
void TLandRoutine() {
  return;
}
#endif
