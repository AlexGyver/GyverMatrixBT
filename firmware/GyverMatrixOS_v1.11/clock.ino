// режим часов

// ****************** НАСТРОЙКИ ЧАСОВ *****************
#define OVERLAY_CLOCK 1     // часы на фоне всех эффектов и игр. Жрёт SRAM память!
#define CLOCK_ORIENT 0      // 0 горизонтальные, 1 вертикальные
#define CLOCK_X 0           // позиция часов по X (начало координат - левый нижний угол)
#define CLOCK_Y 6           // позиция часов по Y (начало координат - левый нижний угол)
#define COLOR_MODE 2        // Режим цвета часов
//                          0 - заданные ниже цвета
//                          1 - радужная смена (каждая цифра)
//                          2 - радужная смена (часы, точки, минуты)

#define MIN_COLOR CRGB::White     // цвет минут
#define HOUR_COLOR CRGB::White    // цвет часов
#define DOT_COLOR CRGB::Red       // цвет точек

#define HUE_STEP 5          // шаг цвета часов в режиме радужной смены
#define HUE_GAP 30          // шаг цвета между цифрами в режиме радужной смены

// эффекты, в которых отображаются часы в наложении
byte overlayList[] = {
  MADNESS_NOISE,
  OCEAN_NOISE,
};

/*
   Список режимов:
  GAME_MODE
  MADNESS_NOISE
  CLOUD_NOISE
  LAVA_NOISE
  PLASMA_NOISE
  RAINBOW_NOISE
  RAINBOWSTRIPE_NOISE
  ZEBRA_NOISE
  FOREST_NOISE
  OCEAN_NOISE
  SNOW_ROUTINE
  SPARKLES_ROUTINE
  MATRIX_ROUTINE
  STARFALL_ROUTINE
  BALL_ROUTINE
  BALLS_ROUTINE
  RAINBOW_ROUTINE
  RAINBOWDIAGONAL_ROUTINE
  FIRE_ROUTINE
  IMAGE_MODE
*/

// ****************** ДЛЯ РАЗРАБОТЧИКОВ ****************
byte listSize = sizeof(overlayList);
byte clockHue;
#if (OVERLAY_CLOCK == 1 && CLOCK_ORIENT == 0)
CRGB overlayLEDs[75];
#elif (OVERLAY_CLOCK == 1 && CLOCK_ORIENT == 1)
CRGB overlayLEDs[70];
#endif

#if (USE_CLOCK == 1)
CRGB clockLED[5] = {CRGB::White, CRGB::White, CRGB::Red, CRGB::White, CRGB::White};

#if (MCU_TYPE == 1)
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
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
  if (millis() - ntp_t > 3000) {
    ntp_t = 0;
    return;
  }
  int cb = udp.parsePacket();
  if (!cb) {
    return;
  } else {
    ntp_t = 0;
    init_time = 1;
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    unsigned long seventyYears = 2208988800UL ;
    //DateTime now = secsSince1900 - seventyYears + (timeZoneOffset + daylight) * 3600;
    
    time_t t = secsSince1900 - seventyYears + (timeZoneOffset) * 3600;
    setTime(t);
  }
}

void getNTP() {
  if (!connected) {
    return;
  }
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  ntp_t = millis();
}
#endif

boolean overlayAllowed() {
  for (byte i = 0; i < listSize; i++)
    if (modeCode == overlayList[i]) return true;
  return false;
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
#if (CLOCK_ORIENT == 0)
  if (hrs > 9) drawDigit3x5(hrs / 10, X, Y, clockLED[0]);
  drawDigit3x5(hrs % 10, X + 4, Y, clockLED[1]);
  if (dots) {
    drawPixelXY(X + 7, Y + 1, clockLED[2]);
    drawPixelXY(X + 7, Y + 3, clockLED[2]);
  } else {
    if (modeCode == 1) {
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
    if (modeCode == 1) {
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
    modeCode = 1;
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
    if (needColor()) clockColor();

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

boolean needColor() {
  if (modeCode == 1 ||
      modeCode == 9 ||
      modeCode == 12 ||
      modeCode == 13 ||
      modeCode == 14 ||
      modeCode == 15 ||
      modeCode == 16 ||
      modeCode == 17 ||
      modeCode == 20) return true;
  else return false;
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
  if (modeCode == 12 ||
      modeCode == 13 ||
      modeCode == 14 ||
      modeCode == 15 ||
      modeCode == 17 ||
      modeCode == 20) return true;
  else return false;
}

void blackClock() {
  for (byte i = 0; i < 5; i++) clockLED[i] = 0;
  clockLED[2] = CRGB::White;
}

void setOverlayColors() {
  switch (modeCode) {
    case 1: clockColor();
      break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: blackClock();
      break;
    case 9: clockColor();
      break;
    case 10:
    case 11: blackClock();
      break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: clockColor();
      break;
    case 18:
    case 19: blackClock();
      break;
    case 20: clockColor();
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
