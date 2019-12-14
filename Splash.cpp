#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Config.h"
#include "Graphics.h"
#include "Moon.h"
#include "Splash.h"

//#define QUICK_SPLASH

const char* pName = "ArDSKYlite";
const char* pCredit = "Mark Wilson MMXIX";
const char* pHelp = "Press PROG for Info";

void DoSplash()
{
  bool North = Config::_OutputFlags & Config::MaskNorthern;
  lcd.fillByte(lcd.beginFill(0, 0, LCD_WIDTH, LCD_HEIGHT), 0);
  int scale = 2;
  int line = 4;
  Font5x7::drawText((LCD_WIDTH - strlen(pName)*(FONT5x7_WIDTH + 1)*scale)/2, line, pName, 0x001F, 0x0000, scale);
  line += scale*(FONT5x7_HEIGHT + 2);
  scale = 1;
  Font5x7::drawText((LCD_WIDTH - strlen(pCredit)*(FONT5x7_WIDTH + 1)*scale)/2, line, pCredit, 0xF800, 0x0000, scale);
  Font5x7::drawText((LCD_WIDTH - strlen(pHelp)*(FONT5x7_WIDTH + 1)*scale)/2, LCD_HEIGHT - FONT5x7_HEIGHT - 4, pHelp, 0x07E0, 0x0000, scale);
  
  word palette[16];
  Moon::CreatePalette(palette);

  Moon::DrawGore(0, 180, palette, true);  // start Full
#ifndef QUICK_SPLASH
  int step = 1; // 1 degree
  for (int pass = 0; pass < 2; pass++)
    if (North)
      for (int angle = 180 - step; angle >= 0 ; angle -= step)
        Moon::DrawGore(angle, angle + step, palette, pass % 2);
    else    
      for (int angle = 0; angle <= 180 - step; angle += step)
        Moon::DrawGore(angle, angle + step, palette, pass % 2);
  // ends Full, draw landing site
  Moon::DrawLandingSite();
  delay(2000);
#endif  
}
