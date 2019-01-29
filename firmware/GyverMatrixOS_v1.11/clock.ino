// режим часов

// ****************** НАСТРОЙКИ ЧАСОВ *****************
#define OVERLAY_CLOCK 1     // часы на фоне всех эффектов и игр. Жрёт SRAM память!
#define CLOCK_ORIENT 0      // 0 горизонтальные, 1 вертикальные
#if (CLOCK_ORIENT == 0)
#define CLOCK_X 0           // позиция часов по X (начало координат - левый нижний угол)
#define CLOCK_Y 6           // позиция часов по Y (начало координат - левый нижний угол)
#else
#define CLOCK_X 5           // позиция часов по X (начало координат - левый нижний угол)
#define CLOCK_Y 3           // позиция часов по Y (начало координат - левый нижний угол)
#endif
#define COLOR_MODE 0        // Режим цвета часов
//                          0 - заданные ниже цвета
//                          1 - радужная смена (каждая цифра)
//                          2 - радужная смена (часы, точки, минуты)

#define MIN_COLOR CRGB::White          // цвет минут
#define HOUR_COLOR CRGB::White         // цвет часов
#define DOT_COLOR CRGB::White          // цвет точек
#define CONTRAST_COLOR CRGB::Orange    // контрастный цвет часов

#define HUE_STEP 5          // шаг цвета часов в режиме радужной смены
#define HUE_GAP 30          // шаг цвета между цифрами в режиме радужной смены

// ****************** ДЛЯ РАЗРАБОТЧИКОВ ****************
bool overlayEnabled = getClockOverlayEnabled();
byte listSize = sizeof(overlayList);
CRGB contrastColor = CONTRAST_COLOR;
byte clockHue;

#if (OVERLAY_CLOCK == 1 && CLOCK_ORIENT == 0)
CRGB overlayLEDs[75];
#elif (OVERLAY_CLOCK == 1 && CLOCK_ORIENT == 1)
CRGB overlayLEDs[70];
#endif

#if (USE_CLOCK == 1)
CRGB clockLED[5] = {HOUR_COLOR, HOUR_COLOR, DOT_COLOR, MIN_COLOR, MIN_COLOR};

#if (WIFI_MODE == 1 && USE_CLOCK == 1)
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
#if (BT_MODE == 0)  
  Serial.println("sending NTP packet...");
#endif  
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void parseNTP() {
#if (BT_MODE == 0)  
    Serial.println("Parsing NTP data");
#endif
    ntp_t = 0;
    init_time = 1;
    //udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(incomeBuffer[40], incomeBuffer[41]);
    unsigned long lowWord = word(incomeBuffer[42], incomeBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    unsigned long seventyYears = 2208988800UL ;
    //DateTime now = secsSince1900 - seventyYears + (timeZoneOffset + daylight) * 3600;
    
    time_t t = secsSince1900 - seventyYears + (timeZoneOffset) * 3600;
#if (BT_MODE == 0)  
    Serial.print("Seconds since 1970: ");
    Serial.println(t);
#endif
    setTime(t);
}

void getNTP() {
  if (!wifi_connected) {
    return;
  }
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  ntp_t = millis();
}
#endif

boolean overlayAllowed() {
  // Оверлей разрешен общими настройками часов?
  bool allowed = getClockOverlayEnabled();
  // Оверлей разрешен настройками списка разрешенных для оверлея эффектов?
  if (allowed) {
    for (byte i = 0; i < listSize; i++) {
      allowed = (modeCode == overlayList[i]);
      if (allowed) break;
    }
  }

#if (BT_MODE == 1 || WIFI_MODE == 1)  

  // Эффекты Дыхание, Цвета, Радуга пикс. и Часы, a также функция isColorEffect() есть только при включенных опциях BT_MODE или WIFI_MODE
  // Оверлей разрешен настройками параметров эффекта? (в эффектах Дыхание, Цвета, Радуга пикс. и Часы оверлей часов не разрешен)
  if (allowed && BTcontrol && effectsFlag && !(isColorEffect(effect) || effect == MC_CLOCK)) {
    allowed = getEffectClock(effect);   
  }
  
#endif
  
  // Если в режиме авто - найти соответствие номера отображаемого режима номеру эффекта и проверить - разрешен ли для него оверлей часов
  if (allowed && !BTcontrol) {
    byte tmp_effect = mapModeToEffect(thisMode);
    if (tmp_effect <= MAX_EFFECT) 
      allowed = getEffectClock(tmp_effect);   
  }

  return allowed;
}

String clockCurrentText() {

#if (USE_RTC == 1 && (MCU_TYPE == 0 || MCU_TYPE == 1))
    DateTime now = rtc.now();
    hrs = now.hour();
    mins = now.minute();
#else if (MCU_TYPE == 1) 
    hrs = hour();
    mins = minute();
#endif

  String sHrs = "0" + String(hrs);  
  String sMin = "0" + String(mins);
  if (sHrs.length() > 2) sHrs = sHrs.substring(1);
  if (sMin.length() > 2) sMin = sMin.substring(1);
  return sHrs + ":" + sMin;
}

void clockColor() {
  if (COLOR_MODE == 0) {
    clockLED[0] = MIN_COLOR;
    clockLED[1] = MIN_COLOR;
    clockLED[2] = DOT_COLOR;
    clockLED[3] = HOUR_COLOR;
    clockLED[4] = HOUR_COLOR;
  } else if (COLOR_MODE == 1) {
    for (byte i = 0; i < 5; i++) clockLED[i] = CHSV(clockHue + HUE_GAP * i, 255, 255);
    clockLED[2] = CHSV(clockHue + 128 + HUE_GAP * 1, 255, 255); // точки делаем другой цвет
  } else if (COLOR_MODE == 2) {
    clockLED[0] = CHSV(clockHue + HUE_GAP * 0, 255, 255);
    clockLED[1] = CHSV(clockHue + HUE_GAP * 0, 255, 255);
    clockLED[2] = CHSV(clockHue + 128 + HUE_GAP * 1, 255, 255); // точки делаем другой цвет
    clockLED[3] = CHSV(clockHue + HUE_GAP * 2, 255, 255);
    clockLED[4] = CHSV(clockHue + HUE_GAP * 2, 255, 255);
  }
}

// нарисовать часы
void drawClock(byte hrs, byte mins, boolean dots, byte X, byte Y) {
  if  (MCU_TYPE == 1) {
    hrs = hour();
    mins = minute();
  }
#if (CLOCK_ORIENT == 0)
  if (hrs > 9) drawDigit3x5(hrs / 10, X, Y, clockLED[0]);
  drawDigit3x5(hrs % 10, X + 4, Y, clockLED[1]);
  if (dots) {
    drawPixelXY(X + 7, Y + 1, clockLED[2]);
    drawPixelXY(X + 7, Y + 3, clockLED[2]);
  } else {
    if (modeCode == MC_CLOCK) {
      drawPixelXY(X + 7, Y + 1, 0);
      drawPixelXY(X + 7, Y + 3, 0);
    }
  }
  drawDigit3x5(mins / 10, X + 8, Y, clockLED[3]);
  drawDigit3x5(mins % 10, X + 12, Y, clockLED[4]);
#else // Вертикальные часы
  //if (hrs > 9) // Так реально красивей
  drawDigit3x5(hrs / 10, X, Y + 6, clockLED[0]);
  drawDigit3x5(hrs % 10, X + 4, Y + 6, clockLED[1]);
  if (dots) { // Мигающие точки легко ассоциируются с часами
    drawPixelXY(X + 1, Y + 5, clockLED[2]);
    drawPixelXY(X + 5, Y + 5, clockLED[2]);
  } else {
    if (modeCode == MC_CLOCK) {
      drawPixelXY(X + 1, Y + 5, 0);
      drawPixelXY(X + 5, Y + 5, 0);
    }
  }
  drawDigit3x5(mins / 10, X, Y, clockLED[3]);
  drawDigit3x5(mins % 10, X + 4, Y, clockLED[4]);
#endif
}

void clockRoutine() {
  if (loadingFlag) {
#if (USE_RTC == 1 && (MCU_TYPE == 0 || MCU_TYPE == 1))
    DateTime now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
#endif
    loadingFlag = false;
    modeCode = MC_CLOCK;
  }

  FastLED.clear();
  if (!clockSet) {
    clockTicker();
    drawClock(hrs, mins, dotFlag, CLOCK_X, CLOCK_Y);
  } else {
    if (halfsecTimer.isReady()) {
      dotFlag = !dotFlag;
      if (dotFlag) clockColor();
      else for (byte i = 0; i < 5; i++) clockLED[i].fadeToBlackBy(190);
    }
    drawClock(hrs, mins, 1, CLOCK_X, CLOCK_Y);
  }
}

void clockTicker() {
  if (halfsecTimer.isReady()) {
    clockHue += HUE_STEP;
    setOverlayColors();

    dotFlag = !dotFlag;
// С библиотекой OldTime "закат солнца вручную" не нужен )
#if (MCU_TYPE != 1)
    if (dotFlag) {          // каждую секунду пересчёт времени
      secs++;
      if (secs > 59) {      // каждую минуту
        secs = 0;
        mins++;

#if (USE_RTC == 1 && (MCU_TYPE == 0 || MCU_TYPE == 1))
        DateTime now = rtc.now();
        time_t t = now; // Устанавливаем время в библиотеке OldTimer
        setTime(t);
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
#endif
      }
      if (mins > 59) {      // каждый час
        mins = 0;
        hrs++;
        if (hrs > 23) hrs = 0;  // сутки!
      }
    }
#endif
  }
}

#else
void clockRoutine() {
  return;
}
#endif

#if (CLOCK_ORIENT == 0 && USE_CLOCK == 1 && OVERLAY_CLOCK == 1)
void clockOverlayWrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 5; j++) {
      overlayLEDs[thisLED] = leds[getPixelNumber(i, j)];
      thisLED++;
    }
  }
  clockTicker();
  drawClock(hrs, mins, dotFlag, posX, posY);
}

void clockOverlayUnwrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 15; i++) {
    for (byte j = posY; j < posY + 5; j++) {
      leds[getPixelNumber(i, j)] = overlayLEDs[thisLED];
      thisLED++;
    }
  }
}

#elif (CLOCK_ORIENT == 1 && USE_CLOCK == 1 && OVERLAY_CLOCK == 1)
void clockOverlayWrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 7; i++) {
    for (byte j = posY; j < posY + 10; j++) {
      overlayLEDs[thisLED] = leds[getPixelNumber(i, j)];
      thisLED++;
    }
  }
  clockTicker();
  drawClock(hrs, mins, dotFlag, posX, posY);
}

void clockOverlayUnwrap(byte posX, byte posY) {
  byte thisLED = 0;
  for (byte i = posX; i < posX + 7; i++) {
    for (byte j = posY; j < posY + 10; j++) {
      leds[getPixelNumber(i, j)] = overlayLEDs[thisLED];
      thisLED++;
    }
  }
}
#endif

#if (OVERLAY_CLOCK == 1 && USE_CLOCK == 1)
boolean needUnwrap() {
  if (modeCode == MC_SNOW ||
      modeCode == MC_SPARKLES ||
      modeCode == MC_MATRIX ||
      modeCode == MC_STARFALL ||
      modeCode == MC_BALLS ||
      modeCode == MC_FIRE) return true;
  else return false;
}

void contrastClock() {
  for (byte i = 0; i < 5; i++) clockLED[i] = contrastColor;
}

void setOverlayColors() {
  switch (modeCode) {
    case MC_CLOCK: 
    case MC_GAME: 
    case MC_SPARKLES:
    case MC_MATRIX:
    case MC_STARFALL:
    case MC_BALL:
    case MC_BALLS: 
    case MC_FIRE: 
    case MC_NOISE_RAINBOW:
    case MC_NOISE_RAINBOW_STRIP: 
    case MC_RAINBOW:
    case MC_RAINBOW_DIAG: 
      clockColor();
      break;
    case MC_SNOW:
    case MC_NOISE_ZEBRA: 
    case MC_NOISE_MADNESS:
    case MC_NOISE_CLOUD:
    case MC_NOISE_LAVA:
    case MC_NOISE_PLASMA:
    case MC_NOISE_FOREST:
    case MC_NOISE_OCEAN: 
      contrastClock();
      break;
  }
}

#else
void clockOverlayWrap(byte posX, byte posY) {
  return;
}
void clockOverlayUnwrap(byte posX, byte posY) {
  return;
}
boolean needUnwrap() {
  return true;
}
#endif
