// вкладка работы с bluetooth

#if (BT_MODE == 1 || WIFI_MODE == 1)
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
byte intData[PARSE_AMOUNT];                     // массив численных значений после парсинга
uint32_t prevColor;
boolean recievedFlag;
byte lastMode = 0;
boolean parseStarted;

#if (WIFI_MODE == 1)
char incomeBuffer[UDP_TX_PACKET_MAX_SIZE];      // Буфер для приема строки команды из wifi udp сокета
char replyBuffer[] = "ack\r\n";                 // ответ WiFi-клиенту - подтверждения получения команды
#endif

void bluetoothRoutine() {
  parsing();                                    // принимаем данные

  if (!parseStarted && BTcontrol) {             // на время принятия данных матрицу не обновляем!
    if (runningFlag) fillString(runningText, globalColor);   // бегущая строка
    if (gameFlag) games();                      // игры
    if (effectsFlag) effects();                 // эффекты
  }
}

// блок эффектов, работают по общему таймеру
void effects() {
  
  // Только эффекты 0 и 1 совместимы с бегущей строкой - они меняют цвет букв
  // Остальные эффекты портят бегущую строку - ее нужно отключать  
  if (runningFlag && effect > 1) runningFlag = false;
    
  if (effectTimer.isReady()) {
    switch (effect) {
      case 0: brightnessRoutine();
        break;
      case 1: colorsRoutine();
        break;
      case 2: snowRoutine();
        break;
      case 3: ballRoutine();
        break;
      case 4: rainbowRoutine();
        break;
      case 5: rainbowColorsRoutine();
        break;
      case 6: fireRoutine();
        break;
      case 7: matrixRoutine();
        break;
      case 8: ballsRoutine();
        break;
      case 9: clockRoutine();
        break;
      case 10: starfallRoutine();
        break;
      case 11: sparklesRoutine();
        break;
      case 12: rainbowDiagonalRoutine();
        break;
      case 13: madnessNoise();
        break;
      case 14: cloudNoise();
        break;
      case 15: lavaNoise();
        break;
      case 16: plasmaNoise();
        break;
      case 17: rainbowNoise();
        break;
      case 18: rainbowStripeNoise();
        break;
      case 19: zebraNoise();
        break;
      case 20: forestNoise();
        break;
      case 21: oceanNoise();
        break;
    }
    FastLED.show();
  }
}

// блок игр
void games() {

  // Lkz игр отключаем бегущую строку и эффекты
  if (effectsFlag) effectsFlag = false;
  if (runningFlag) runningFlag = false;
  
  switch (game) {
    case 0:
      snakeRoutine();
      break;
    case 1:
      tetrisRoutine();
      break;
    case 2:
      mazeRoutine();
      break;
    case 3:
      runnerRoutine();
      break;
    case 4:
      flappyRoutine();
      break;
    case 5:
      arkanoidRoutine();
      break;
  }
}

byte parse_index;
String string_convert = "";
enum modes {NORMAL, COLOR, TEXT} parseMode;

bool fromWiFi = false;
bool fromBT = false;
bool haveIncomeData = false;
char incomingByte;
int  bufIdx = 0;
int  packetSize = 0;

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
    15 - скорость $15 скорость;
    16 - Режим смены эффектов: $16 value; N:  0 - Autoplay on; 1 - Autoplay off; 2 - PrevMode; 3 - NextMode
    17 - Время фвтосмены эффектов: $17 сек;
  */
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;

    if (intData[0] != 16) {
      idleTimer.reset();
      idleState = false;

      if (!BTcontrol) {
        gameSpeed = globalSpeed * 4;
        gameTimer.setInterval(gameSpeed);
        BTcontrol = true;
      }
    }

    switch (intData[0]) {
      case 1:
        if (runningFlag) runningFlag = false;
        if (gameFlag && game > 0) gameFlag = false;  
        if (effectsFlag && effect > 1) effectsFlag = false;
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 2:
        if (gameFlag) gameFlag = false;  
        if (effectsFlag && effect > 1) effectsFlag = false;
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        break;
      case 3:
        if (runningFlag) runningFlag = false;
        if (gameFlag) gameFlag = false;  
        if (effectsFlag && effect > 1) effectsFlag = false;
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
        if (runningFlag) runningFlag = false;
        if (gameFlag) gameFlag = false;  
        if (effectsFlag && effect > 1) effectsFlag = false;
        drawPixelXY(intData[2], intData[3], gammaCorrection(globalColor));
        // делаем обновление матрицы каждую строчку, чтобы реже обновляться
        // и не пропускать пакеты данных (потому что отправка на большую матрицу занимает много времени)
        if (prevY != intData[3] || ( (intData[3] == 0) && (intData[2] == WIDTH - 1) ) ) {
          prevY = intData[3];
          FastLED.show();
        }
        break;
      case 6:
        loadingFlag = true;
        // строка принимается в переменную runningText
        break;
      case 7:
        if (intData[1] == 1) runningFlag = true;
        if (intData[1] == 0) runningFlag = false;
        if (runningFlag) {
          gameFlag = false;
          effectsFlag =false;
        }
        break;
      case 8:
        if (intData[1] == 0) {
          effect = intData[2];
          if (effect>1) runningFlag = true;
          gameFlag = false;
          loadingFlag = true;
          breathBrightness = globalBrightness;
          FastLED.setBrightness(globalBrightness);    // возвращаем яркость
          globalSpeed = intData[3];
          gameTimer.setInterval(globalSpeed * 4);
        }
        else if (intData[1] == 1) {
          effectsFlag = !effectsFlag;
          if (effectsFlag) gameFlag = false;
          if (effectsFlag && effect > 1) runningFlag = false;
        }        
        break;
      case 9:
        if (lastMode != 1) loadingFlag = true;    
        effectsFlag = false;
        runningFlag = false;        
        game = intData[1];
        globalSpeed = intData[2];
        gameSpeed = globalSpeed * 4;
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
          gameSpeed = globalSpeed * 4;      // для игр скорость нужно меньше!
          gameTimer.setInterval(gameSpeed);
        }
        if (effectsFlag) effectTimer.setInterval(globalSpeed);
        if (runningFlag) scrollTimer.setInterval(globalSpeed);
        break;
      case 16:
        if (intData[1] == 0) AUTOPLAY = true;
        else if (intData[1] == 1) AUTOPLAY = false;
        else if (intData[1] == 2) prevMode();
        else if (intData[1] == 3) nextMode();
        break;
      case 17: autoplayTime = ((long)intData[1] * 1000);
        autoplayTimer = millis();
        break;
    }
    lastMode = intData[0];  // запомнить предыдущий режим
  }

  // ****************** ПАРСИНГ *****************
  fromWiFi = false;
  fromBT = false;
  haveIncomeData = false;

#if (WIFI_MODE == 1)
    if (!haveIncomeData) {

      // Если предыдущий буфер еще не разобран - новых данных из сокета не читаем, продолжаем разбор уже считанного буфера
      haveIncomeData = bufIdx > 0 && bufIdx < packetSize; 
      if (!haveIncomeData) {
        packetSize = udp.parsePacket();      
        haveIncomeData = packetSize > 0;      
      
        if (haveIncomeData) {                
          // read the packet into packetBufffer
          int len = udp.read(incomeBuffer, UDP_TX_PACKET_MAX_SIZE);
          if (len > 0) {
            fromWiFi = true;
            incomeBuffer[len] =0;
          }
          bufIdx = 0;
          
          delay(0);            // ESP8266 при вызове delay отпрабатывает стек IP протокола, дадим ему поработать        

          // Если управление через BT включено - Serial для коммуникации через BT,
          // если выключено - используем для вывода диагностики в монитор порта  
#if (BT_MODE == 0)
          Serial.print("UDP пакeт размером ");
          Serial.print(packetSize);
          Serial.print(" от ");
          IPAddress remote = udp.remoteIP();
          for (int i = 0; i < 4; i++) {
            Serial.print(remote[i], DEC);
            if (i < 3) {
              Serial.print(".");
            }
          }
          Serial.print(", port ");
          Serial.println(udp.remotePort());
          Serial.print("Содержимое: ");
          Serial.println(incomeBuffer);
        }
#endif
      }

      if (haveIncomeData) {         
        if (parseMode == TEXT) {                         // если нужно принять строку - принимаем всю
            // Из за ошибки в компоненте UdpSender в Thunkable - теряются половина отправленных 
            // символов, если их кодировка - двухбайтовый UTF8, т.к. оно вычисляет длину строки без учета двухбайтовости
            // Чтобы символы не терялись - при отправке строки из андроид-программы, она добивается с конца пробелами
            // Здесь эти конечные пробелы нужно предварительно удалить
            while (packetSize > 0 && incomeBuffer[packetSize-1] == ' ') packetSize--;
            incomeBuffer[packetSize] = 0;
  
            // Оставшийся буфер преобразуем с строку
            runningText = String(&incomeBuffer[bufIdx+1]); // 
                      
            incomingByte = ending;                       // сразу завершаем парс
            parseMode = NORMAL;
            bufIdx = 0; 
            packetSize = 0;                              // все байты из входящего пакета обработаны
          } else {
            incomingByte = incomeBuffer[bufIdx++];       // обязательно ЧИТАЕМ входящий символ
        } 
      }       
    }
#endif

#if (BT_MODE == 1)
    // Если есть не разобранный буфер от WiFi сокета - данные BT пока не разбираем
    if (!haveIncomeData) {
      haveIncomeData = Serial.available() > 0;
      if (haveIncomeData) {
        true;
        fromBT = true;
        if (parseMode == TEXT) {              // если нужно принять строку
          runningText = Serial.readString();  // принимаем всю
          incomingByte = ending;              // сразу завершаем парс
          parseMode = NORMAL;
        } else {
          incomingByte = Serial.read();        // обязательно ЧИТАЕМ входящий символ
        }
      }
    }
#endif
  
  if (haveIncomeData) {

    if (parseStarted) {                                             // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != divider && incomingByte != ending) {      // если это не пробел И не конец
        string_convert += incomingByte;                             // складываем в строку
      } else {                                                      // если это пробел или ; конец пакета
        if (parse_index == 0) {
          byte thisMode = string_convert.toInt();
          if (thisMode == 0 || thisMode == 5) parseMode = COLOR;    // передача цвета (в отдельную переменную)
          else if (thisMode == 6) parseMode = TEXT;
          else parseMode = NORMAL;
        //if (thisMode != 7 || thisMode != 0) runningFlag = false;
        }

        if (parse_index == 1) {       // для второго (с нуля) символа в посылке
          if (parseMode == NORMAL) intData[parse_index] = string_convert.toInt();             // преобразуем строку в int и кладём в массив}
        //if (parseMode == COLOR) globalColor = strtol(&string_convert[0], NULL, 16);         // преобразуем строку HEX в цифру
          if (parseMode == COLOR) globalColor = (uint32_t)HEXtoInt(string_convert);           // преобразуем строку HEX в цифру
        } else {
          intData[parse_index] = string_convert.toInt();  // преобразуем строку в int и кладём в массив
        }
        string_convert = "";                        // очищаем строку
        parse_index++;                              // переходим к парсингу следующего элемента массива
      }
    }

    if (incomingByte == header) {                   // если это $
      parseStarted = true;                          // поднимаем флаг, что можно парсить
      parse_index = 0;                              // сбрасываем индекс
      string_convert = "";                          // очищаем строку
    }

    if (incomingByte == ending) {                   // если таки приняли ; - конец парсинга
      parseMode == NORMAL;
      parseStarted = false;                         // сброс
      recievedFlag = true;                          // флаг на принятие
      loadingFlag = false;                          // 
    }

    if (bufIdx >= packetSize) {                     // Весь буфер разобран 
      bufIdx = 0;
      packetSize = 0;
    }
  }
  
#if (WIFI_MODE == 1)
  // Если данные былиполучены по WiFi - отправить подтверждение, чтобы клиентский сокет прервал ожидание
  if (fromWiFi) {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(replyBuffer);
    udp.endPacket();
  }
#endif

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
