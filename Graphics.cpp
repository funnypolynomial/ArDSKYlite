#include <Arduino.h>
#include "Graphics.h"

extern Graphics grpahics;

void Graphics::DrawPanel(int x, int y)
{
  // draw the panel surrounding lights and numbers
  // outer
  DrawRect(x, y, Config::PanelRectWidth, Config::PanelRectHeight, Config::PanelRectThickness,  Config::ForegroundColour);
  // inner
  DrawRect(x + Config::PanelInnerOffset, y + Config::PanelInnerOffset, Config::PanelRectWidth - 2*Config::PanelInnerOffset, Config::PanelRectHeight - 2*Config::PanelInnerOffset, Config::PanelRectThickness, Config::ForegroundColour);
  // corner rounding details
  DrawDetail(x, y, 0xFFE0C08F90909090ULL, Config::ForegroundColour, Config::BackgroundColour);
  DrawDetail(x + Config::PanelRectWidth-8, y + Config::PanelRectHeight-8, 0xFFE0C08F90909090ULL, Config::ForegroundColour, Config::BackgroundColour, true);
  DrawDetail(x + Config::PanelRectWidth-8, y, 0xFF0703F109090909ULL, Config::ForegroundColour, Config::BackgroundColour);
  DrawDetail(x, y + Config::PanelRectHeight-8, 0xFF0703F109090909ULL, Config::ForegroundColour, Config::BackgroundColour, true);
}

void Graphics::DrawRect(int x, int y, int w, int h, int t, word fg, word bg)
{
  // simple rect at x,y, dims w,h, thickness f, colours fg,bg
  if (bg != NoFill)
    lcd.fillColour(lcd.beginFill(x+t, y+t, w-2*t, h-2*t), bg);
  if (t)
  {
    lcd.fillColour(lcd.beginFill(x, y, w, t), fg);
    lcd.fillColour(lcd.beginFill(x, y, t, h), fg);
    lcd.fillColour(lcd.beginFill(x, y + h - t, w, t), fg);
    lcd.fillColour(lcd.beginFill(x + w - t, y, t, h), fg);
  }
}

void Graphics::FillRect(int x, int y, int w, int h, word c)
{
    lcd.fillColour(lcd.beginFill(x, y, w, h), c);
}

  char labelBuffer[Config::LabelMaxLen];

char* Graphics::LookupLabel(int idx, const char* labels)
{
  // copy the text for the idx'th item in labels into label
  // label lists are simple PROGMEM strings, null delimited
  const char* pLabel = labels;
  while (idx)
    if (pgm_read_byte_near(pLabel++) == 0)
      idx--;
  strcpy_P(labelBuffer, pLabel);
  return labelBuffer;
}

void Graphics::DrawLabel(int centreX, int centreY, const char* str, word foreground, word background, int scale)
{
  char label[Config::LabelMaxLen];
  strcpy(label, str);
  
  char* pNewLine = strstr(label, "\n");
  char* pLabel = label;
  int dY = 0;
  centreY -= (FONT5x7_WIDTH+1)*scale/2;
  if (pNewLine)
  {
    dY = (FONT5x7_HEIGHT - 1)*scale;
    *pNewLine = '\0';
    Font5x7::drawText(centreX - (strlen(label)*(FONT5x7_WIDTH+1)*scale - scale)/2, centreY - dY, label, foreground, background, scale);
    pLabel = pNewLine + 1;
  }
  Font5x7::drawText(centreX - (strlen(pLabel)*(FONT5x7_WIDTH+1)*scale - scale)/2, centreY + dY, pLabel, foreground, background, scale);
}

void Graphics::DrawDetail(int x, int y, unsigned long long int pattern, word fg, word bg, bool rev)
{
  // draws 8x8 pattern from bits in pattern,
  // highest bit is top-left pixel 
  lcd.beginFill(x, y, 8, 8);
  unsigned long long int mask = 0x01L;
  if (!rev)
    mask <<= 63;
  while (mask)
  {
    lcd.fillColour(1, (mask & pattern)?fg:bg);
    if (rev)
      mask <<= 1;
    else
      mask >>= 1;
  }
}
