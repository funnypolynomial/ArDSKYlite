#include <Arduino.h>
#include "Graphics.h"
#include "Lights.h"

// the light-up indicators on the left panel
Lights lights;

static const char Lights::_labels[] PROGMEM =
  "UPLINK\nACTY\0"  "TEMP\0"
  "NO ATT\0"        "GIMBAL\nLOCK\0"
  "STBY\0"          "PROG\0"
  "KEY REL\0"       "RESTART\0"
  "OPR ERR\0"       "TRACKER\0"
  "\0"              "ALT\0"
  "\0"              "VEL\0\0";

void Lights::Init(bool reinit)
{
  // status
  _statusFlags = 0x0001 << RESTART;
  if (reinit)
    _statusFlags = 0;
  // displayed status
  _currentDisplay = ~_statusFlags;
  // these are blinkable
  _blinkMask = (0x0001 << OPR_ERR) | (0x0001 << KEY_REL);
  // blinking
  _blinkFlags = 0;
  // blink displayed
  _currentBlink = 0;
  _originX = Config::PanelRectOffsetX + Config::PanelInnerOffset + Config::PanelRectThickness + Config::LightsSideOffset;
  _originY = Config::PanelRectOffsetY + Config::PanelInnerOffset + Config::PanelRectThickness + Config::LightsTopOffset;
}

void Lights::Draw()
{
  // draw the whole indicator lights panel;
  graphics.DrawPanel(Config::PanelRectOffsetX, Config::PanelRectOffsetY);
  Update();
}

void Lights::Update()
{
  // draw the lights that have changed since last time
  for (int ind = firstIndicator; ind < lastIndicator; ind++)
  {
    word mask = 0x0001 << ind;
    if ((_currentDisplay & mask) != (_statusFlags & mask) || (_currentBlink & mask) != (_blinkFlags & mask))
      DrawIndicator(ind);
  }
  _currentDisplay = _statusFlags;
  _currentBlink = _blinkFlags;
}

void Lights::Blink()
{
  if (_statusFlags & _blinkMask)
  {
    _blinkFlags ^= _blinkMask;
    Update();
  }
  else
  {
    _blinkFlags = 0;
  }
}

void Lights::SetFlag(int ind, bool on)
{
  word mask = 0x0001 << ind;
  if (on)
  {
    if (ind >= lastIndicator) // all
      _statusFlags = 0xFFFF;
    else
      _statusFlags |= mask;
  }
  else
  {
    if (ind >= lastIndicator)
      _statusFlags = 0x0000;  // all
    else
      _statusFlags &= ~mask;
  }
  Update();
}

bool Lights::GetFlag(int ind)
{
  return (_statusFlags & (0x0001 << ind)) != 0;
}


void Lights::DrawIndicator(int ind)
{
    word mask = 0x0001 << ind;
    bool blinkOff = (_blinkMask & mask) != 0 && (_blinkFlags & mask) != 0;
    bool on = (_statusFlags & mask) != 0 && !blinkOff;
    DrawIndicator(ind, on);
}

void Lights::DrawIndicator(int ind, bool on)
{
    int row = ind / Config::LightsCols;
    int col = ind % Config::LightsCols;
    int originX = _originX + col*(Config::LightsCellWidth + Config::LightsRowGap);
    int originY = _originY + row*(Config::LightsCellHeight + Config::LightsRowGap);
    word lampColour = col?Config::IndicatorYellow:Config::IndicatorWhite;
    word bkColour = on?lampColour:Config::IndicatorOffColour;
    graphics.DrawRect(originX, originY, Config::LightsCellWidth, Config::LightsCellHeight, 0, Config::ForegroundColour, bkColour);
    graphics.DrawLabel(originX + Config::LightsCellWidth/2, originY + Config::LightsCellHeight/2, graphics.LookupLabel(ind, _labels), Config::ForegroundColour, bkColour, 1);
}

int Lights::Touched(int x, int y)
{
    // returns touched light, SystemAccess or lastIndicator (i.e. N/A)
    int row = SystemAccess / Config::LightsCols;
    int col = SystemAccess % Config::LightsCols;
    int originX = _originX + col*(Config::LightsCellWidth + Config::LightsRowGap);
    int originY = _originY + row*(Config::LightsCellHeight + Config::LightsRowGap);
    if (originX <= x && x <= originX + Config::LightsCellWidth &&
        originY <= y && y <= originY +  Config::LightsCellHeight)
   {
      return SystemAccess;
   }
   return lastIndicator;
}
