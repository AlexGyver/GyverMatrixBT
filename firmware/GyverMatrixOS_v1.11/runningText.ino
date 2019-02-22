// работа с бегущим текстом

// **************** НАСТРОЙКИ ****************
#define MIRR_V 0          // отразить текст по вертикали (0 / 1)
#define MIRR_H 0          // отразить текст по горизонтали (0 / 1)

#define TEXT_HEIGHT 0     // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH 5       // ширина буквы шрифта
#define LET_HEIGHT 8      // высота буквы шрифта
#define SPACE 1           // пробел

// --------------------- ДЛЯ РАЗРАБОТЧИКОВ ----------------------

#if (USE_FONTS == 1)

int offset = WIDTH;

void fillString(String text, uint32_t color) {
  if (loadingFlag) {
    offset = WIDTH;   // перемотка в правый край
    loadingFlag = false;    
#if (SMOOTH_CHANGE == 1)
    loadingFlag = modeCode == MC_TEXT && fadeMode < 2 ;
#else
    loadingFlag = false;        
#endif
    modeCode = MC_TEXT;
    fullTextFlag = false;
  }
  
  if (scrollTimer.isReady()) {
    FastLED.clear();
    byte i = 0, j = 0;
    while (text[i] != '\0') {
      if ((byte)text[i] > 191) {    // работаем с русскими буквами!
        i++;
      } else {
        drawLetter(j, text[i], offset + j * (LET_WIDTH + SPACE), color);
        i++;
        j++;
      }
    }
    fullTextFlag = false;

    offset--;
    if (offset < -j * (LET_WIDTH + SPACE)) {    // строка убежала
      offset = WIDTH + 3;
      fullTextFlag = true;
    }
    
    // Сразу показывать сформированный текст не нужно - на него могут далее налагаться эффекты мерцания / цвета
    // там где это нужно - будет принудительный вызов FastLED.show()
    // FastLED.show(); 
  }
}

void drawLetter(uint8_t index, uint8_t letter, int16_t offset, uint32_t color) {
  int8_t start_pos = 0, finish_pos = LET_WIDTH;
  int8_t offset_y = (HEIGHT - LET_HEIGHT) / 2;     // по центру матрицы по высоте
  
  CRGB letterColor;
  if (color == 1) letterColor = CHSV(byte(offset * 10), 255, 255);
  else if (color == 2) letterColor = CHSV(byte(index * 30), 255, 255);
  else letterColor = color;

  if (offset < -LET_WIDTH || offset > WIDTH) return;
  if (offset < 0) start_pos = -offset;
  if (offset > (WIDTH - LET_WIDTH)) finish_pos = WIDTH - offset;

  for (byte i = start_pos; i < finish_pos; i++) {
    int thisByte;
    if (MIRR_V) thisByte = getFont((byte)letter, LET_WIDTH - 1 - i);
    else thisByte = getFont((byte)letter, i);

    for (byte j = 0; j < LET_HEIGHT; j++) {
      boolean thisBit;

      if (MIRR_H) thisBit = thisByte & (1 << j);
      else thisBit = thisByte & (1 << (LET_HEIGHT - 1 - j));

      // рисуем столбец (i - горизонтальная позиция, j - вертикальная)
      if (thisBit) leds[getPixelNumber(offset + i, offset_y + TEXT_HEIGHT + j)] = letterColor;
    }
  }
}

// ------------- СЛУЖЕБНЫЕ ФУНКЦИИ --------------

// интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
uint8_t getFont(uint8_t font, uint8_t row) {
  font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
  if (font <= 90) return pgm_read_byte(&(fontHEX[font][row]));     // для английских букв и символов
  else if (font >= 112 && font <= 159) {    // и пизд*ц для русских
    return pgm_read_byte(&(fontHEX[font - 17][row]));
  } else if (font >= 96 && font <= 111) {
    return pgm_read_byte(&(fontHEX[font + 47][row]));
  }
}

#elif (USE_FONTS == 0)
void fillString(String text, uint32_t color) {
  fullTextFlag = false;
  modeCode = MC_TEXT;
  return;
}
#endif

/*
  // интерпретатор кода символа по ASCII в его номер в массиве fontHEX (для Arduino IDE до 1.6.*)
  uint8_t getFontOld(uint8_t font, uint8_t row) {
  font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
  if (font < 126) return pgm_read_byte(&(fontHEX[font][row]));   // для английских букв и символов
  else return pgm_read_byte(&(fontHEX[font - 65][row]));         // для русских букв и символов (смещение -65 по массиву)
  }
*/
