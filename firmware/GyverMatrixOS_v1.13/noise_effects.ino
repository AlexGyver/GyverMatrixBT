// крутые полноэкраные эффекты
// ******************* НАСТРОЙКИ *****************
// "масштаб" эффектов. Чем меньше, тем крупнее!
#define MADNESS_SCALE 100
#define CLOUD_SCALE 30
#define LAVA_SCALE 50
#define PLASMA_SCALE 30
#define RAINBOW_SCALE 30
#define RAINBOW_S_SCALE 20
#define ZEBRA_SCALE 30
#define FOREST_SCALE 120
#define OCEAN_SCALE 90

// ***************** ДЛЯ РАЗРАБОТЧИКОВ ******************
#if (USE_NOISE_EFFECTS == 1)
// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

uint16_t speed = 20; // speed is set dynamically once we've started up
uint16_t scale = 30; // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
#define MAX_DIMENSION (max(WIDTH, HEIGHT))
#if (WIDTH > HEIGHT)
uint8_t noise[WIDTH][WIDTH];
#else
uint8_t noise[HEIGHT][HEIGHT];
#endif

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t colorLoop = 1;
uint8_t ihue = 0;

void madnessNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    scale = MADNESS_SCALE;
    modeCode = 3;
  }
  fillnoise8();
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      if (variant){
      drawPixelXY(i, j, CHSV(noise[j][i],256 - noise[i][j], noise[i][j]));
      }else{
      drawPixelXY(i, j, CHSV(noise[j][i], 255, noise[i][j]));}
     
      // You can also explore other ways to constrain the hue used, like below
      // leds[XY(i,j)] = CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]);
    }
  }
  ihue += 1;
}
void rainbowNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    scale = RAINBOW_SCALE; colorLoop = 1;
    modeCode = 7;
  }
  if (variant){
    currentPalette = RainbowStripeColors_p;
    scale = RAINBOW_S_SCALE;
}else{
    currentPalette = RainbowColors_p;
    scale = RAINBOW_SCALE;}
  fillNoiseLED();
}
void zebraNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    scale = ZEBRA_SCALE; colorLoop = 1;

    modeCode = 9;
  }
  fillNoiseLED();
}
void forestNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = ForestColors_p;
    scale = FOREST_SCALE; colorLoop = 0;

    modeCode = 10;
  }
  fillNoiseLED();
}
void oceanNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = OceanColors_p;
    scale = OCEAN_SCALE; colorLoop = 0;

    modeCode = 11;
  }

  fillNoiseLED();
}
void plasmaNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = PartyColors_p;
    scale = PLASMA_SCALE; colorLoop = 1;

    modeCode = 6;
  }
  fillNoiseLED();
}
void cloudNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = CloudColors_p;
    scale = CLOUD_SCALE; colorLoop = 0;

    modeCode = 4;
  }
  fillNoiseLED();
}
void lavaNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    scale = LAVA_SCALE; colorLoop = 0;
    modeCode = 5;
  }
  if (variant)
    currentPalette = HeatColors_p;
  else
    currentPalette = LavaColors_p;
  fillNoiseLED();
}

void FillNoise(byte Speed, byte Scale, bool ShiftX, bool ShiftY, bool ShiftZ, CRGBPalette16 palette, bool ShiftHue, bool BriNoise) {
 double t = (millis() / (map(Speed, 1, 255, 60, 0)));
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      uint8_t noise = inoise8((x * Scale) + ((ShiftX) ? t : t / 8), (y * Scale) + ((ShiftY) ? t : t / 16), ((ShiftZ) ? t : 0));
      uint8_t Inoise = inoise8((y * Scale) + ((ShiftY) ? t : t / 16), (x * Scale) + ((ShiftX) ? t : t / 8), ((ShiftZ) ? t : 0));
      leds[XY(x, y)] = ColorFromPalette(palette, noise + hue,(!BriNoise || Inoise > 127)? 255 : dim8_raw(Inoise * 2));
    }
  }
  if (ShiftHue) { hue++; } else { hue = 0; }
}

#else
void madnessNoise() {
  return;
}
void cloudNoise() {
  return;
}
void lavaNoise() {
  return;
}
void plasmaNoise() {
  return;
}
void rainbowNoise() {
  return;
}
void zebraNoise() {
  return;
}
void forestNoise() {
  return;
}
void oceanNoise() {
  return;
}
#endif
