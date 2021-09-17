// вкладка работы с bluetooth

#if (BT_MODE == 1)
#define PARSE_AMOUNT 4    // максимальное количество значений в массиве, который хотим получить
#define header '$'        // стартовый символ
#define divider ' '       // разделительный символ
#define ending ';'        // завершающий символ

byte prevY = 0;
byte prevX = 0;
boolean runningFlag;
boolean gameFlag;
boolean effectsFlag;
byte game;
byte effect;
byte intData[PARSE_AMOUNT];     // массив численных значений после парсинга
uint32_t prevColor;
boolean recievedFlag;
byte lastMode = 0;
boolean parseStarted;

void bluetoothRoutine() {
  parsing();                           // принимаем данные

  if (!parseStarted && BTcontrol) {                // на время принятия данных матрицу не обновляем!

    if (runningFlag) fillString(runningText, globalColor);   // бегущая строка
    if (gameFlag) games();                      // игры
    if (effectsFlag) effects();                 // эффекты
    filter();
    FastLED.show();
  }
}

// блок эффектов, работают по общему таймеру
void effects() {
  if (effectTimer.isReady()) {
    switch (effect) {
      case 1 : rainbowRoutine();               break;
      case 2 : rainbowDiagonalRoutine();       break;
      case 3 : fireRoutine();                  break;
      case 4 : snowRoutine();                  break;
      case 5 : matrixRoutine();                break;
      case 6 : ballRoutine();                  break;
      case 7 : ballsRoutine();                 break;
      case 8 : starfallRoutine();              break;
      case 9 : sparklesRoutine();              break;
      case 10: patternsRoutine();              break;
      case 11: TLandRoutine();                 break;
      case 12: madnessNoise();                 break;
      case 13: cloudNoise();                   break;
      case 14: lavaNoise();                    break;
      case 15: plasmaNoise();                  break;
      case 16: rainbowNoise();                 break;
      case 17: zebraNoise();                   break;
      case 18: forestNoise();                  break;
      case 19: oceanNoise();                   break;
      case 20: clockRoutine();                 break;
      case 21: animation1();                   break;
    }
  }
}

// блок игр
void games() {
  switch (game) {
    case 0: snakeRoutine();    break;
    case 1: tetrisRoutine();   break;
    case 2: mazeRoutine();     break;
    case 3: runnerRoutine();   break;
    case 4: flappyRoutine();   break;
    case 5: arkanoidRoutine(); break;
    case 6: runRoutine();      break;
  }
}

byte parse_index;
String string_convert = "";
enum modes {NORMAL, COLOR, TEXT} parseMode;


// ********************* ПРИНИМАЕМ ДАННЫЕ **********************
void parsing() {
  // ****************** ОБРАБОТКА *****************
  /*
    Протокол связи, посылка начинается с режима. Режимы:
    0 - отправка цвета $0 colorHEX;
    1 - отправка координат точки $1 X Y;
    2 - заливка - $2;
    3 - очистка - $3;
    4 - яркость - $4 value;
    5 - картинка $5 colorHEX X Y;
    6 - текст $6 some text
    7 - управление текстом: $7 1; пуск, $7 0; стоп
    8 - эффект
      - $8 0 номерЭффекта;
      - $8 1 старт/стоп;
    9 - игра
    10 - кнопка вверх
    11 - кнопка вправо
    12 - кнопка вниз
    13 - кнопка влево
    14 - пауза в игре
    15 - скорость $8 скорость;
    16 - Работа с Демо режимом
    17 - время смены ефектов
    18 - 1 Часы 2 Минуты
    19 - Фильтр(Дыхание, Радуга, Цвета)
  */
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;

    if (intData[0] != 16) {
      idleTimer.reset();
      idleState = false;

      if (!BTcontrol) {
        gameSpeed = (256 - globalSpeed) * 4;
        gameTimer.setInterval(gameSpeed);
        BTcontrol = true;
      }
    }

    switch (intData[0]) {
      case 1:
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 2:
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 3:
        FastLED.clear();
        FastLED.show();
        break;
      case 4:
        globalBrightness = intData[1];
        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
        FastLED.show();
        break;
      case 5:
        drawPixelXY(intData[2], intData[3], gammaCorrection(globalColor));
        // делаем обновление матрицы каждую строчку, чтобы реже обновляться
        // и не пропускать пакеты данных (потому что отправка на большую матрицу занимает много времени)
        if ((intData[3] == 0) && (intData[2] == WIDTH - 1)) {
          FastLED.show();
        }
        break;
      case 6:
        loadingFlag = true;
        // строка принимается в переменную runningText
        break;
      case 7:
        runningFlag = !runningFlag;
        break;
      case 8:
        if (intData[1] == 0) {
          effect = intData[2];
          gameFlag = false;
          loadingFlag = true;
          breathBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);    // возвращаем яркость
          globalSpeed = intData[3];
          gameTimer.setInterval((256 - globalSpeed) * 4);
        }
        else if (intData[1] == 1) effectsFlag = !effectsFlag;
        break;
      case 9:
        if (lastMode != 1) loadingFlag = true;    // начать новую игру при переходе со всех режимов кроме рисования
        effectsFlag = false;
        //gameFlag = true;
        game = intData[1];
        globalSpeed = intData[2];
        gameSpeed = (256 - globalSpeed) * 4;
        gameTimer.setInterval(gameSpeed);
        break;
      case 10:
        buttons = 0;
        controlFlag = true;
        break;
      case 11:
        buttons = 1;
        controlFlag = true;
        break;
      case 12:
        buttons = 2;
        controlFlag = true;
        break;
      case 13:
        buttons = 3;
        controlFlag = true;
        break;
      case 14:
        gameFlag = !gameFlag;
        break;
      case 15: globalSpeed = intData[1];
        if (gameFlag) {
          gameSpeed = (256 - globalSpeed) * 4;    // для игр скорость нужно меньше!
          gameTimer.setInterval(gameSpeed);
        }
        if (effectsFlag) effectTimer.setInterval(256 - globalSpeed);
        if (runningFlag) scrollTimer.setInterval(256 - globalSpeed);
        break;
      case 16:
        if (intData[1] == 0) {
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
        else if (intData[1] == 1) AUTOPLAY = !AUTOPLAY;
        else if (intData[1] == 2) prevMode();
        else if (intData[1] == 3) nextMode();
        break;
      case 17: autoplayTime = ((long)intData[1] * 1000);
        autoplayTimer = millis();
        break;
      case 18: hrs = intData[1]; mins = intData[2];
        break;
      case 19: variant = intData[1];
        break;
    }
    lastMode = intData[0];  // запомнить предыдущий режим
  }

  // ****************** ПАРСИНГ *****************
  if (Serial.available() > 0) {
    char incomingByte;
    if (parseMode == TEXT) {     // если нужно принять строку
      runningText = Serial.readString();  // принимаем всю
      incomingByte = ending;              // сразу завершаем парс
      parseMode = NORMAL;
    } else {
      incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ
    }
    if (parseStarted) {                         // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != divider && incomingByte != ending) {   // если это не пробел И не конец
        string_convert += incomingByte;       // складываем в строку
      } else {                                // если это пробел или ; конец пакета
        if (parse_index == 0) {
          byte thisMode = string_convert.toInt();
          if (thisMode == 0 || thisMode == 5) parseMode = COLOR;    // передача цвета (в отдельную переменную)
          else if (thisMode == 6) parseMode = TEXT;
          else parseMode = NORMAL;
          //if (thisMode != 7 || thisMode != 0) runningFlag = false;
        }

        if (parse_index == 1) {       // для второго (с нуля) символа в посылке
          if (parseMode == NORMAL) intData[parse_index] = string_convert.toInt();             // преобразуем строку в int и кладём в массив}
          //if (parseMode == COLOR) globalColor = strtol(&string_convert[0], NULL, 16);     // преобразуем строку HEX в цифру
          if (parseMode == COLOR) globalColor = (uint32_t)HEXtoInt(string_convert);     // преобразуем строку HEX в цифру
        } else {
          intData[parse_index] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
        }
        string_convert = "";                  // очищаем строку
        parse_index++;                              // переходим к парсингу следующего элемента массива
      }
    }
    if (incomingByte == header) {             // если это $
      parseStarted = true;                      // поднимаем флаг, что можно парсить
      parse_index = 0;                              // сбрасываем индекс
      string_convert = "";                    // очищаем строку
    }
    if (incomingByte == ending) {             // если таки приняли ; - конец парсинга
      parseMode == NORMAL;
      parseStarted = false;                     // сброс
      recievedFlag = true;                    // флаг на принятие
    }
  }
}


// hex string to uint32_t
uint32_t HEXtoInt(String hexValue) {
  byte tens, ones, number1, number2, number3;
  tens = (hexValue[0] < '9') ? hexValue[0] - '0' : hexValue[0] - '7';
  ones = (hexValue[1] < '9') ? hexValue[1] - '0' : hexValue[1] - '7';
  number1 = (16 * tens) + ones;

  tens = (hexValue[2] < '9') ? hexValue[2] - '0' : hexValue[2] - '7';
  ones = (hexValue[3] < '9') ? hexValue[3] - '0' : hexValue[3] - '7';
  number2 = (16 * tens) + ones;

  tens = (hexValue[4] < '9') ? hexValue[4] - '0' : hexValue[4] - '7';
  ones = (hexValue[5] < '9') ? hexValue[5] - '0' : hexValue[5] - '7';
  number3 = (16 * tens) + ones;

  return ((uint32_t)number1 << 16 | (uint32_t)number2 << 8 | number3 << 0);
}

#elif (BT_MODE == 0)
void bluetoothRoutine() {
  return;
}
#endif
