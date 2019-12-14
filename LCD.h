#pragma once

// If defined, LCD oriented with USB down, otherwise up
//#define ROTATION_USB_DOWN

// optionally dump graphics cmds to serial:
//#define SERIALIZE

#define RGB(_r, _g, _b) (word)((_b & 0x00F8) >> 3) | ((_g & 0x00FC) << 3) | ((_r & 0x00F8) << 8)

// optimised code, specific to Uno
#define ToggleDataWR PORTC = B00010101; PORTC = B00010111; // keeps RST, RS & RD HIGH 
#define FastData(d) FastData2((d) & B11111100, (d) & B00000011);
// Preserves 10,11,12 & 13 on B
#define FastData2(h, l) PORTD = (h); PORTB = (PORTB & B11111100) | (l); ToggleDataWR;

#ifdef SERIALIZE
#define SERIALISE_ON(_on) lcd._serialise=_on;
#define SERIALISE_COMMENT(_c) if (lcd._serialise) { Serial.print("; ");Serial.println(_c);}
#define SERIALISE_INIT(_w,_h,_s) if (lcd._serialise) { Serial.print(_w);Serial.print(',');Serial.print(_h);Serial.print(',');Serial.println(_s);}
#define SERIALISE_BEGINFILL(_x,_y,_w,_h) if (lcd._serialise) { Serial.print(_x);Serial.print(',');Serial.print(_y);Serial.print(',');Serial.print(_w);Serial.print(',');Serial.println(_h);}
#define SERIALISE_FILLCOLOUR(_len,_colour) if (lcd._serialise) { Serial.print(_len);Serial.print(',');Serial.println(_colour);}
#define SERIALISE_FILLBYTE(_len,_colour) if (lcd._serialise) { Serial.print(_len);Serial.print(',');Serial.println(_colour?0xFFFF:0x0000);}
#else
#define SERIALISE_ON(_on)
#define SERIALISE_COMMENT(_c)
#define SERIALISE_INIT(_w,_h,_s)
#define SERIALISE_BEGINFILL(_x,_y,_w,_h)
#define SERIALISE_FILLCOLOUR(_len,_colour)
#define SERIALISE_FILLBYTE(_len,_colour)
#endif

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

class LCD
{
  public:
     void init();
     void ChipSelect(bool select);

     unsigned long beginFill(int x, int y, int w, int h);
     // faster when not relying on pixel wrapping in window (eg strips)
     // set the window:
     void setWindow(int x, int y, int w, int h);
     // set the position
     void setPosition(int x, int y, int w, int h);
     
     void fillColour(unsigned long size, word colour);
     void fillByte(unsigned long size, byte colour);

     bool isTouch(int x, int y, int w,int h);
     bool getTouch(int& x, int& y);
     
     int  _width;
     int  _height;
     int  _rotation;

#ifdef SERIALIZE
     bool _serialise = false;
#endif
};

extern LCD lcd;
