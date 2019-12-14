#pragma once

#define FONT5x7_HEIGHT 7
#define FONT5x7_WIDTH  5

class Font5x7
{
  public:
    static byte getRow(char Char, int Line);
    static void drawText(int x, int y, const char* str, word foreground, word background, int scale=1, bool stretch=false, bool descenders = false, int gap = 1);
};
