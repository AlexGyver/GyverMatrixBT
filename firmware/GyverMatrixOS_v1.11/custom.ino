// свой список режимов

// ************************ НАСТРОЙКИ ************************
#define SMOOTH_CHANGE 1     // плавная смена режимов через чёрный
#define SHOW_FULL_TEXT 1    // не переключать режим, пока текст не покажется весь
#define SHOW_TEXT_ONCE 1    // показывать бегущий текст только 1 раз

/*
   Режимы:
    clockRoutine();       // часы на чёрном фоне

   Эффекты:
    sparklesRoutine();    // случайные цветные гаснущие вспышки
    snowRoutine();        // снег
    matrixRoutine();      // "матрица"
    starfallRoutine();    // звездопад (кометы)
    ballRoutine();        // квадратик
    ballsRoutine();       // шарики
    rainbowRoutine();     // радуга во всю матрицу горизонтальная
    rainbowDiagonalRoutine();   // радуга во всю матрицу диагональная
    fireRoutine();        // огонь

  Крутые эффекты "шума":
    madnessNoise();       // цветной шум во всю матрицу
    cloudNoise();         // облака
    lavaNoise();          // лава
    plasmaNoise();        // плазма
    rainbowNoise();       // радужные переливы
    rainbowStripeNoise(); // полосатые радужные переливы
    zebraNoise();         // зебра
    forestNoise();        // шумящий лес
    oceanNoise();         // морская вода

  Игры:
    snakeRoutine();     // змейка
    tetrisRoutine();    // тетрис
    mazeRoutine();      // лабиринт
    runnerRoutine();    // бегалка прыгалка
    flappyRoutine();    // flappy bird
    arkanoidRoutine();  // арканоид

  Бегущая строка:
    fillString("Ваш текст", цвет);    // цвет вида 0x00ff25 или CRGB::Red и проч. цвета
    fillString("Ваш текст", 1);       // радужный перелив текста
    fillString("Ваш текст", 2);       // каждая буква случайным цветом!

  Рисунки и анимации:
    loadImage(<название массива>);    // основная функция вывода картинки
    imageRoutine();                   // пример использования
    animation();                      // пример анимации

*/

// ************************* СВОЙ СПИСОК РЕЖИМОВ ************************
// список можно менять, соблюдая его структуру. Можно удалять и добавлять эффекты, ставить их в
// любой последовательности или вообще оставить ОДИН. Удалив остальные case и break. Cтруктура оч простая:
// case <номер>: <эффект>;
//  break;

// не забудьте указать количество режимов для корректного переключения с последнего на первый

// количество кастомных режимов (которые переключаются сами или кнопкой)
#if (USE_ANIMATION == 1 && WIDTH == 16 && HEIGHT == 16)
#define MODES_AMOUNT 29   
#else
#define MODES_AMOUNT 28
#endif

String text;

void customModes() {
  switch (thisMode) {    
    case 0: 
      text = runningText == "" ? "Gyver Matrix" : runningText;
      fillString(text, CRGB::RoyalBlue);
      break;
    case 1: 
      text = runningText == "" ? "РАДУГА" : runningText;
      fillString(text, 1);
      break;
    case 2: 
      text = runningText == "" ? "RGB LED" : runningText;
      fillString(text, 2);
      break;
    case 3: madnessNoise();
      break;
    case 4: cloudNoise();
      break;
    case 5: lavaNoise();
      break;
    case 6: plasmaNoise();
      break;
    case 7: rainbowNoise();
      break;
    case 8: rainbowStripeNoise();
      break;
    case 9: zebraNoise();
      break;
    case 10: forestNoise();
      break;
    case 11: oceanNoise();
      break;
    case 12: snowRoutine();
      break;
    case 13: sparklesRoutine();
      break;
    case 14: matrixRoutine();
      break;
    case 15: starfallRoutine();
      break;
    case 16: ballRoutine();
      break;
    case 17: ballsRoutine();
      break;
    case 18: rainbowRoutine();
      break;
    case 19: rainbowDiagonalRoutine();
      break;
    case 20: fireRoutine();
      break;
    case 21: snakeRoutine();
      break;
    case 22: tetrisRoutine();
      break;
    case 23: mazeRoutine();
      break;
    case 24: runnerRoutine();
      break;
    case 25: flappyRoutine();
      break;
    case 26: arkanoidRoutine();
      break;
    case 27: clockRoutine();
      break;
#if (USE_ANIMATION == 1 && WIDTH == 16 && HEIGHT == 16)
    case 28: animation();
      break;
#endif      
  }
}

// ********************* ОСНОВНОЙ ЦИКЛ РЕЖИМОВ *******************
#if (SMOOTH_CHANGE == 1)
byte fadeMode = 4;
boolean modeDir;
#endif

static void nextMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = true;
#else
  nextModeHandler();
#endif
}

static void prevMode() {
#if (SMOOTH_CHANGE == 1)
  fadeMode = 0;
  modeDir = false;
#else
  prevModeHandler();
#endif
}

void nextModeHandler() {
  thisMode++;
  if (thisMode >= MODES_AMOUNT) thisMode = 0;
  loadingFlag = true;
  gamemodeFlag = false;
  autoplayTimer = millis();
  FastLED.clear();
  FastLED.show();
}

void prevModeHandler() {
  thisMode--;
  if (thisMode < 0) thisMode = MODES_AMOUNT - 1;
  loadingFlag = true;
  gamemodeFlag = false;
  autoplayTimer = millis();
  FastLED.clear();
  FastLED.show();
}

int fadeBrightness;
int fadeStepCount = 10;     // За сколько шагов убирать/добавлять яркость при смене режимов
int fadeStepValue = 5;      // Шаг убавления яркости

#if (SMOOTH_CHANGE == 1)
void modeFader() {
  if (fadeMode == 0) {
    fadeMode = 1;
    fadeStepValue = fadeBrightness / fadeStepCount;
    if (fadeStepValue < 1) fadeStepValue = 1;
  } else if (fadeMode == 1) {
    if (changeTimer.isReady()) {
      fadeBrightness -= fadeStepValue;
      if (fadeBrightness < 0) {
        fadeBrightness = 0;
        fadeMode = 2;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  } else if (fadeMode == 2) {
    fadeMode = 3;
    if (modeDir) nextModeHandler();
    else prevModeHandler();
  } else if (fadeMode == 3) {
    if (changeTimer.isReady()) {
      fadeBrightness += fadeStepValue;
      if (fadeBrightness > globalBrightness) {
        fadeBrightness = globalBrightness;
        fadeMode = 4;
      }
      FastLED.setBrightness(fadeBrightness);
    }
  }
}
#endif

boolean loadFlag2;

void customRoutine() {
   
  if (!gamemodeFlag) {
    if (effectTimer.isReady()) {
      
#if (OVERLAY_CLOCK == 1 && USE_CLOCK == 1)
      if (overlayEnabled && overlayAllowed()) {
        if (!loadingFlag && !gamemodeFlag && needUnwrap() && modeCode != MC_TEXT) clockOverlayUnwrap(CLOCK_X, CLOCK_Y);
        if (loadingFlag) loadFlag2 = true;
      }
#endif

      customModes();                // режимы крутятся, пиксели мутятся

#if (OVERLAY_CLOCK == 1 && USE_CLOCK == 1)
      if (overlayEnabled && overlayAllowed()) {
        if (!gamemodeFlag && modeCode != MC_TEXT) clockOverlayWrap(CLOCK_X, CLOCK_Y);
        if (loadFlag2) {
          setOverlayColors();
          loadFlag2 = false;
        }
      }
#endif
      loadingFlag = false;
    }
  } else {
    customModes();
  }
  FastLED.show();
  btnsModeChange();
  checkIdleState();
}

void checkIdleState() {

#if (SMOOTH_CHANGE == 1)
  modeFader();
#endif

  
  if (idleState) {
    if (fullTextFlag && SHOW_TEXT_ONCE) {
      fullTextFlag = false;
      autoplayTimer = millis();
      nextMode();
    }
    
    if (millis() - autoplayTimer > autoplayTime && AUTOPLAY) {    // таймер смены режима
      if (modeCode == MC_TEXT && SHOW_FULL_TEXT) {    // режим текста
        if (fullTextFlag) {
          fullTextFlag = false;
          autoplayTimer = millis();
          nextMode();
        }
      } else {
        autoplayTimer = millis();
        nextMode();
      }
    }
  } else {
    if (idleTimer.isReady()) {      // таймер холостого режима
      idleState = true;
      autoplayTimer = millis();
      gameDemo = true;

      gameSpeed = DEMO_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);

      loadingFlag = true;
      BTcontrol = false;
      FastLED.clear();
      FastLED.show();
    }
  }  
}

#if (MCU_TYPE == 1)
void weatherRequest() {
  if (init_time == 0) {
    return;
  }
  Serial.println("Weather request...");
  client.stop();
  
  if (client.connect(server, 80)) {
    Serial.println("Connected to port 80");
    //client.println("GET /data/2.5/weather?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&lang=ru HTTP/1.1");
    client.println("GET /data/2.5/forecast?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&cnt=2&lang=ru HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    Serial.println("Weather request sent");
    weather_t = millis();
  }
  else {
    Serial.println("Weather server connection failed!");
  }
}

void parseWeather() {
  char c = 0;
  text = "";
  while (client.available()) {
    c = client.read();
    if (c == '{') {
      startJson = true;
      jsonend++;
    }
    if (c == '}') {
      jsonend--;
    }
    if (startJson == true) {
      text += c;
    }
    if (jsonend == 0 && startJson == true) {
      parseJson(text.c_str());
      init_weather = 1;
      weather_t = 0;
      startJson = false;
    }
  }
}

void parseJson(const char * jsonString) {
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.parseObject(jsonString);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  JsonArray& list = root["list"];
  JsonObject& nowT = list[0];
  JsonObject& later = list[1];

  String city = root["city"]["name"];

  float tempNow = nowT["main"]["temp"];
  float humidityNow = nowT["main"]["humidity"];
  float windspeedNow = nowT["wind"]["speed"];
  float pressureNow = nowT["main"]["pressure"];
  pressureNow = pressureNow * HPaTomm;
  String weatherNow = nowT["weather"][0]["description"];

  float tempLater = later["main"]["temp"];
  float humidityLater = later["main"]["humidity"];
  float windspeedLater = later["wind"]["speed"];
  float pressureLater = later["main"]["pressure"];
  pressureLater = pressureLater * HPaTomm;
  String weatherLater = later["weather"][0]["description"];
  
  // Формируем строку с сообщением о погоде
  runningText = "Погода сейчас:"+weatherNow+" температура:"+ String(tempNow,1) +"C влажность:"+String(humidityNow,0)+"% ветер:"+String(windspeedNow,1)+"м/с давление:"+String(pressureNow,0)+"мм.рт.ст.";
  Serial.println(runningText);
}

#endif

#if (MCU_TYPE != 1)
void timeSet(boolean type, boolean dir) {    // type: 0-часы, 1-минуты, dir: 0-уменьшить, 1-увеличить
  if (type) {
    if (dir) hrs++;
    else hrs--;
  } else {
    if (dir) mins++;
    else mins--;
    if (mins > 59) {
      mins = 0;
      hrs++;
    }
    if (mins < 0) {
      mins = 59;
      hrs--;
    }
  }
  if (hrs > 23) hrs = 0;
  if (hrs < 0) hrs = 23;
}
#endif

void btnsModeChange() {
#if (USE_BUTTONS == 1)
  if (bt_set.clicked()) {
    if (gamemodeFlag) gameDemo = !gameDemo;
    if (gameDemo) {
      gameSpeed = DEMO_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);
      AUTOPLAY = true;
    } else {
      gameSpeed = D_GAME_SPEED;
      gameTimer.setInterval(gameSpeed);
      AUTOPLAY = false;
    }
  }
  if (bt_set.holded()) {
    if (modeCode == MC_GAME)
      mazeMode = !mazeMode;
    if (modeCode == MC_CLOCK) {    // вход в настройку часов
      clockSet = !clockSet;
      AUTOPLAY = false;
      secs = 0;
#if (USE_CLOCK == 1 && USE_RTC == 1)
      if (!clockSet) rtc.adjust(DateTime(2014, 1, 21, hrs, mins, 0)); // установка нового времени в RTC
#endif
    }
  }

  // timeSet type: 0-часы, 1-минуты, dir: 0-уменьшить, 1-увеличить

  if (gameDemo) {
    if (bt_right.clicked()) {
      if (!clockSet) {
        autoplayTimer = millis();
        nextMode();
      } else {
#if (MCU_TYPE == 1)
        adjustTime(60);
#else
        timeSet(1, 1);
#endif
      }
    }

    if (bt_left.clicked()) {
      if (!clockSet) {
        autoplayTimer = millis();
        prevMode();
      } else {
#if (MCU_TYPE == 1)
        adjustTime(-60);
#else
        timeSet(1, 0);
#endif
      }
    }

    if (bt_up.clicked()) {
      if (!clockSet) {
        AUTOPLAY = true;
        autoplayTimer = millis();
      } else {
#if (MCU_TYPE == 1)
        adjustTime(3600);
#else
        timeSet(0, 1);
#endif
      }
    }
    if (bt_down.clicked()) {
      if (!clockSet) {
        AUTOPLAY = false;
      } else {
#if (MCU_TYPE == 1)
        adjustTime(-3600);
#else
        timeSet(0, 0);
#endif
      }
    }

    if (bt_right.holding())
      if (changeTimer.isReady()) {
        if (!clockSet) {
          effects_speed -= 2;
          if (effects_speed < 30) effects_speed = 30;
          effectTimer.setInterval(effects_speed);
        } else {
#if (MCU_TYPE == 1)
          adjustTime(60);
#else
          timeSet(1, 1);
#endif
        }
      }
    if (bt_left.holding())
      if (changeTimer.isReady()) {
        if (!clockSet) {
          effects_speed += 2;
          if (effects_speed > 300) effects_speed = 300;
          effectTimer.setInterval(effects_speed);
        } else {
#if (MCU_TYPE == 1)
          adjustTime(-60);
#else
          timeSet(1, 0);
#endif
        }
      }
    if (bt_up.holding())
      if (changeTimer.isReady()) {
        if (!clockSet) {
          globalBrightness += 2;
          if (globalBrightness > 255) globalBrightness = 255;
          fadeBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);
        } else {
#if (MCU_TYPE == 1)
          adjustTime(3600);
#else
          timeSet(0, 1);
#endif
        }
      }
    if (bt_down.holding())
      if (changeTimer.isReady()) {
        if (!clockSet) {
          globalBrightness -= 2;
          if (globalBrightness < 0) globalBrightness = 0;
          fadeBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);
        } else {
#if (MCU_TYPE == 1)
          adjustTime(-3600);
#else
          timeSet(0, 0);
#endif
        }
      }
  }
#endif
}
