#include <EEPROM.h>
#define EEPROM_OK 0xA5        // Флаг, показывающий, что EEPROM инициализирована корректными данными 
#define MAX_EFFECT 22         // количество эффектов, определенных в прошивке
#define MAX_GAME 6            // количество игр, определенных в прошивке
#define EFFECT_EEPROM 20      // начальная ячейка eeprom с параметрами эффектов
#define GAME_EEPROM 100       // начальная ячейка eeprom с параметрами игр

bool eepromModified = false;

void loadSettings() {

  // Адреса в EEPROM:
  //   0 - если 0xAA - EEPROM инициализировано, если другое значение - нет 
  //   1 - максимальная яркость ленты 1-255
  //   2 - скорость прокрутки текста по умолчанию
  //   3 - скорость эффектов по умолчанию
  //   4 - скорость игр по умолчанию
  //   5 - зарезервировано
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
    gameSpeed =   map(EEPROM.read(4),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);        
  } else {
    globalBrightness = BRIGHTNESS;
    scrollSpeed = D_TEXT_SPEED;
    effectSpeed = D_EFFECT_SPEED;
    gameSpeed = D_GAME_SPEED;
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
}

void saveSettings() {

  // Поставить отметку, что EEPROM инициализировано параметрами эффектов
  EEPROM.write(0, EEPROM_OK);
  EEPROM.write(1, globalBrightness);

  EEPROM.write(2, constrain(map(scrollSpeed, D_TEXT_SPEED_MIN, D_TEXT_SPEED_MAX, 0, 255), 0, 255));
  EEPROM.write(3, constrain(map(effectSpeed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));
  EEPROM.write(4, constrain(map(gameSpeed, D_GAME_SPEED_MIN, D_GAME_SPEED_MAX, 0, 255), 0, 255));
  
  EEPROM.commit();
}

void saveEffectParams(byte effect, int speed, boolean overlay) {
  const int addr = EFFECT_EEPROM;  
  EEPROM.write(addr + effect*2, constrain(map(speed, D_EFFECT_SPEED_MIN, D_EFFECT_SPEED_MAX, 0, 255), 0, 255));        // Скорость эффекта
  EEPROM.write(addr + effect*2 + 1, overlay ? 1 : 0);             // По умолчанию оверлей часов для эффекта отключен  
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
}

int getGameSpeed(byte game) {
  const int addr = GAME_EEPROM;  
  return map(EEPROM.read(addr + game*2),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);
}
