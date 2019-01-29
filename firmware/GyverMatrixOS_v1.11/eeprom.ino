#if (USE_EEPROM == 1)
#define EEPROM_OK 0xA5        // Флаг, показывающий, что EEPROM инициализирована корректными данными 
#define EFFECT_EEPROM 20      // начальная ячейка eeprom с параметрами эффектов
#define GAME_EEPROM 100       // начальная ячейка eeprom с параметрами игр

void loadSettings() {

  // Адреса в EEPROM:
  //   0 - если 0xAA - EEPROM инициализировано, если другое значение - нет 
  //   1 - максимальная яркость ленты 1-255
  //   2 - скорость прокрутки текста по умолчанию
  //   3 - скорость эффектов по умолчанию
  //   4 - скорость игр по умолчанию
  //   5 - разрешен оверлей часов для эффектов
  //   6 - автосмена режима в демо: вкл/выкл
  //   7 - время автосмены режимов
  //   8 - время бездействия до переключения в авторежим
  //   9 - зарезервировано
  // ... - зарезервировано
  //  19 - зарезервировано
  //  20 - 20+(Nэфф*2)   - скорость эффекта
  //  21 - 20+(Nэфф*2)+1 - 1 - оверлей часов разрешен; 0 - нет оверлея часов
  // ....
  //  100 - 100+(Nигр*2)   - скорость игры
  //  100 - 100+(Nигр*2)+1 - зарезервировано

  // Инициализировано ли EEPROM
  bool isInitialized = EEPROM.read(0) == EEPROM_OK;  
  
  if (isInitialized) {    
    globalBrightness = EEPROM.read(1);
    scrollSpeed = map(EEPROM.read(2),0,255,D_TEXT_SPEED_MIN,D_TEXT_SPEED_MAX);
    effectSpeed = map(EEPROM.read(3),0,255,D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX);
    gameSpeed = map(EEPROM.read(4),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);   
    overlayEnabled = EEPROM.read(5);
    AUTOPLAY = EEPROM.read(6) == 1;
    autoplayTime = EEPROM.read(7) * 1000;
    idleTime = EEPROM.read(8) * 1000;
  } else {
    globalBrightness = BRIGHTNESS;
    scrollSpeed = D_TEXT_SPEED;
    effectSpeed = D_EFFECT_SPEED;
    gameSpeed = D_GAME_SPEED;
    overlayEnabled = true;
    AUTOPLAY = true;
    autoplayTime = ((long)AUTOPLAY_PERIOD * 1000);
    idleTime = ((long)IDLE_TIME * 1000);
  }

  scrollTimer.setInterval(scrollSpeed);
  effectTimer.setInterval(effectSpeed);
  gameTimer.setInterval(gameSpeed);
  
  // После первой инициализации значений - сохранить их принудительно
  if (!isInitialized) {
    saveDefaults();
    saveSettings();
  }
}

void saveDefaults() {

  EEPROM.write(1, globalBrightness);

  EEPROM.write(2, constrain(map(scrollSpeed, D_TEXT_SPEED_MIN, D_TEXT_SPEED_MAX, 0, 255), 0, 255));
  EEPROM.write(3, constrain(map(effectSpeed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));
  EEPROM.write(4, constrain(map(gameSpeed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));

  EEPROM.write(5, overlayEnabled);
  EEPROM.write(6, AUTOPLAY ? 1 : 0);
  EEPROM.write(7, autoplayTime / 1000);

  // Настройки по умолчанию для эффектов
  int addr = EFFECT_EEPROM;
  for (int i = 0; i < MAX_EFFECT; i++) {
    saveEffectParams(i, effectSpeed, false);
  }

  // Настройки по умолчанию для игр
  addr = GAME_EEPROM;
  for (int i = 0; i < MAX_GAME; i++) {
    saveGameParams(i, gameSpeed);
  }
  
  eepromModified = true;
}

void saveSettings() {

  if (!eepromModified) return;
  
  // Поставить отметку, что EEPROM инициализировано параметрами эффектов
  EEPROM.write(0, EEPROM_OK);

  EEPROM.commit();

#if (BT_MODE == 0)
  Serial.println("Настройки сохранены в EEPROM");
#endif
  
  eepromModified = false;
}

void saveEffectParams(byte effect, int speed, boolean overlay) {
  const int addr = EFFECT_EEPROM;  
  EEPROM.write(addr + effect*2, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        // Скорость эффекта
  EEPROM.write(addr + effect*2 + 1, overlay ? 1 : 0);             // По умолчанию оверлей часов для эффекта отключен  
  eepromModified = true;
}

void saveEffectSpeed(byte effect, int speed) {
  if (speed != getEffectSpeed(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROM.write(addr + effect*2, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        // Скорость эффекта
    eepromModified = true;
  }
}

void saveEffectClock(byte effect, boolean overlay) {
  if (overlay != getEffectClock(effect)) {
    const int addr = EFFECT_EEPROM;  
    EEPROM.write(addr + effect*2 + 1, overlay ? 1 : 0);             // По умолчанию оверлей часов для эффекта отключен  
    eepromModified = true;
  }
}

int getEffectSpeed(byte effect) {
  const int addr = EFFECT_EEPROM;
  return map(EEPROM.read(addr + effect*2),0,255,D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX);
}

boolean getEffectClock(byte effect) {
  const int addr = EFFECT_EEPROM;
  return EEPROM.read(addr + effect*2 + 1) == 1;
}

void saveGameParams(byte game, int speed) {
  const int addr = GAME_EEPROM;  
  EEPROM.write(addr + game*2, constrain(map(speed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));         // Скорость эффекта
  EEPROM.write(addr + game*2 + 1, 0);         // Зарезервироано
  eepromModified = true;
}

void saveGameSpeed(byte game, int speed) {
  if (speed != getGameSpeed(game)) {
    const int addr = GAME_EEPROM;  
    EEPROM.write(addr + game*2, constrain(map(speed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));         // Скорость эффекта
    eepromModified = true;
  }
}

int getGameSpeed(byte game) {
  const int addr = GAME_EEPROM;  
  return map(EEPROM.read(addr + game*2),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);
}

boolean getClockOverlayEnabled() {
  return EEPROM.read(5) == 1;
}

void saveClockOverlayEnabled(boolean enabled) {
  if (enabled != getClockOverlayEnabled()) {
    EEPROM.write(5, enabled ? 1 : 0);
    eepromModified = true;
  }
}

void saveScrollSpeed(int speed) {
  if (speed != getScrollSpeed()) {
    EEPROM.write(2, constrain(map(speed, D_TEXT_SPEED_MIN, D_TEXT_SPEED_MAX, 0, 255), 0, 255));
    eepromModified = true;
  }
}

int getScrollSpeed() {
  return map(EEPROM.read(2),0,255,D_TEXT_SPEED_MIN,D_TEXT_SPEED_MAX);
}

byte getMaxBrightness() {
  return EEPROM.read(1);
}

void saveMaxBrightness(byte brightness) {
  if (brightness != getMaxBrightness()) {
    EEPROM.write(1, globalBrightness);
    eepromModified = true;
  }
}

void saveAutoplay(boolean value) {
  if (value != getAutoplay()) {
    EEPROM.write(6, value);
    eepromModified = true;
  }
}

bool getAutoplay() {
  return EEPROM.read(6) == 1;
}

void saveAutoplayTime(long value) {
  if (value != getAutoplayTime()) {
    EEPROM.write(7, constrain(value / 1000, 0, 255));
    eepromModified = true;
  }
}

long getAutoplayTime() {
  long time = EEPROM.read(7) * 1000;  
  if (time == 0) time = ((long)AUTOPLAY_PERIOD * 1000);
  return time;
}

void saveIdleTime(long value) {
  if (value != getIdleTime()) {
    EEPROM.write(8, constrain(value / 1000, 0, 255));
    eepromModified = true;
  }
}

long getIdleTime() {
  long time = EEPROM.read(8) * 1000;  
  if (time == 0) time = ((long)IDLE_TIME * 1000);
  return time;
}

#else

void loadSettings() { }
void saveSettings() { eepromModified = false; }
void saveEffectParams(byte effect, int speed, boolean overlay) { }
void saveEffectSpeed(byte effect, int speed) { }
void saveEffectClock(byte effect, boolean overlay) { }
int getEffectSpeed(byte effect) { return effectSpeed; }
boolean getEffectClock(byte effect) { return overlayAllowed(); }
void saveGameParams(byte game, int speed) { }
void saveGameSpeed(byte game, int speed) { }
int getGameSpeed(byte game) { return gameSpeed; }
boolean getClockOverlayEnabled() { return overlayEnabled; }
void saveClockOverlayEnabled(boolean enabled) { }
void saveScrollSpeed(int speed) { }
int getScrollSpeed() { return scrollSpeed; }
byte getMaxBrightness(byte brightness) { return globalBrightness; }
void saveMaxBrightness(byte brightness) {}
void saveAutoplay(boolean value) { }
bool getAutoplay() { return AUTOPLAY; }
void saveAutoplayTime(long value) { }
long getAutoplayTime() { return autoplayTime; }
void saveIdleTime(long value) { }
long getIdleTime() { return autoplayTime; }

#endif
