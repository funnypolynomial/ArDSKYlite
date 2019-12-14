#include <Arduino.h>
// "XC4630d" LCD code from this demo https://www.jaycar.co.nz/rfid-keypad NOT the code on the part's download page https://www.jaycar.co.nz/240x320-lcd-touch-screen-for-arduino/p/XC4630
#include "XC4630d.h"
#include "LCD.h"

LCD lcd;

#define LCD_RD   B00000001
#define LCD_WR   B00000010     
#define LCD_RS   B00000100  
#define LCD_CS   B00001000       
#define LCD_RST  B00010000

// Preserves 10,11,12 & 13 on B
#define FastCmdByte(c) PORTC = LCD_RST | LCD_RD; PORTD = c & B11111100; PORTB = (PORTB & B11111100) | (c & B00000011);PORTC |= LCD_WR; PORTC |= LCD_RS;
#define FastCmd(c) FastCmdByte(0); FastCmdByte(c);

void LCD::init()
{
  // reset
  delay(250);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW);
  delay(100);
  digitalWrite(A4, HIGH);
  delay(500);
  

  XC4630_init();
  ChipSelect(true);  

#ifdef ROTATION_USB_DOWN
  XC4630_rotate(3);  
#else
  XC4630_rotate(1);  
#endif  
  _rotation = XC4630_orientation;
  _width = XC4630_width;
  _height = XC4630_height;
  SERIALISE_COMMENT("*** START");
  SERIALISE_INIT(_width,_height,1);
}

void LCD::ChipSelect(bool select)
{
  if (select)
    PORTC &= ~LCD_CS;
  else
    PORTC |= LCD_CS;
}


unsigned long LCD::beginFill(int x, int y, int w, int h)
{
  SERIALISE_BEGINFILL(x, y, w, h);
#ifdef ROTATION_USB_DOWN
  int x2 = x + w - 1;
  int y2 = y + h - 1;
  FastCmd(0x50);               //set x bounds  
  FastData(x >> 8); 
  FastData(x);
  FastCmd(0x51);               //set x bounds  
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x52);               //set y bounds
  FastData(y >> 8);
  FastData(y);
  FastCmd(0x53);               //set y bounds  
  FastData(y2 >> 8);
  FastData(y2);

  FastCmd(0x20);               //set x pos
  FastData(x >> 8);
  FastData(x);
  FastCmd(0x21);               //set y pos
  FastData(y >> 8);
  FastData(y);
  
  FastCmd(0x22);  // Write Data to GRAM
#else
  int x2 = x + w - 1;
  int y2 = y + h - 1;
  
  int tmp = x;
  x = _width - x2 - 1;
  x2 = _width - tmp - 1;
  tmp = y;
  y = _height - y2 - 1;
  y2 = _height - tmp - 1;

  FastCmd(0x50);               //set x bounds  
  FastData(x >> 8); 
  FastData(x);
  FastCmd(0x51);               //set x bounds  
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x52);               //set y bounds
  FastData(y >> 8);
  FastData(y);
  FastCmd(0x53);               //set y bounds  
  FastData(y2 >> 8);
  FastData(y2);

  FastCmd(0x20);               //set x pos
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x21);               //set y pos
  FastData(y2 >> 8);
  FastData(y2);
  
  FastCmd(0x22);  // Write Data to GRAM
#endif
  unsigned long count = w;
  count *= h;
  return count;  
}

void LCD::setWindow(int x, int y, int w, int h)
{
  // just defines the window
#ifdef ROTATION_USB_DOWN
  int x2 = x + w - 1;
  int y2 = y + h - 1;
  FastCmd(0x50);               //set x bounds  
  FastData(x >> 8); 
  FastData(x);
  FastCmd(0x51);               //set x bounds  
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x52);               //set y bounds
  FastData(y >> 8);
  FastData(y);
  FastCmd(0x53);               //set y bounds  
  FastData(y2 >> 8);
  FastData(y2);
#else
  int x2 = x + w - 1;
  int y2 = y + h - 1;
  int tmp = x;
  x = _width - x2 - 1;
  x2 = _width - tmp - 1;
  tmp = y;
  y = _height - y2 - 1;
  y2 = _height - tmp - 1;

  FastCmd(0x50);               //set x bounds  
  FastData(x >> 8); 
  FastData(x);
  FastCmd(0x51);               //set x bounds  
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x52);               //set y bounds
  FastData(y >> 8);
  FastData(y);
  FastCmd(0x53);               //set y bounds  
  FastData(y2 >> 8);
  FastData(y2);
#endif
}

void LCD::setPosition(int x, int y, int w, int h)
{
    // just defines the cursor (pixel write) position
  SERIALISE_BEGINFILL(x, y, w, h);  // is this right?
#ifdef ROTATION_USB_DOWN
  FastCmd(0x20);               //set x pos
  FastData(x >> 8);
  FastData(x);
  FastCmd(0x21);               //set y pos
  FastData(y >> 8);
  FastData(y);
  
  FastCmd(0x22);  // Write Data to GRAM
#else
  int x2 = x + w - 1;
  int y2 = y + h - 1;
  int tmp = x;
  x = _width - x2 - 1;
  x2 = _width - tmp - 1;
  tmp = y;
  y = _height - y2 - 1;
  y2 = _height - tmp - 1;
  
  FastCmd(0x20);               //set x pos
  FastData(x2 >> 8);
  FastData(x2);
  FastCmd(0x21);               //set y pos
  FastData(y2 >> 8);
  FastData(y2);
  
  FastCmd(0x22);  // Write Data to GRAM
#endif    
}


void LCD::fillColour(unsigned long count, word colour)
{
  SERIALISE_FILLCOLOUR(count, colour);
  
  // fill with full 16-bit colour
  byte h1 = (colour >> 8) & B11111100;
  byte l1 = (colour >> 8) & B00000011;
  byte h2 =  colour       & B11111100;
  byte l2 =  colour       & B00000011;
  while (count--)
  {
    FastData2(h1, l1);
    FastData2(h2, l2);
  }
}

void LCD::fillByte(unsigned long count, byte colour)
{
  SERIALISE_FILLBYTE(count, colour);
  
  // fill with just one byte, i.e. 0/black or 255/white, or other, for pastels
  PORTD = colour & B11111100; 
  PORTB = (PORTB & B11111100) | (colour & B00000011);
  while (count--)
  {
    ToggleDataWR;
    ToggleDataWR;
  }
}

bool LCD::isTouch(int x, int y, int w,int h)
{
  return XC4630_istouch(x,y,x+w,y+h);
}

bool LCD::getTouch(int& x, int& y)
{
  x = XC4630_touchx();
  y = XC4630_touchy();
  return x >= 0 && x >= 0;
}
