// работа с бегущим текстом

// чем больше матрица и MAX_TEXT, тем выше шанс того, что всё зависнет!
#define MAX_TEXT 20       // максимальная длина бегущей строки (русский символ занимает 2 байта!!!)
#define TEXT_DIRECTION 1  // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V 0          // отразить текст по вертикали (0 / 1)
#define MIRR_H 0          // отразить текст по горизонтали (0 / 1)

#define TEXT_HEIGHT 0     // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH 5       // ширина буквы шрифта
#define LET_HEIGHT 8      // высота буквы шрифта
#define SPACE 1           // пробел

byte string_length;
int offset, stop_pos;
byte string_byte[MAX_TEXT];

// вот она, вот она жесть! Английские символы (латиница) кодируются 
// одним байтом, а русские - двумя! Поэтому для отображения русских букв
// используем вот такой хитрющий алгоритм. Привет Вольтнику! Я помогал
// ему с этим алгоритмом для проекта GSM Pager =)
void initString() {
  int i = 0, j = 0;
  while (runningText[i] != '\0') {
    if ((byte)runningText[i] > 191) {
      i++;
      continue;
    } else {
      string_byte[j] = (byte)runningText[i];
      j++;
      i++;
    }
    if (j >= MAX_TEXT - 1) break;   // защита от переполнения массива
  }
  string_length = j;
  offset = WIDTH;
  stop_pos = -string_length * (LET_WIDTH + SPACE);
}

void fillString() {
  if (scrollTimer.isReady()) {
    FastLED.clear();
    for (int i = 0; i < string_length; i++) {
      drawLetter(string_byte[i], offset + i * (LET_WIDTH + SPACE));
    }
    offset--;
    if (offset < stop_pos) {
      offset = WIDTH;
    }
    FastLED.show();
  }
}

void drawLetter(uint8_t letter, int16_t offset) {
  int8_t start_pos = 0, finish_pos = LET_WIDTH;

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
      if (TEXT_DIRECTION) {
        if (thisBit) drawPixelXY(offset + i, TEXT_HEIGHT + j, globalColor);
        else drawPixelXY(offset + i, TEXT_HEIGHT + j, 0x000000);
      } else {
        if (thisBit) drawPixelXY(i, offset + TEXT_HEIGHT + j, globalColor);
        else drawPixelXY(i, offset + TEXT_HEIGHT + j, 0x000000);
      }

    }
  }
}

// ------------- СЛУЖЕБНЫЕ ФУНКЦИИ --------------

// интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
uint8_t getFont(uint8_t font, uint8_t row) {
  font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
  if (font <= 90) return pgm_read_byte(&(fontHEX[font][row]));     // для английских букв и символов
  else if (font >= 112 && font <= 159) {    // и пизд*ц ждя русских
    return pgm_read_byte(&(fontHEX[font - 17][row]));
  } else if (font >= 96 && font <= 111) {
    return pgm_read_byte(&(fontHEX[font + 47][row]));
  }
}

/*
// интерпретатор кода символа по ASCII в его номер в массиве fontHEX (для Arduino IDE до 1.6.*)
uint8_t getFontOld(uint8_t font, uint8_t row) {
  font = font - '0' + 16;   // перевод код символа из таблицы ASCII в номер согласно нумерации массива
  if (font < 126) return pgm_read_byte(&(fontHEX[font][row]));   // для английских букв и символов
  else return pgm_read_byte(&(fontHEX[font - 65][row]));         // для русских букв и символов (смещение -65 по массиву)
}
*/
