#include <Arduino.h>
#include "Config.h"
#include "Graphics.h"
#include "Programs.h"
#include "LCD.h"
#include "Font5x7.h"
#include "Help.h"

const char pHelpText[] PROGMEM = 
// Yeah, nice. Arduino supports this syntax. Can't embed escaped chars like \t tho'
//                         max width->| (38 cols)
//-------------------------------------
R"(^ArDSKYlite
A very ^lightweight representation of 
the Apollo ^DSKY.

VERB list:          NOUN list:
05 Display Oct      24 Uptime (H,M,S)  
06 Display Dec      28 Moon Phase
15 Monitor Oct      29 Auto-run (V,N)
16 Monitor Dec      30 Output Flagword
18 Monitor Graphic  31 Input Flagword
25 Enter Data       36 Clk Time (H,M,S)
35 Lamp Test        39 Clk Date (Y,M,D)
50 Please Perform   9x Stack Op
99 Engine Ign

^Decimals start with +/-, else ^Octal.

V16N36:Display Time, V18N28:Moon Phase
V25N28:(D,H,M)=New Moon in D days @ H:M
V25N30:0wxyz: w=N/S Hemi, x=24/12h,
  y=Leading 0s, z=YMD/DMY/MDY (^Output)
V25N31:0000z: z=Flash Btn
V50N9x:RPN Calc, V35:Lights, V99:Action
N9x=0:Clr,1:Push,2:Pop,3:Dup,4:Add,
    5:Sub,6:Mult,7:Div,8:Neg,9:Swap)";
//-------------------------------------

void Help::Draw()
{
  lcd.fillByte(lcd.beginFill(0, 0, LCD_WIDTH, LCD_HEIGHT), 0);

  int margin = 2;
  int lineHeight = (FONT5x7_HEIGHT + 3);
  int cols = (lcd._width - 2*margin)/(FONT5x7_WIDTH + 1);
  int lines = (LCD_HEIGHT)/lineHeight;
  int col = 0;
  int line = 0;
  char wordStr[64];
  bool lineBreak = false;
  bool textEffect = false;
  word fore = 0xFFFF;
  word effect = 0x07E0;
  const char* pText = pHelpText;
  while (true)
  {
    char ch = pgm_read_byte_near(pText++);
    char* pWord = wordStr;
    
    // load buffer...
    
    if (ch == '^')
    {
      textEffect = true;
      ch = pgm_read_byte_near(pText++);
    }
    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
    {
      // word
      while (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9'))
      {
        *(pWord++) = ch;
        ch = pgm_read_byte_near(pText++);
      }
      if ((wordStr[0] == 'V' || wordStr[0] == 'N') && wordStr[1] <= 'Z' && wordStr[1] >= '0')
        textEffect = true;
      pText--;
    }
    else if (ch == '\n')
      lineBreak = true;
    else if (ch == 0)
      break;
    else // just a char
      *(pWord++) = ch;
       
    *pWord = 0;
    int len = strlen(wordStr);
    if (col + len > cols)
    {
      // word wrap
      lineBreak = true;
    }
    
    if (lineBreak)
    {
      col = 0;
      line++;
      if (line > lines)
        return;
    }
    Font5x7::drawText(margin + col*(FONT5x7_WIDTH + 1), margin + line*lineHeight, wordStr, textEffect?effect:fore, 0x0000, 1, false, true);
    col += len;
    lineBreak = textEffect = false;
  }
  line += 2;
  col = 0;
  strcpy(wordStr, "");
  if (Programs::GetNotAvailable(wordStr))
  {
    Font5x7::drawText(margin + col*(FONT5x7_WIDTH + 1), margin + line*lineHeight, "Not available:", 0xF81F, 0x0000, 1, false, true);
    line++;
    col++;
    Font5x7::drawText(margin + col*(FONT5x7_WIDTH + 1), margin + line*lineHeight, wordStr, 0xF800, 0x0000, 1, false, true);
  }
}
