#pragma once

#include "Config.h"
#include "LCD.h"
#include "Font5x7.h"

class Graphics
{
  public:
    static const word NoFill = 1;
    void DrawPanel(int x, int y);
    void DrawRect(int x, int y, int w, int h, int t, word fg, word bg = NoFill);
    void FillRect(int x, int y, int w, int h, word c);

    char* LookupLabel(int idx, const char* labels);
    void DrawLabel(int centreX, int centreY, const char* str, word foreground, word background, int scale);
    void DrawDetail(int x, int y, unsigned long long int pattern, word fg, word bg, bool rev = false);
};

extern Graphics graphics;
