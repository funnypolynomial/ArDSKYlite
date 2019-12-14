#include <Arduino.h>
#include "Graphics.h"
#include "Keyboard.h"
#include "Lights.h"
#include "Programs.h"
#include "DSKY.h"
#include "Numerics.h"

Numerics numerics;

static const char* verbNounEmpty = "  ";
static const char* registerEmpty = "      ";

#define FONT_WIDTH (15)
#define FONT_HEIGHT (25)
// 15x25
//   A
// F   B
//   G
// E   C
//   D   H=DP
// plus some extras for the +/-
byte fontSegments[] =
{
  // Each segment is two rectangle fills.
  // A fill is three bytes: [flags in upper nibble (F), top-left col in lower nibble (C)], [top-left row (RR)], [width in upper nibble (W), height in lower nibble (H)]
  // Coords are relative to top-left
  // Flags is two bits 0b00xy to clear a pixel in a rectangle corner (except top-left)
  
  // FC    RR    WH
  // A
   0x05, 0x00, 0x71,  
   0x06, 0x01, 0x61,  
  // B
   0x0D, 0x00, 0x26,  
   0x0C, 0x06, 0x26,  
  // C
   0x0B, 0x0D, 0x26,  
   0x1A, 0x13, 0x26,
  // D
   0x03, 0x17, 0x61,  
   0x02, 0x18, 0x81,  
  // E
   0x01, 0x0D, 0x26,  
   0x30, 0x13, 0x26,
  // F
   0x23, 0x00, 0x26,  
   0x02, 0x06, 0x26,  
  // G
   0x05, 0x0B, 0x52,  
   0x00, 0x00, 0x00,  
  // H=DP
   0x0D, 0x17, 0x22,  
   0x00, 0x00, 0x00,  

  // I= vertical in '+' (two parts)
   0x07, 0x06, 0x24,  
   0x07, 0x0E, 0x24,  
  // J= horizontal in '+'/'-'
   0x03, 0x0B, 0xA2,  
   0x00, 0x00, 0x00,  
};

byte fontCharacters[] =
{
  //.GFEDCBA
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
};

void Numerics::Init(bool reinit)
{
  _statusFlags = 0b00001110;
  _changedFlags = 0xFFFF;

  if (_originX && !reinit)
  {
    ClearDigits(PROG);
    ClearDigits(VERB);
    ClearDigits(NOUN);
    ClearDigits(R1);
    ClearDigits(R2);
    ClearDigits(R3);
  }
  _blink = false;
  _blinkFlags = 0;

  _originX = Config::DisplayWidth - Config::PanelRectWidth - Config::PanelRectOffsetX + Config::PanelInnerOffset + Config::PanelRectThickness + Config::NumericSideOffset;
  _originY = Config::PanelRectOffsetY + Config::PanelInnerOffset + Config::PanelRectThickness + Config::NumericTopOffset;
}

void Numerics::Draw()
{
  // draw the whole numeric display panel
  graphics.DrawPanel(Config::DisplayWidth - Config::PanelRectWidth - Config::PanelRectOffsetX, Config::PanelRectOffsetY);
  _changedFlags = 0xFFFF;
  Update();
}

void Numerics::Update()
{
  // COMP ACTY
  int x = _originX;
  int y = _originY;
  if (Changed(COMP_ACTY))
    DrawIndicator(x, y, COMP_ACTY, Config::NumericLabelCellWidth);
  
  // PROG
  x = _originX + Config::NumericSpaceWidth - Config::NumericLabelCellWidth;
  if (Changed(PROG))
  DrawIndicator(x, y, PROG);
  y += Config::NumericLabelCellHeight + 2;
  if (Changed(PROG))
    DrawStr(x, y, _program, Config::NumericColour, Config::BackgroundColour, PROG);

  // NOUN
  y += Config::NumericDigitCellHeight;
  if (Changed(NOUN))
    DrawIndicator(x, y, NOUN);
  if (Changed(NOUN) || Blinking(NOUN))
  {
    DrawStr(x, y + Config::NumericLabelCellHeight + 2, _noun, Config::NumericColour, Config::BackgroundColour, NOUN);
  }

  // VERB
  x = _originX;
  if (Changed(VERB))
    DrawIndicator(x, y, VERB);
  y += Config::NumericLabelCellHeight + 2;
  if (Changed(VERB) || Blinking(VERB))
    DrawStr(x, y, _verb, Config::NumericColour, Config::BackgroundColour, VERB);

  // registers
  y += Config::NumericDigitCellHeight;
  for (int lbl = R1; lbl <= R3; lbl++)
    DrawRegister(x, y, lbl, Config::NumericColour, Config::BackgroundColour);
      
  _changedFlags = 0;
}

void Numerics::Blink()
{
  if (_blinkFlags)
  {
    _blink = !_blink;
    Update();
  }
  else
  {
    _blink = false;
  }
}

void Numerics::DrawIndicator(int x, int y, int lbl, int h)
{
  word bkColour = (_statusFlags & 0x01 << lbl)?Config::NumericColour:Config::BackgroundColour;
  if (bkColour == Config::NumericColour && lbl == COMP_ACTY)
    bkColour = Config::IndicatorYellow;
  graphics.FillRect(x, y, Config::NumericLabelCellWidth, h, bkColour);
  graphics.DrawLabel(x + Config::NumericLabelCellWidth/2, y + h/2, graphics.LookupLabel(lbl, _labels), Config::ForegroundColour, bkColour, 1);
}

void Numerics::DrawRegister(int x, int& y, int lbl, word foreground, word background)
{
  // line
  if (Changed(lbl))
    graphics.FillRect(x + Config::NumericDigitCellWidth, y, Config::NumericSpaceWidth - Config::NumericDigitCellWidth, Config::NumericUnderlineThickness, foreground);
  // numbers
  y += Config::NumericUnderlineThickness;
  y += Config::NumericUnderlineOffset;
  if (Changed(lbl))
    DrawStr(x, y, _registers[lbl - R1], foreground, background, lastLabel);
  y += Config::NumericDigitCellHeight;
}

void Numerics::DrawStr(int x, int y, const char* str, word foreground, word background, int lbl)
{
  if (_blink && (_blinkFlags & (0x0001 << lbl)) != 0)
    foreground = background;
  
  while (*str)
  {
    DrawDigit(x, y, *str, foreground, background);
    x += 16;
    str++;
  }
}

void Numerics::DrawDigit(int x, int y, char ch, word foreground, word background)
{
  graphics.FillRect(x, y, 15, 25, background);
  word segments = 0;
  if (ch == '+')
    segments = 0b0000001100000000;
  else if (ch == '-')
    segments = 0b0000001000000000;
  else if ('0' <= ch && ch <= '9')
    segments = fontCharacters[ch - '0'];
  else
    return;
  
  for (int segIdx = 0; segIdx < 10; segIdx++)
    if (segments & (0x01 << segIdx))
    {
      byte* pSegDef = fontSegments + 6*segIdx;
      for (int region = 0; region < 2; region++)
      {
        byte flags = *pSegDef >> 4;
        byte col = *pSegDef & 0x0F;
        pSegDef++;
        byte row = *pSegDef;
        pSegDef++;
        if (*pSegDef == 0x00) break;
        byte wd = *pSegDef >> 4;
        byte ht = *pSegDef & 0x0F;
        pSegDef++;
        if (segments & (0x01 << segIdx))
          graphics.FillRect(x + col, y + row, wd, ht, foreground);
        else
          graphics.FillRect(x + col, y + row, wd, ht, Config::SegmentOffColour);
        if (flags & 0x03) // clear a pixel
        {
          wd = (flags & 0x02)?wd - 1:0;
          ht = (flags & 0x01)?ht - 1:0;
          ht *= flags  & 0x01;
          graphics.FillRect(x + col + wd, y + row + ht, 1, 1, background);
        }
      }
    }
}

void Numerics::ClearDigits(int lbl)
{
  // blank the verb/noun/reg
  Change(lbl);
  if (lbl < lastLabel)
    strcpy(GetValue(lbl), verbNounEmpty);
  else
    strcpy(GetValue(lbl), registerEmpty);
  Update();
}

bool Numerics::AddDigit(int lbl, int btn)
{
  // "type" a digit into verb/noun/register.  true if complete (2 digits/5 digits)
  if (lbl < lastLabel)
  {
    char digit = 0;
    char* vn = GetValue(lbl);
    if (keyboard.AsDigit(btn, digit))
    {
      if (vn[0] == ' ')
        vn[0] = digit;
      else if (vn[1] == ' ')
        vn[1] = digit;
      Change(lbl);
      Update();
    }
    return vn[1] == digit;
  }
  else if (lbl <= R3)
  {
    char digit = 0;
    char* vn = GetValue(lbl);
    if (btn == Keyboard::PLUS)
    {
      if ((vn[0] == ' ') && (vn[1] == ' '))
      {
        // first btn only
        vn[0] = '+';
        Change(lbl);
        Update();
      }        
    }
    else if (keyboard.AsDigit(btn, digit))
    {
      int idx = 1;
      while (vn[idx] != ' ')
        idx++;
      if (vn[idx] == ' ')
        vn[idx] = digit;
      Change(lbl);
      Update();
    }
    return vn[5] == digit;
  }
  return false;
}

long Numerics::GetDigits(int lbl)
{
  // returns verb/noun 0..99 or -2 if empty (EmptyValue), -1 if partial (PartialValue)
  // also returns decimal value of register
  if (lbl < lastLabel)
  {
    int digits = EmptyValue;
    char* vn = GetValue(lbl);
    if (vn[0] != ' ' && vn[1] == ' ')
      digits = PartialValue;
    if (vn[0] != ' ' && vn[1] != ' ')
    {
      digits = (vn[0] - '0')*10 + (vn[1] - '0');
    }
    return digits;
  }
  else if (lbl <= R3)
  {
    long result = 0;
    char* r = GetValue(lbl);
    if (*r == ' ')
    {
      // octal
      r++;
      do
      {
        result <<= 3;
        result |= (*r - '0');
        r++;
      } while (*r);
    }
    else
    {
      // decimal
      long sign = (*r == '+')?+1L:-1L;
      r++;
      do
      {
        result *= 10;
        result += (*r - '0');
        r++;
      } while (*r);
      result *= sign;
    }
    return result;
  }
  return -100000L;
}

char* Numerics::GetValue(int lbl)
{
  if (lbl < lastLabel)
    if (lbl == PROG)
      return _program;
    else
      return (lbl == VERB)?_verb:_noun;
  else
    return _registers[lbl - R1];
}

void Numerics::SetValue(int lbl, const char* value)
{
  Change(lbl);
  strcpy(GetValue(lbl), value);  
}

void Numerics::ToNNString(long value, char* pBuffer, bool leading0)
{
  int idx = 0;  
  if (leading0 || value >= 10)
  {
    pBuffer[idx++] = '0' + value/10;    
  }
  pBuffer[idx++] = '0' + value%10;
  pBuffer[idx] = 0;
}

void Numerics::SetValue(int lbl, long value, int base)
{
  if (PROG <= lbl && lbl <= NOUN)
  {
    ToNNString(value, GetValue(lbl));
    Change(lbl);
    return;
  }
  if (base == DEC)
  {
    bool leadingZeros = Config::_OutputFlags & Config::MaskLeading0;
    if (-99999L <= value && value <= +99999L)
    {
      char buffer[8];
      char* ptr = buffer + 6;
      *ptr = 0;
      ptr--;
      buffer[0] = (value >= 0)?'+':'-';
      value = abs(value);
      while (ptr > buffer)
      {
        *ptr = '0' + value % 10L;
        value /= 10L;
        ptr--;
      }
      
      if (!leadingZeros)
      {
        // strip them
        ptr = buffer + 1;
        while (*ptr && ptr < (buffer + 6))
        {
          if (*ptr == '0')
            *ptr = ' ';
          else
            break;
          ptr++;
        }
      }
      
      if (strcmp(GetValue(lbl), buffer) != 0)
      {
        numerics.SetValue(lbl, buffer);
      }
    }
  }
  else if (base == OCT)
  {
    if (0L <= value && value <= +077777L)
    {
      char buffer[8];
      char* ptr = buffer + 6;
      *ptr = 0;
      ptr--;
      buffer[0] = ' ';
      while (ptr > buffer)
      {
        *ptr = '0' + value % 8L;
        value /= 8L;
        ptr--;
      }
      if (strcmp(GetValue(lbl), buffer) != 0)
      {
        numerics.SetValue(lbl, buffer);
      }
    }
  }
}

void Numerics::SetBlinking(int lbl, bool on)
{
  if (on)
    _blinkFlags |= 0x0001 << lbl;
  else
    _blinkFlags &= ~(0x0001 << lbl);
  Change(lbl);
}

void Numerics::CompActy(bool on)
{
  word old = _statusFlags;
  if (on)
    _statusFlags |= 0x0001 << COMP_ACTY;
  else
    _statusFlags &= ~(0x0001 << COMP_ACTY);
  if (old != _statusFlags)
    Change(COMP_ACTY);
}

int Numerics::EnterRegister(int lbl)
{
  // decimals start with +/-, octal doesn't, CLR for register entry
  // VN blink while accepting reg input
  // handles entering a register:
  // optional + or - first
  // up to 5 digits, ignored after that
  // CLR clears
  // ENTR ends, returns ENTR if OK
  // other btns end, btn is returned
  ClearDigits(lbl);
  char* r = GetValue(lbl);
  int idx = 0;
  int btn = Keyboard::noButton;
  char digit = '?';

  while (true)
  {
    dsky.CheckBlink();
    if (keyboard.CheckButtonPress(btn))
    {
      if (lights.GetFlag(Lights::OPR_ERR))
      {
        // if there's and error only RSET will do
        if (btn == Keyboard::RSET)
          lights.SetFlag(Lights::OPR_ERR, false);
      }
      else if (btn == Keyboard::PLUS && idx == 0)
      {
        r[idx] = '+';
        Change(lbl);
      }
      else if (btn == Keyboard::MINUS && idx == 0)
      {
        r[idx] = '-';
        Change(lbl);
      }
      else if (keyboard.AsDigit(btn, digit) && idx < 5)
      {
        if (r[0] == ' ' && digit > '7')
        {
          // non-octal digit
          lights.SetFlag(Lights::OPR_ERR, true);
        }
        else
        {
          idx++;
          r[idx] = digit;
          Change(lbl);
        }
      }
      else if (btn == Keyboard::ENTR)
      {
        if (idx != 5)
        {
          // not all 5 digits
          lights.SetFlag(Lights::OPR_ERR, true);
        }
        else
        {
          return btn;
        }
      }
      else if (btn == Keyboard::CLR)
      {
        ClearDigits(lbl);
        r = GetValue(lbl);
        idx = 0;
      }
      else
      {
        return btn;
      }
    }
    Update();
  }
}


static const char Numerics::_labels[] PROGMEM =
  "COMP\nACTY\0"
  "PROG\0"
  "VERB\0"
  "NOUN\0\0";
