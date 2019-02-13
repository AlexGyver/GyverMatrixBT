#if (BT_MODE == 1 || USE_WIFI == 1)
#define PARSE_AMOUNT 4       // максимальное количество значений в массиве, который хотим получить
#define header '$'           // стартовый символ
#define divider ' '          // разделительный символ
#define ending ';'           // завершающий символ

#if (USE_WIFI == 1)
int16_t intData[PARSE_AMOUNT];  // массив численных значений после парсинга - для WiFi часы время синхр м.б отрицательным + 
                                // период синхронизации м.б больше 255 сек - нужен тип int16_t В MCU_TYPE == 1 памяти много, можно не экономить
#else                           // в ардуино TZ и ТЕЗ yt используются - принимаемые значения байты - экономим память
byte intData[PARSE_AMOUNT];     // массив численных значений после парсинга
#endif

uint32_t prevColor;
boolean recievedFlag;
byte lastMode = 0;
boolean parseStarted;
String pictureLine;

#if (USE_WIFI==1)
char incomeBuffer[UDP_TX_PACKET_MAX_SIZE];        // Буфер для приема строки команды из wifi udp сокета
#else
char incomeBuffer[18];                            // Буфер для разбора строки команды передачи картинки из BT-соединения
#endif
char replyBuffer[7];                              // ответ клиенту - подтверждения получения команды: "ack;/r/n/0"

unsigned long ackCounter = 0;

void bluetoothRoutine() {  
  parsing();                                    // принимаем данные

  // на время принятия данных матрицу не обновляем!
  if (!parseStarted) {                          

    #if (MCU_TYPE == 1 && USE_WIFI == 1 && (USE_CLOCK == 1 || GET_WEATHER == 1))
      if (WifiTimer.isReady() && wifi_connected) {
  
        #if (USE_CLOCK == 1)    
          if (useNtp) {
            if (ntp_t > 0 && millis() - ntp_t > 3000) {
            #if (BT_MODE == 0)  
              Serial.println("NTP request timeout!");
            #endif  
              init_time = 0;
              ntp_t = 0;
            }
            if (ntpTimer.isReady() || (init_time == 0 && ntp_t == 0)) {
              getNTP();
            }
          }
        #endif
      
        #if (GET_WEATHER == 1)    
          if (weather_t > 0 && millis() - weather_t > 5000) {
            init_weather = 0;
            weather_t = 0;
            #if (BT_MODE == 0)  
            Serial.println("Weather request timeout!");
            #endif
            client.stop();
          }
          if (weather_t > 0) {
            parseWeather();
          }
          if (WeatherCheck.isReady() || (init_weather == 0 && weather_t == 0)) {
            weatherRequest();
          }
        #endif
      }
    #endif    

    if (!BTcontrol && effectsFlag && !isColorEffect(effect)) effectsFlag = false;

    if (runningFlag) {                         // бегущая строка - Running Text
      String text = runningText;
      if (text == "") {
        #if (USE_CLOCK == 1)          
           text = init_time ==0 
             ? clockCurrentText()
             : clockCurrentText() + " " + dateCurrentTextLong();  // + dateCurrentTextShort()
        #else
           text = "Gyver Matrix";
        #endif           
      }
      fillString(text, globalColor); 
      // Включенная бегущая строка только формирует строку в массиве точек матрицы, но не отображает ее
      // Если эффекты выключены - нужно принудительно вызывать отображение матрицы
      if (!effectsFlag) 
        FastLED.show();
      else if (isColorEffect(effect)) 
        effects();   
    }

    else if (drawingFlag) {
      // Рисование. Если эффект цветов - применить
      if (effectsFlag && isColorEffect(effect)) {  
         effects();   
      }
    }
    
    // Один из режимов игры. На игры эффекты не налагаются
    else if (gamemodeFlag && (!gamePaused || loadingFlag)) {
      // Для игр отключаем бегущую строку и эффекты
      effectsFlag = false;
      runningFlag = false;
      customRoutine();        
    }

    // Бегущая строка (0,1,2) или Часы в основном режиме и эффект Дыхание или Цвета, Радуга пикс
    else if ((thisMode < 3 || thisMode == 27) && effectsFlag && isColorEffect(effect)) { 
      // Подготовить изображение
      customModes();
      // Наложить эффект Дыхание / Цвета и вывести в матрицу
      effects();
    } else {
      // Сформировать и вывести на матрицу текущий демо-режим
      if (!BTcontrol || effectsFlag) 
        customRoutine();
      else if (BTcontrol && effectsFlag && isColorEffect(effect)) {
        effects();  
      }
    }            

    // Проверить - если долгое время не было ручного управления - переключиться в автоматический режим
    checkIdleState();
  }
}

// Блок эффектов наложения цветовых эффектов на сформированное изображение
void effects() {
  
  // Эффекты наложения цвета на изображение, имеющееся на матрице
  // Только эффекты 0, 1 и 5 совместимы с бегущей строкой - они меняют цвет букв
  // Остальные эффекты портят бегущую строку - ее нужно отключать  
  if (runningFlag && !isColorEffect(effect)) runningFlag = false;  // Дыхание, Цвет, Радуга пикс
    
  if (effectTimer.isReady()) {
    switch (effect) {
      case 0: brightnessRoutine();         // Дыхание
        break;
      case 1: colorsRoutine();             // Цвет
        break;
      case 5: rainbowColorsRoutine();      // Радуга пикс.
        break;
    }
    FastLED.show();
  }
}

byte parse_index;
String string_convert = "";
enum modes {NORMAL, COLOR, TEXT} parseMode;

bool fromWiFi = false;
bool fromBT = false;
bool haveIncomeData = false;
char incomingByte;

#if (MCU_TYPE == 1)
int  bufIdx = 0;         // В MCU_TYPE == 1 могут приниматься пакеты > 255 байт
int  packetSize = 0;
#else
byte  bufIdx = 0;
byte  packetSize = 0;
#endif

// разбор строки картинки - команда $5
char *pch;
int pntX, pntY, pntColor, pntIdx;
char buf[14];               // точка картинки FFFFFF XXX YYY
String pntPart[WIDTH];      // массив разобранной входной строки на строки точек

// ********************* ПРИНИМАЕМ ДАННЫЕ **********************
void parsing() {
// ****************** ОБРАБОТКА *****************
  String str;
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
      - $8 1 N X старт/стоп; N - номер эффекта, X=0 - стоп X=1 - старт 
    9 - игра
    10 - кнопка вверх
    11 - кнопка вправо
    12 - кнопка вниз
    13 - кнопка влево
    14 - пауза в игре
    15 - скорость $15 скорость таймер; 0 - таймер эффектов, 1 - таймер скроллинга текста 2 - таймер игр
    16 - Режим смены эффектов: $16 value; N:  0 - Autoplay on; 1 - Autoplay off; 2 - PrevMode; 3 - NextMode
    17 - Время автосмены эффектов и бездействия: $17 сек сек;
    18 - Запрос текущих параметров программой: $18 page;  page: 1 - настройки; 2 - рисование; 3 - картинка; 4 - текст; 5 - эффекты; 6 - игра; 7 - часы; 8 - о приложении 
    19 - работа с настройками часов
  */  
  if (recievedFlag) {      // если получены данные
    recievedFlag = false;

    // Режимы 16,17,18  не сбрасывают idleTimer
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
        drawingFlag = true;
        runningFlag = false;
        if (gamemodeFlag && game==1) gamePaused = true;
        else {
          gamemodeFlag = false;
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
        gamemodeFlag = false;
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
        gamemodeFlag = false;
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
        saveMaxBrightness(globalBrightness);
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
        gamemodeFlag = false;
        drawingFlag = true;
        loadingFlag = false;

        // строка картинки - в pictureLine в формате:  для UDP:  'Y colorHEX X|colorHEX X|...|colorHEX X'
        //                                             для BT будет один пакет 'colorHEX X Y' из за ограничения размера приемного буфера

        if (fromWiFi) {
          // Разбираем СТРОКУ из принятого буфера формата 'Y colorHEX X|colorHEX X|...|colorHEX X'
          // Получить номер строки (Y) для которой получили строку с данными 
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
          
          for (int i=0; i<pntIdx; i++) {
            str = pntPart[i];
            str.toCharArray(buf, str.length()+1);
  
            pntColor=HEXtoInt(String(strtok(buf," ")));
            pntX=atoi(strtok(NULL," "));
  
            // начало картинки - очистить матрицу
            if ((pntX == 0) && (pntY == HEIGHT - 1)) {
              FastLED.clear(); 
              FastLED.show();
            }
            
            drawPixelXY(pntX, pntY, gammaCorrection(pntColor));
          }
  
          // Выводить построчно для ускорения вывода на экран
          if (pntX == WIDTH - 1)
            FastLED.show();
  
          // Подтвердить прием строки изображения
          str = "$5 " + String(pntY)+ "-" + String(pntX) + " ack" + String(ackCounter++) + ";";
    
  #if (USE_WIFI == 1)
          if (fromWiFi) { 
            str.toCharArray(incomeBuffer, str.length()+1);    
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write(incomeBuffer);
            udp.endPacket();
            delay(0);
          }  
  #endif
        }

        if (fromBT) {
          // Разбираем массив принятых параметров intData[2]..intData[3], где
          // intData[2] - координата X
          // intData[3] - координата Y
          // Цвет точки - в globalColor
          pntX = intData[2];
          pntY = intData[3];
          
          // начало картинки - очистить матрицу
          if ((pntX == 0) && (pntY == HEIGHT - 1)) {
            FastLED.clear(); 
            FastLED.show();
          }

          drawPixelXY(pntX, pntY, gammaCorrection(globalColor));
  
          // Выводить построчно для ускорения вывода на экран
          if (pntX == WIDTH - 1)
            FastLED.show();
  
          // Подтвердить прием строки изображения
          str = "$5 " + String(pntY)+ "-" + String(pntX) + " ack" + String(ackCounter++) + ";";
  
  #if (BT_MODE == 1)
          // После отправки команды из Андроид-программы, она ждет подтверждения получения"
          if (fromBT) {
            Serial.println(str);
          }
  #endif  
        }
        
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
          gamemodeFlag = false;
          drawingFlag = false;
          if (!isColorEffect(effect)) {
            effectsFlag = false;
          }
        }
        sendAcknowledge();
        break;
      case 8:
        effect = intData[2];
        gamemodeFlag = false;
        loadingFlag = !isColorEffect(effect);
        effectsFlag = true;
        if (!BTcontrol) BTcontrol = !isColorEffect(effect);     // При установке эффекта дыхание / цвета / радуга пикс - переключаться в управление по BT не нужно
        if (!isColorEffect(effect)) {
          drawingFlag = false;
          runningFlag = false;
        }
        
        effectSpeed = getEffectSpeed(effect);
        effectTimer.setInterval(effectSpeed);
                
        // intData[1] : дейстие -> 0 - выбор эффекта  = 1 - старт/стоп
        // intData[2] : номер эффекта
        // intData[3] : 0 - стоп 1 - старт
        effectsFlag = intData[1] == 0 || (intData[1] == 1 && intData[3] == 1); // выбор эффекта - сразу запускать
        
        // Найти соответствие thisMode указанному эффекту. 
        // Дльнейшее отображение изображения эффекта будет выполняться стандартной процедурой customRoutin()
        if (!isColorEffect(effect)) {            
           b_tmp = mapEffectToMode(effect);
           if (b_tmp != 255) thisMode = b_tmp;
        }

        breathBrightness = globalBrightness;
        FastLED.setBrightness(globalBrightness);    
        
        // Для "0" - отправляются параметры, подтверждение отправлять не нужно. Для остальных - нужно
        if (intData[1] == 0) {
          sendPageParams(5);
        } else { 
          sendAcknowledge();
        }
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
        gamemodeFlag = true;
        gamePaused = true;
        game = intData[1];
        
        // Найти соответствие thisMode указанной игре. 
        // Дльнейшее отображение изображения эффекта будет выполняться стандартной процедурой customRoutin()
        b_tmp = mapGameToMode(game);
        if (b_tmp != 255) thisMode = b_tmp;

        gameSpeed = getGameSpeed(game);
        gameTimer.setInterval(gameSpeed);        
        // Отправить программе актуальное состояние параметров эффектов (6 - страница "Игры")
        sendPageParams(6);
        break;        
      case 10:
        BTcontrol = true;        
        buttons = 0;
        controlFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 11:
        BTcontrol = true;
        buttons = 1;
        controlFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 12:
        BTcontrol = true;
        buttons = 2;
        controlFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 13:
        BTcontrol = true;
        buttons = 3;
        controlFlag = true;
        gamePaused = false;
        sendAcknowledge();
        break;
      case 14:
        BTcontrol = true;
        b_tmp = mapGameToMode(game);
        if (b_tmp != 255) {
          thisMode = b_tmp;
          if (!drawingFlag || (drawingFlag && game != 0) || runningFlag) {        // начать новую игру при переходе со всех режимов кроме рисования
            loadingFlag = true;                                                   // если игра в паузе змейка - продолжить, иначе начать  новую игру 
            FastLED.clear(); 
            FastLED.show(); 
          }
          effectsFlag = false;
          drawingFlag = false;
          runningFlag = false;
          gamemodeFlag = true;
          gamePaused = intData[1] == 0;  
          gameSpeed = getGameSpeed(game);
          gameTimer.setInterval(gameSpeed);        
        }
        sendAcknowledge();
        break;
      case 15: 
        if (intData[2] == 0) {
          effectSpeed = intData[1]; 
          saveEffectSpeed(effect, effectSpeed);
          effectTimer.setInterval(effectSpeed);
        } else if (intData[2] == 1) {
          scrollSpeed = intData[1]; 
          scrollTimer.setInterval(scrollSpeed);
          saveScrollSpeed(scrollSpeed);
        } else if (intData[2] == 2) {
          gameSpeed = map(constrain(intData[1],0,255),0,255,D_GAME_SPEED_MIN,D_GAME_SPEED_MAX);      // для игр скорость нужна меньше! вх 0..255 преобразовать в 25..375
          saveGameSpeed(game, gameSpeed);
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
        saveAutoplay(AUTOPLAY);

        if (!BTcontrol) {
          runningFlag = false;
          controlFlag = false;      // После начала игры пока не трогаем кнопки - игра автоматическая 
          drawingFlag = false;
          gamemodeFlag = false;
          gamePaused = false;
          loadingFlag = true;       // если false - при переключении с эффекта бегущий текст на демо-режим "бегущий текст" текст демо режима не сначала, а с позиции где бежал текст эффекта
                                    // если true - текст начинает бежать сначала, потом плавно затухает на смену режима и потом опять начинает сначала.
                                    // И так и так не хорошо. Как починить? 
        } else {
          // Если при переключении в ручной режим был демонстрационный режим бегущей строки - включить ручной режим бегщей строки
          if (intData[1] == 0 || (intData[1] == 1 && thisMode < 3)) {
            loadingFlag = true;
            runningFlag = true;          
          }
        }

        if (!BTcontrol && AUTOPLAY) {
          sendPageParams(1);
        } else {        
          sendAcknowledge();
        }
        
        break;
      case 17: 
        autoplayTime = ((long)intData[1] * 1000);   // секунды -> миллисек 
        idleTime = ((long)intData[2] * 60 * 1000);  // минуты -> миллисек
        saveAutoplayTime(autoplayTime);
        saveIdleTime(idleTime);
        idleState = !BTcontrol && AUTOPLAY; 
        if (AUTOPLAY) {
          autoplayTimer = millis();
        }
        if (idleState) {
          if (idleTime == 0) // тамймер отключен
            idleTimer.setInterval(4294967295);
          else
            idleTimer.setInterval(idleTime);
          idleTimer.reset();
        }
        sendAcknowledge();
        break;
      case 18: 
        if (intData[1] == 0)  // ping
          sendAcknowledge();
        else {                // запрос параметров страницы приложения
          sendPageParams(intData[1]);
          saveSettings();
        }
        break;
      case 19: 
#if (USE_CLOCK == 1)
         switch (intData[1]) {
           case 0:               // $19 0 N X; - сохранить настройку X "Часы в эффекте" для эффекта N
             saveEffectClock(intData[2], intData[3] == 1);
             break;
           case 1:               // $19 1 X; - сохранить настройку X "Часы в эффектах"
             saveClockOverlayEnabled(intData[2] == 1);
             break;
           case 2:               // $19 2 X; - Использовать синхронизацию часов NTP  X: 0 - нет, 1 - да
#if (USE_WIFI == 1)
             useNtp = intData[2] == 1;
             saveUseNtp(useNtp);
             init_time = 0; ntp_t = 0;
#endif             
             break;
           case 3:               // $19 3 N Z; - Период синхронизации часов NTP и Часовой пояс
#if (USE_WIFI == 1)
             SYNC_TIME_PERIOD = intData[2];
             timeZoneOffset = (int8_t)intData[3];
             saveTimeZone(timeZoneOffset);
             saveNtpSyncTime(SYNC_TIME_PERIOD);
             saveTimeZone(timeZoneOffset);
             ntpTimer.setInterval(1000 * 60 * SYNC_TIME_PERIOD);
             init_time = 0; ntp_t = 0;
#endif             
             break;
           case 4:               // $19 4 X; - Ориентация часов  X: 0 - горизонтально, 1 - вертикально
             CLOCK_ORIENT = intData[2] == 1 ? 1  : 0;
             // Центрируем часы по горизонтали/вертикали по ширине / высоте матрицы
             if (CLOCK_ORIENT == 0) {
               CLOCK_X = CLOCK_X_H;
               CLOCK_Y = CLOCK_Y_H;
             } else {
               CLOCK_X = CLOCK_X_V;
               CLOCK_Y = CLOCK_Y_V;
             }
             if (CLOCK_X < 0) CLOCK_X = 0;
             if (CLOCK_Y < 0) CLOCK_Y = 0;
             saveClockOrientation(CLOCK_ORIENT);
             break;
           case 5:               // $19 5 X; - Режим цвета часов  X: 0 - горизонтально, 1 - вертикально
             COLOR_MODE = intData[2];
             if (COLOR_MODE > 3) COLOR_MODE = 0;
             saveClockColorMode(COLOR_MODE);
             break;
        }
        sendAcknowledge();
#endif        
        break;
    }
    lastMode = intData[0];  // запомнить предыдущий режим
  }

  // ****************** ПАРСИНГ *****************
  haveIncomeData = false;

#if (USE_WIFI == 1)
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
#endif
      }

#if (USE_CLOCK == 1)
      // NTP packet from time server
      if (udp.remotePort() == 123) {
        parseNTP();
        haveIncomeData = 0;
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
          if (intData[0] == 5) {  // строка картинки
            pictureLine = String(&incomeBuffer[bufIdx]);
          } if (intData[0] == 6) {  // текст бегущей строки
            runningText = String(&incomeBuffer[bufIdx]);
            runningText.trim();
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
      fromBT = true;
      if (parseMode == TEXT) {              // если нужно принять строку
        str = Serial.readString();          // принимаем всю
        if (intData[0] == 6) {            // текст бегущей строки
          runningText = str;
          runningText.trim();
          if (runningText == ".") runningText = "";
        }

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
          intData[0] = cmdMode;
          if (cmdMode == 0 || (fromBT && cmdMode == 5)) parseMode = COLOR;                      // передача цвета (в отдельную переменную)
          else if (cmdMode == 6 || (fromWiFi && cmdMode == 5)) {
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
            } else {
              parseMode = NORMAL;
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

void sendPageParams(int page) {
  // W:число    ширина матрицы
  // H:число    высота матрицы
  // DM:Х       демо режим, где Х = 0 - выкл (ручное управление); 1 - вкл
  // AP:Х       автосменарежимов, где Х = 0 - выкл; 1 - вкл
  // PD:число   продолжительность режима в секундах
  // IT:число   время бездействия в секундах
  // BR:число   яркость
  // CL:HHHHHH  текущий цвет рисования, HEX
  // TX:[текст] текст, ограничители [] обязательны
  // TS:Х       состояние бегущей строки, где Х = 0 - выкл; 1 - вкл
  // ST:число   скорость прокрутки текста
  // EF:число   текущий эффект
  // ES:Х       состояние эффектов, где Х = 0 - выкл; 1 - вкл
  // EC:X       оверлей часов для эффекта вкл/выкл, где Х = 0 - выкл; 1 - вкл
  // SE:число   скорость эффектов
  // GM:число   текущая игра
  // GS:Х       состояние игры, где Х = 0 - выкл; 1 - вкл
  // SG:число   скорость игры
  // CE:X       оверлей часов вкл/выкл, где Х = 0 - выкл; 1 - вкл
  // CС:X       режим цвета часов: 0,1,2
  // NP:Х       использовать NTP, где Х = 0 - выкл; 1 - вкл
  // NT:число   период синхронизации NTP в минутах
  // NZ:число   часовой пояс -12..+12
  String str = "", color, text;
  boolean allowed;
  byte b_tmp;
  switch (page) { 
    case 1:  // Настройки. Вернуть: Ширина/Высота матрицы; Яркость; Деморежм и Автосмена; Время смены режимо
      str="$18 W:"+String(WIDTH)+"|H:"+String(HEIGHT)+"|DM:";
      if (BTcontrol) str+="0|AP:"; else str+="1|AP:";
      if (AUTOPLAY)  str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness) + "|PD:" + String(autoplayTime / 1000) + "|IT:" + String(idleTime / 60 / 1000) +  ";";
      break;
    case 2:  // Рисование. Вернуть: Яркость; Цвет точки;
      color = ("000000" + String(globalColor, HEX));
      color = color.substring(color.length() - 6); // FFFFFF             
      str="$18 BR:"+String(globalBrightness) + "|CL:" + color + ";";
      break;
    case 3:  // Картинка. Вернуть: Яркость;
      str="$18 BR:"+String(globalBrightness) + ";";
      break;
    case 4:  // Текст. Вернуть: Яркость; Скорость текста; Вкл/Выкл; Текст
      text = runningText;
      text.replace(";","~");
      str="$18 BR:"+String(globalBrightness) + "|ST:" + String(constrain(map(scrollSpeed, D_TEXT_SPEED_MIN,D_TEXT_SPEED_MAX, 0, 255), 0,255)) + "|ST:";
      if (runningFlag)  str+="1|TX:["; else str+="0|TX:[";
      str += text + "]" + ";";
      break;
    case 5:  // Эффекты. Вернуть: Номер эффекта, Остановлен или играет; Яркость; Скорость эффекта; Оверлей часов 
      allowed = false;
#if (USE_CLOCK == 1 && OVERLAY_CLOCK == 1)      
      b_tmp = mapEffectToMode(effect);
      if (b_tmp != 255) {
        for (byte i = 0; i < sizeof(overlayList); i++) {
          allowed = (b_tmp == overlayList[i]);
          if (allowed) break;
        }
      }
#endif      
      str="$18 EF:"+String(effect+1) + "|ES:";
      if (effectsFlag)  str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness) + "|SE:" + String(constrain(map(effectSpeed, D_EFFECT_SPEED_MIN,D_EFFECT_SPEED_MAX, 0, 255), 0,255));
#if (USE_CLOCK == 1)      
      if (isColorEffect(effect) || !allowed || effect == 9) 
          str+="|EC:X;";  // X - параметр не используется (неприменим)
      else    
          str+="|EC:" + String(getEffectClock(effect)) + ";";
#else
      str+="|EC:X;";  // X - параметр не используется (неприменим)
#endif      
      break;
    case 6:  // Игры. Вернуть: Номер игры; Вкл.выкл; Яркость; Скорость игры
      str="$18 GM:"+String(game+1) + "|GS:";
      if (gamemodeFlag && !gamePaused)  str+="1|BR:"; else str+="0|BR:";
      str+=String(globalBrightness) + "|SG:" + String(constrain(map(gameSpeed, D_GAME_SPEED_MIN,D_GAME_SPEED_MAX, 0, 255), 0,255)) + ";"; 
      break;
    case 7:  // Настройки часов. Вернуть: Оверлей вкл/выкл
#if (USE_CLOCK == 1)      
      str="$18 CE:"+String(getClockOverlayEnabled()) + "|CC:" + String(COLOR_MODE)
#if (USE_WIFI == 1)      
      + "|NP:"; 
      if (useNtp)  str+="1|NT:"; else str+="0|NT:";
      str+=String(SYNC_TIME_PERIOD) + "|NZ:" + String(timeZoneOffset)  
#endif      
      + ";";
#endif      
      break;
  }
  
  if (str.length() > 0) {
    // Отправить клиенту запрошенные параметры страницы / режимов
#if (BT_MODE == 1)
    // После отправки команды из Андроид-программы, она ждет подтверждения получения - ответ "ack;"
    Serial.println(str);
#endif
#if (USE_WIFI == 1)
    str.toCharArray(incomeBuffer, str.length()+1);    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(incomeBuffer);
    udp.endPacket();
    delay(0);
#endif
  } else {
    sendAcknowledge();
  }
}

void sendAcknowledge() {
#if (BT_MODE == 1)
  // После отправки команды из Андроид-программы, она ждет подтверждения получения - ответ "ack;"
  if (fromBT) {
    Serial.println("ack" + String(ackCounter++) + ";");
  }
#endif
#if (USE_WIFI == 1)
  // Если данные были получены по WiFi - отправить подтверждение, чтобы клиентский сокет прервал ожидание
  if (fromWiFi) {
    String reply = "ack" + String(ackCounter++) + ";";
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
  customRoutine();
}
#endif

byte mapEffectToMode(byte effect) {
  byte tmp_mode = 255;
  
  switch (effect) {
    case 2:  tmp_mode = 12; break;       // snowRoutine();
    case 3:  tmp_mode = 16; break;       // ballRoutine();
    case 4:  tmp_mode = 18; break;       // rainbowRoutine();
    case 6:  tmp_mode = 20; break;       // fireRoutine()
    case 7:  tmp_mode = 14; break;       // matrixRoutine();
    case 8:  tmp_mode = 17; break;       // ballsRoutine();
    case 9:  tmp_mode = 27; break;       // clockRoutine();
    case 10: tmp_mode = 15; break;       // starfallRoutine()
    case 11: tmp_mode = 13; break;       // sparklesRoutine()
    case 12: tmp_mode = 19; break;       // rainbowDiagonalRoutine();
    case 13: tmp_mode = 3;  break;       // madnessNoise();
    case 14: tmp_mode = 4;  break;       // cloudNoise();
    case 15: tmp_mode = 5;  break;       // lavaNoise();
    case 16: tmp_mode = 6;  break;       // plasmaNoise();
    case 17: tmp_mode = 7;  break;       // rainbowNoise();
    case 18: tmp_mode = 8;  break;       // rainbowStripeNoise();
    case 19: tmp_mode = 9;  break;       // zebraNoise();
    case 20: tmp_mode = 10; break;       // forestNoise();
    case 21: tmp_mode = 11; break;       // oceanNoise();
    case 22: tmp_mode = 28; break;       // animation();

    // Нет соответствия - выполняются для текущего режима thisMode
    case 0:  // Дыхание
    case 1:  // Цвет
    case 5:  // Радуга пикс
      break;
  }

  return tmp_mode;
}

byte mapGameToMode(byte game) {
  byte tmp_mode = 255;
  
  switch (game) {
    case 0: tmp_mode = 21; break;  // snakeRoutine(); 
    case 1: tmp_mode = 22; break;  // tetrisRoutine();
    case 2: tmp_mode = 23; break;  // mazeRoutine();
    case 3: tmp_mode = 24; break;  // runnerRoutine();
    case 4: tmp_mode = 25; break;  // flappyRoutine();
    case 5: tmp_mode = 26; break;  // arkanoidRoutine();
  }

  return tmp_mode;
}

byte mapModeToEffect(byte aMode) {
  byte tmp_effect = 255;
  // Если режима нет в списке - ему нет соответствия среди эффектов - значит это игра или бегущий текст
  switch (aMode) {
    case 3:  tmp_effect = 13; break;       // madnessNoise();
    case 4:  tmp_effect = 14; break;       // cloudNoise();
    case 5:  tmp_effect = 15; break;       // lavaNoise();
    case 6:  tmp_effect = 16; break;       // plasmaNoise();
    case 7:  tmp_effect = 17; break;       // rainbowNoise();
    case 8:  tmp_effect = 18; break;       // rainbowStripeNoise();
    case 9:  tmp_effect = 19; break;       // zebraNoise();
    case 10: tmp_effect = 20; break;       // forestNoise();
    case 11: tmp_effect = 21; break;       // oceanNoise();
    case 12: tmp_effect = 2;  break;       // snowRoutine();
    case 13: tmp_effect = 11; break;       // sparklesRoutine()
    case 14: tmp_effect = 7;  break;       // matrixRoutine();
    case 15: tmp_effect = 10; break;       // starfallRoutine()
    case 16: tmp_effect = 3;  break;       // ballRoutine();
    case 17: tmp_effect = 8;  break;       // ballsRoutine();
    case 18: tmp_effect = 4;  break;       // rainbowRoutine();
    case 19: tmp_effect = 12; break;       // rainbowDiagonalRoutine();
    case 20: tmp_effect = 6;  break;       // fireRoutine()
    case 28: tmp_effect = 22; break;       // animation();

    case 0:  break;  // Бегущий текст
    case 1:  break;  // Бегущий текст
    case 2:  break;  // Бегущий текст

    case 21: break;  // snakeRoutine(); 
    case 22: break;  // tetrisRoutine();
    case 23: break;  // mazeRoutine();
    case 24: break;  // runnerRoutine();
    case 25: break;  // flappyRoutine();
    case 26: break;  // arkanoidRoutine();
    
    case 27: break;  // clockRoutine();     
  }
  return tmp_effect;
}

byte mapModeToGame(byte aMode) {
  byte tmp_game = 255;
  // Если режима нет в списке - ему нет соответствия среди тгр - значит это эффект или бегущий текст
  switch (aMode) {
    case 3:  break;  // madnessNoise();
    case 4:  break;  // cloudNoise();
    case 5:  break;  // lavaNoise();
    case 6:  break;  // plasmaNoise();
    case 7:  break;  // rainbowNoise();
    case 8:  break;  // rainbowStripeNoise();
    case 9:  break;  // zebraNoise();
    case 10: break;  // forestNoise();
    case 11: break;  // oceanNoise();
    case 12: break;  // snowRoutine();
    case 13: break;  // sparklesRoutine()
    case 14: break;  // matrixRoutine();
    case 15: break;  // starfallRoutine()
    case 16: break;  // ballRoutine();
    case 17: break;  // ballsRoutine();
    case 18: break;  // rainbowRoutine();
    case 19: break;  // rainbowDiagonalRoutine();
    case 20: break;  // fireRoutine()
    case 28: break;  // animation();

    case 0:  break;  // Бегущий текст
    case 1:  break;  // Бегущий текст
    case 2:  break;  // Бегущий текст

    case 21: tmp_game = 0; break;  // snakeRoutine(); 
    case 22: tmp_game = 1; break;  // tetrisRoutine();
    case 23: tmp_game = 2; break;  // mazeRoutine();
    case 24: tmp_game = 3; break;  // runnerRoutine();
    case 25: tmp_game = 4; break;  // flappyRoutine();
    case 26: tmp_game = 5; break;  // arkanoidRoutine();
    
    case 27: break;  // clockRoutine();     
  }
  return tmp_game;
}
