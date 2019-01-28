// вкладка работы с bluetooth

#if (BT_MODE == 1 || WIFI_MODE == 1)
#define PARSE_AMOUNT 4    // максимальное количество значений в массиве, который хотим получить
#define header '$'        // стартовый символ
#define divider ' '       // разделительный символ
#define ending ';'        // завершающий символ

boolean runningFlag;
boolean gameFlag;
boolean gamePaused;
boolean drawingFlag;
boolean effectsFlag;
byte game;
byte effect;
byte intData[PARSE_AMOUNT];                     // массив численных значений после парсинга
uint32_t prevColor;
boolean recievedFlag;
byte lastMode = 0;
boolean parseStarted;
String pictureLine;

char incomeBuffer[UDP_TX_PACKET_MAX_SIZE];      // Буфер для приема строки команды из wifi udp сокета
char replyBuffer[20];                           // ответ WiFi-клиенту - подтверждения получения команды

unsigned long ackCounter = 0;

void bluetoothRoutine() {
  parsing();                                    // принимаем данные

  // на время принятия данных матрицу не обновляем!
  if (!parseStarted) {                          

#if (MCU_TYPE == 1 && WIFI_MODE == 1)
    if (WifiTimer.isReady()) {
      if (ntp_t > 0 && millis() - ntp_t > 3000) { 
        ntp_t = 0;
      }
      if (wifi_connected && (NTPCheck.isReady() || (init_time == 0 && ntp_t == 0))) {
        getNTP();
      }
    }
#endif    

    // Ручное управление из Android-программы
    if (BTcontrol) {
      
      if (runningFlag) {                         // бегущая строка - Running Text
        String text = runningText;
        if (text == "") {
          #if (USE_CLOCK == 1)          
             text = clockCurrentText();
          #else
             text = "Gyver Matrix";
          #endif           
        }
        fillString(text, globalColor); 
      }
      if (gameFlag && (!gamePaused|| loadingFlag)) games();       // игры
      if (effectsFlag) effects();                 // эффекты
      
      if (runningFlag && !effectsFlag)  {         // Включенная бегущая строка только формирует строку в массиве точек матрицы, но не отображает ее
        FastLED.show();                           // Если эффекты выключены - нужно принудительно вызывать отображение матрицы
      }
      
    } else  {      
      // Автоматический режим - все режимы по циклу

      // Бегущая строка (0,1,2) или Часы в основном режиме и эффект Дыхание или Цвета, Радуга пикс
      if (effectsFlag && isColorEffect(effect) && (thisMode < 3 || thisMode == 27)) { 
        // Подготовить изображение
        customModes();
        // Наложить эффект Дыхание / Цвета и вывести в матрицу
        effects();
        checkIdleState();
      } else {
        // Сформировать и вывести на матрицу текущий демо-режим
        customRoutine();        
      }
    }    
  }
}

// блок эффектов, работают по общему таймеру
void effects() {
  
  // Только эффекты 0, 1 и 5 совместимы с бегущей строкой - они меняют цвет букв
  // Остальные эффекты портят бегущую строку - ее нужно отключать  
  if (runningFlag && !isColorEffect(effect)) runningFlag = false;  // Дыхание, Цвет, Радуга пикс
    
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

  // Для игр отключаем бегущую строку и эффекты
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

// разбор строки картинки - команда $5
char *pch;
int pntX, pntY, pntColor, pntIdx;
char buf[14];               // точка картинки FFFFFF XXX YYY
String pntPart[WIDTH];      // массив разобранной входной строки на строки точек



// ********************* ПРИНИМАЕМ ДАННЫЕ **********************
void parsing() {
// ****************** ОБРАБОТКА *****************
  String str, color, text;
  byte b_tmp;
  /*
    Протокол связи, посылка начинается с режима. Режимы:
    0 - отправка цвета $0 colorHEX;
    1 - отправка координат точки $1 X Y;
    2 - заливка - $2;
    3 - очистка - $3;
    4 - яркость - $4 value;
    5 - картинка построчно $5 Y colorHEX X|colorHEX X|...|colorHEX X;
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
    15 - скорость $15 скорость таймер; 0 - таймер эффектов, 1 - таймер скроллинга текста 2 - таймер игр
    16 - Режим смены эффектов: $16 value; N:  0 - Autoplay on; 1 - Autoplay off; 2 - PrevMode; 3 - NextMode
    17 - Время автосмены эффектов: $17 сек;
    18 - Запрос текущих параметров программой: $18 page;  page: 1 - настройки; 2 - рисование; 3 - картинка; 4 - текст; 5 - эффекты; 6 - игра; 7 - часы; 8 - о приложении 
  */  
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;

    // Режимы 16,17,18  на сбрасывают idleTimer
    if (intData[0] < 16 || intData[0] > 18) {
      idleTimer.reset();
      idleState = false;
    }

    switch (intData[0]) {
      case 0:
        if (!runningFlag) drawingFlag = true;
        sendAcknowledge();
        break;
      case 1:
        BTcontrol = true;
        if (!(drawingFlag || gameFlag)) {
            FastLED.clear();
        }
        drawingFlag = true;
        runningFlag = false;
        if (gameFlag && game==1) gamePaused = true;
        else {
          gameFlag = false;
          gamePaused = false;
        }
        if (!isColorEffect(effect)) {
            effectsFlag = false;
        }
        drawPixelXY(intData[1], intData[2], gammaCorrection(globalColor));
        FastLED.show();
        sendAcknowledge();
        break;
      case 2:
        BTcontrol = true;
        runningFlag = false;
        drawingFlag = true;
        gameFlag = false;
        gamePaused = false;
        if (!isColorEffect(effect)) {
            effectsFlag = false;
        }
        fillAll(gammaCorrection(globalColor));
        FastLED.show();
        sendAcknowledge();
        break;
      case 3:
        BTcontrol = true;
        runningFlag = false;
        gameFlag = false;
        gamePaused = false;
        drawingFlag = true;
        if (!isColorEffect(effect)) {
            effectsFlag = false;
        }
        FastLED.clear();
        FastLED.show();
        sendAcknowledge();
        break;
      case 4:
        globalBrightness = intData[1];
        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);
        FastLED.show();
        sendAcknowledge();
        break;
      case 5:
        BTcontrol = true;

        if (!drawingFlag) {
          FastLED.clear(); 
        }
        
        effectsFlag = false;        
        runningFlag = false;
        gameFlag = false;
        drawingFlag = true;
        loadingFlag = false;

        // строка картинки - в pictureLine в формате Y colorHEX X|colorHEX X|...|colorHEX X

        b_tmp = pictureLine.indexOf(" ");
        str = pictureLine.substring(0, b_tmp);
        pntY = str.toInt();
        pictureLine = pictureLine.substring(b_tmp+1);
        
        pictureLine.toCharArray(incomeBuffer, pictureLine.length()+1);
        pch = strtok (incomeBuffer,"|");
        pntIdx = 0;
        while (pch != NULL)
        {
          pntPart[pntIdx++] = String(pch);
          pch = strtok (NULL, "|");
        }
        
        for (int i=0; i<WIDTH; i++) {
          str = pntPart[i];
          str.toCharArray(buf, str.length()+1);

          pntColor=HEXtoInt(String(strtok(buf," ")));
          pntX=atoi(strtok(NULL," "));

          // начало картинки - очистить матрицу
          if ((pntX == 0) && (pntY == HEIGHT - 1)) {
            FastLED.clear(); 
          }
          
          drawPixelXY(pntX, pntY, gammaCorrection(pntColor));
        }
        FastLED.show();

        // Подтвердить прием строки изображения
        str = "$5 " + String(pntY) + " ack" + String(ackCounter++) + ";\r\n";
        
#if (BT_MODE == 1)
        // После отправки команды из Андроид-программы, она ждет подтверждения получения"
        if (fromBT) {
          Serial.println(str);
        }
#endif
#if (WIFI_MODE == 1)
        if (fromWiFi) { 
          str.toCharArray(incomeBuffer, str.length()+1);    
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          udp.write(incomeBuffer);
          udp.endPacket();
          delay(0);
        }  
#endif
        // так как Acknowledge не отправляется, а вместо этогоо тправляется ответная посылка,
        // флаги откуда получена команда сбрасываем здесь. Для других команд - сбрасываются в sendAcknowledge()
        fromWiFi = false;
        fromBT = false;
        break;
      case 6:
        loadingFlag = true;
        // строка принимается в переменную runningText
        sendAcknowledge();
        break;
      case 7:
        BTcontrol = true;
        if (intData[1] == 1) runningFlag = true;
        if (intData[1] == 0) runningFlag = false;
        if (runningFlag) {
          gameFlag = false;
          drawingFlag = false;
          if (!isColorEffect(effect)) {
            effectsFlag = false;
          }
        }
        sendAcknowledge();
        break;
      case 8:
        if (intData[1] == 0) {
          
          effect = intData[2];
          gameFlag = false;
          loadingFlag = !isColorEffect(effect);
          effectsFlag = true;
          globalSpeed = intData[3];
          if (!BTcontrol) BTcontrol = !isColorEffect(effect);     // При установек эффекта дыхание / цвета / радуга пикс - переключаться в управление по BT не нужно
          if (!isColorEffect(effect)) drawingFlag = false;
          
        } else if (intData[1] == 1) {
          effectsFlag = intData[2] == 1;
        }
        
        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);    // возвращаем яркость
        effectTimer.setInterval(globalSpeed);
        sendAcknowledge();
        break;
      case 9:        
        BTcontrol = true;        
        if (!drawingFlag || runningFlag) {        // начать новую игру при переходе со всех режимов кроме рисования
          loadingFlag = true;    
          FastLED.clear(); 
          FastLED.show(); 
        }
        effectsFlag = false;
        runningFlag = false;
        controlFlag = false;                      // Посе начала игры пока не трогаем кнопки - игра автоматическая 
        drawingFlag = false;
        gameFlag = true;
        gamePaused = true;
        game = intData[1];
        gameTimer.setInterval(gameSpeed);
        sendAcknowledge();
        break;        
      case 10:
        BTcontrol = true;        
        buttons = 0;
        controlFlag = true;
        gameFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 11:
        BTcontrol = true;
        buttons = 1;
        controlFlag = true;
        gameFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 12:
        BTcontrol = true;
        buttons = 2;
        controlFlag = true;
        gameFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 13:
        BTcontrol = true;
        buttons = 3;
        controlFlag = true;
        gameFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 14:
        BTcontrol = true;
        if (!gameFlag || runningFlag) {
          loadingFlag = true;    
          FastLED.clear(); 
          FastLED.show(); 
        }
        gameFlag = true;
        effectsFlag = false;
        gamePaused = intData[1] == 0;  
        sendAcknowledge();
        break;
      case 15: 
        if (intData[2] == 0) {
          globalSpeed = intData[1];          
          effectTimer.setInterval(globalSpeed);
        } else if (intData[2] == 1) {
          scrollSpeed = intData[1];
          scrollTimer.setInterval(scrollSpeed);
        } else if (intData[2] == 2) {
          gameSpeed = map(intData[1],0,255,25,375);      // для игр скорость нужна меньше!
          gameTimer.setInterval(gameSpeed);
        }
        sendAcknowledge();
        break;
      case 16:
        BTcontrol = intData[1] == 1;
        if (intData[1] == 0) AUTOPLAY = true;
        else if (intData[1] == 1) AUTOPLAY = false;
        else if (intData[1] == 2) prevMode();
        else if (intData[1] == 3) nextMode();
        else if (intData[1] == 4) AUTOPLAY = intData[2] == 1;

        idleState = !BTcontrol && AUTOPLAY; 
        if (AUTOPLAY) {
          autoplayTimer = millis(); // При включении автоматического режима сбросить таймер автосмены режимов
        }

        // Если при переключении в ручной режим был демонстрационный режим бегущей строки - включить ручной режим бегщей строки
        if (intData[1] == 0 || (intData[1] == 1 && thisMode < 3)) {
          loadingFlag = true;
          runningFlag = true;          
        }
        sendAcknowledge();
        break;
      case 17: autoplayTime = ((long)intData[1] * 1000);
        idleState = !BTcontrol && AUTOPLAY; 
        if (AUTOPLAY) {
          autoplayTimer = millis();
        }
        sendAcknowledge();
        break;
      case 18: 
        // W:число    ширина матрицы
        // H:число    высота матрицы
        // DM:Х       демо режим, где Х = 0 - выкл (ручное управление); 1 - вкл
        // AP:Х       автосменарежимов, где Х = 0 - выкл; 1 - вкл
        // PD:число   продолжительность режима в секундах
        // BR:число   яркость
        // CL:HHHHHH  текущий цвет рисования, HEX
        // TX:[текст] текст, ограничители [] обязательны
        // TS:Х       состояние бегущей строки, где Х = 0 - выкл; 1 - вкл
        // ST:число   скорость прокрутки текста
        // EF:число   текущий эффект
        // ES:Х       состояние эффектов, где Х = 0 - выкл; 1 - вкл
        // SE:число   скорость эффектов
        // GM:число   текущая игра
        // GS:Х       состояние игры, где Х = 0 - выкл; 1 - вкл
        // SG:число   скорость игры
        str = "";
        switch (intData[1]) { 
          case 0:  // Проверка связи          
            sendAcknowledge();
            break;
          case 1:  // Настройки. Вернуть: Ширина/Высота матрицы; Яркость; Деморежм и Автосмена; Время смены режимо
            str="$18 W:"+String(WIDTH)+";H:"+String(HEIGHT)+";DM:";
            if (BTcontrol) str+="0;AP:"; else str+="1;AP:";
            if (AUTOPLAY)  str+="1;BR:"; else str+="0;BR:";
            str+=String(globalBrightness) + ";PD:" + String(autoplayTime / 1000) + ";";
            break;
          case 2:  // Рисование. Вернуть: Яркость; Цвет точки;
            color = ("000000" + String(globalColor, HEX));
            color = color.substring(color.length() - 6); // FFFFFF             
            str="$18 BR:"+String(globalBrightness) + ";CL:" + color + ";";
            break;
          case 3:  // Картинка. Вернуть: Яркость;
            str="$18 BR:"+String(globalBrightness) + ";";
            break;
          case 4:  // Текст. Вернуть: Яркость; Скорость текста; Вкл/Выкл; Текст
            text = runningText;
            text.replace(";","~");
            str="$18 BR:"+String(globalBrightness) + ";ST:" + String(scrollSpeed) + ";ST:";
            if (runningFlag)  str+="1;TX:["; else str+="0;TX:[";
            str += text + "]" + ";";
            break;
          case 5:  // Эффекты. Вернуть: Номер эффекта, Остановлен или играет; Яркость; Скорость эффекта 
            str="$18 EF:"+String(effect+1) + ";ES:";
            if (effectsFlag)  str+="1;BR:"; else str+="0;BR:";
            str+=String(globalBrightness) + ";SE:" + String(globalSpeed) + ";";
            break;
          case 6:  // Игры. Вернуть: Номер игры; Вкл.выкл; Яркость; Скорость игры
            str="$18 GM:"+String(game+1) + ";GS:";
            if (gameFlag && !gamePaused)  str+="1;BR:"; else str+="0;BR:";
            str+=String(globalBrightness) + ";SG:" + String(constrain(map(gameSpeed, 25, 375, 0, 255), 0,255)) + ";"; 
            break;
          case 7:  // Настройки часов. Вернуть:
            Serial.println("Настройки часов");
            break;
        }
        
        if (str.length()>0) {
          str += "\r\n";
          // Отправить клиенту запрошенные параметры страницы / режимов
#if (BT_MODE == 1)
          // После отправки команды из Андроид-программы, она ждет подтверждения получения - ответ "ack;"
          Serial.println(str);
#endif
#if (WIFI_MODE == 1)
          str.toCharArray(incomeBuffer, str.length()+1);    
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          udp.write(incomeBuffer);
          udp.endPacket();
          delay(0);
#endif
          // Для режимов $18 кроме 0 - Acknowledge не отправляется, вместо этогоотправляется ответная посылка
          // Поэтому флаги откуда получена команда сбрасываем здесь. Для других команд - сбрасываются в sendAcknowledge()
          fromWiFi = false;
          fromBT = false;
        }
        else
          sendAcknowledge();
        break;
    }
    lastMode = intData[0];  // запомнить предыдущий режим
  }

  // ****************** ПАРСИНГ *****************
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
        if (udp.remotePort() == localPort) {
          Serial.print("Содержимое: ");
          Serial.println(incomeBuffer);
        }
      }
#endif
      // NTP packet from time server
      if (udp.remotePort() == 123) {
        parseNTP();
        haveIncomeData = 0;
      }
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
          if (intData[0] == 5) {  // строка картинки
            pictureLine = String(&incomeBuffer[bufIdx]);
          } if (intData[0] == 6) {  // текст бегщей строки
            runningText = String(&incomeBuffer[bufIdx]);
          }
                    
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

  // Если есть не разобранный буфер от WiFi сокета - данные BT пока не разбираем
  // Кроме команд от BT, еслион не подключен - можновводить команды в монитор порта
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
  
  if (haveIncomeData) {

    if (parseStarted) {                                             // если приняли начальный символ (парсинг разрешён)
      if (incomingByte != divider && incomingByte != ending) {      // если это не пробел И не конец
        string_convert += incomingByte;                             // складываем в строку
      } else {                                                      // если это пробел или ; конец пакета
        if (parse_index == 0) {
          byte cmdMode = string_convert.toInt();
          if (cmdMode == 0) parseMode = COLOR;                      // передача цвета (в отдельную переменную)
          else if (cmdMode == 6 || cmdMode == 5) {
            parseMode = TEXT;
          }
          else parseMode = NORMAL;
          // if (cmdMode != 7 || cmdMode != 0) runningFlag = false;
        }

        if (parse_index == 1) {       // для второго (с нуля) символа в посылке
          if (parseMode == NORMAL) intData[parse_index] = string_convert.toInt();             // преобразуем строку в int и кладём в массив}
          if (parseMode == COLOR) {                                                           // преобразуем строку HEX в цифру
            globalColor = (uint32_t)HEXtoInt(string_convert);           
            if (intData[0] == 0) {
              if (runningFlag && effectsFlag) effectsFlag = false;   
              incomingByte = ending;
              parseStarted = false;
              BTcontrol = true;
            }
          }
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
    }

    if (bufIdx >= packetSize) {                     // Весь буфер разобран 
      bufIdx = 0;
      packetSize = 0;
    }
  }
}

void sendAcknowledge() {
#if (BT_MODE == 1)
  // После отправки команды из Андроид-программы, она ждет подтверждения получения - ответ "ack;"
  if (fromBT) {
    Serial.println("ack" + String(ackCounter++) + ";\r\n");
  }
#endif
#if (WIFI_MODE == 1)
  // Если данные были получены по WiFi - отправить подтверждение, чтобы клиентский сокет прервал ожидание
  if (fromWiFi) {
    String reply = "ack" + String(ackCounter++) + ";\r\n";
    reply.toCharArray(replyBuffer, reply.length()+1);    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(replyBuffer);
    udp.endPacket();
    delay(0);
  }
  fromWiFi = false;
  fromBT = false;
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

bool isColorEffect(byte effect) {
  // Цветовые эффекты - Дыхание, Цвет или Радуга пикс.
  // Они могут работать с custom демо режимами
  return (effect >= 0 && effect <= 1) || effect == 5;
}

#else
void bluetoothRoutine() {
  return;
}
#endif
