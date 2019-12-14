#include <Arduino.h>
#include "Graphics.h"
#include "Lights.h"
#include "Keyboard.h"

// the buttons
Keyboard keyboard;

static const char Keyboard::_labels[] PROGMEM =
  "VERB\0"
  "NOUN\0"
  
  "+\0"
  "-\0"
  "0\0"
  
  "7\0"
  "4\0"
  "1\0"
  
  "8\0"
  "5\0"
  "2\0"
  
  "9\0"
  "6\0"
  "3\0"
  
  "CLR\0"
  "PRO\0"
  "KEY\nREL\0"
  
  "ENTR\0"
  "RSET\0\0";

void Keyboard::Init()
{
  m_iPrevReading = lastButton;
  m_iPrevState = noButton;
  m_iTransitionTimeMS = millis();  
}

void Keyboard::Draw()
{
  // draw all the buttons
  for (int btn = keyboard.firstButton; btn < keyboard.lastButton; btn++)
    DrawButton(btn);
}

void Keyboard::GetButtonXY(int btn, int& x, int& y)
{
  // get the top-left corner coords of the given button
  if (btn > NOUN) btn++;

  // col 0 is leftmost, row 0 is bottom
  // col 0 & 6 are special, buttons at rows 0 & 1 but up half a row
  int col = btn / 3;
  int row = 2 - (btn % 3);
  x = Config::ButtonOffsetLeft + col*(Config::ButtonWidth + Config::ButtonGapX);
  y = Config::DisplayHeight - (Config::ButtonOffsetBottom + (row + 1)*Config::ButtonHeight + row*Config::ButtonGapY);
  if (!(col % (Config::ButtonsAcross - 1)))
  {
    y += (Config::ButtonHeight + Config::ButtonGapY)/2;
  }
}


void Keyboard::DrawButton(int btn, bool pressed)
{
  // draw just the given button, including label
  int x, y, w = Config::ButtonWidth, h = Config::ButtonHeight;
  GetButtonXY(btn, x, y);
  int offset = 1;
  if (pressed)
  {
    // flash it
    if (Config::_InputFlags & Config::MaskFlash)
    {
      lcd.fillColour(lcd.beginFill(x, y, w, h), Config::BackgroundColour);
      delay(25);
    }
    else
    {
      // clear any pixels from un-pressed image
      lcd.fillColour(lcd.beginFill(x, y, offset + 1, h), Config::BackgroundColour);
      lcd.fillColour(lcd.beginFill(x, y, w, offset + 1), Config::BackgroundColour);
    }
    // draw btn offset left and down
    x += offset;
    y += offset;
  }
  else
  {
    // clear any pixels from pressed image
    lcd.fillColour(lcd.beginFill(x + w - offset, y, offset + 1, h), Config::BackgroundColour);
    lcd.fillColour(lcd.beginFill(x, y + h - offset, w, offset + 1), Config::BackgroundColour);
  }
  
  unsigned long fill = lcd.beginFill(x, y, w, h);
  // rounded corners
  lcd.fillColour(1, Config::BackgroundColour);
  lcd.fillByte(w - 2, 0x00);
  lcd.fillColour(1, Config::BackgroundColour);
  lcd.fillByte(fill - 2*w, 0x00);
  lcd.fillColour(1, Config::BackgroundColour);
  lcd.fillByte(w - 2, 0x00);
  lcd.fillColour(1, Config::BackgroundColour);
  char* pLabel = graphics.LookupLabel(btn, _labels);
  graphics.DrawLabel(x + w/2, y + h/2, pLabel, Config::KeyColour, Config::ForegroundColour, (strlen(pLabel) == 1)?2:1);
}

int Keyboard::TouchedButton()
{
  // get the btn currently touched, or noButton
  int tx, ty;
  if (lcd.getTouch(tx, ty))
  {
    if (lights.Touched(tx, ty) == Lights::SystemAccess)
      return Keyboard::SystemAccess;
    for (int btn = keyboard.firstButton; btn < keyboard.lastButton; btn++)
    {
      int bx1, by1,  bx2, by2;
      GetButtonXY(btn, bx1, by1);
      bx2 = bx1 + Config::ButtonWidth;
      by2 = by1 + Config::ButtonHeight;
      if (bx1 <= tx && tx <= bx2 && by1 <= ty && ty <= by2)
        return btn;
    }
  }
  return noButton;
}

void Keyboard::WaitForDown(int& tx, int& ty)
{
  // wait for touch down, with semi-debounce
  int tx1, ty1;
  while (true)
  {
    while (!lcd.getTouch(tx1, ty1))
      ;
    delay(10);
    while (!lcd.getTouch(tx, ty))
      ;

    if (abs(tx-tx1) < 5 && abs(ty-ty1) < 5)
      break;
  }
}

void Keyboard::WaitForUp()
{
  int tx, ty;
  while (lcd.getTouch(tx, ty))
    ;
  delay(10);
  while (lcd.getTouch(tx, ty))
    ;
}


#define HOLD_TIME_MS 50L

bool Keyboard::CheckButtonPress(int& btn)
{
  // get a debounced btn press, true if touching, btn filled in
  if (m_UnGotButton != noButton)
  {
    btn = m_UnGotButton;
    m_UnGotButton = noButton;
    return true;
  }
  
  // debounced button, true if button pressed
  int ThisReading = TouchedButton();
  if (ThisReading != m_iPrevReading)
  {
    // state change, reset the timer
    m_iPrevReading = ThisReading;
    m_iTransitionTimeMS = millis();
  }
  else if (ThisReading != m_iPrevState &&
           (millis() - m_iTransitionTimeMS) >= HOLD_TIME_MS)
  {
    // a state other than the last one and held for long enough
    btn = m_iPrevState = ThisReading;
    if (btn < lastButton)
    {
      // yes, pressed
      DrawButton(btn, true);
      WaitForUp();
      DrawButton(btn, false);
      return true;
    }
    else if (btn == SystemAccess)
    {
      // special case, clicking on a lamp to get system access
      lights.DrawIndicator(Lights::SystemAccess, true);
      WaitForUp();
      lights.DrawIndicator(Lights::SystemAccess, false);
      return true;
    }
    
  }
  return false;
}

static const char* _charMap = "  +-0741852963     ";

bool Keyboard::IsDigit(int btn)
{
  return ZERO <= btn && btn <= THREE;
}

bool Keyboard::AsDigit(int btn, char& digit)
{
  // returns the digit corresponding to the btn
  if (IsDigit(btn))
  {
    digit = _charMap[btn];
    return true;
  }
  return false;
}

void Keyboard::UnGetButton(int btn)
{
  // "push" the btn so it will be processed next
  m_UnGotButton = btn;
}
