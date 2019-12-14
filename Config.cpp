#include <Arduino.h>
#include <EEPROM.h>
#include "Config.h"

void Config::Load()
{
  // config layout is
  // 0: output flags high
  // 1: output flags low
  // 2: input flags high
  // 3: input flags low
  // 4: auto-load verb
  // 5: auto-load noun
  // 6-10 reference new moon (unsigned long, big-endian)

  Reset();
  int idx = 0;
  byte chk = EEPROM.read(idx++);
  if (chk != 0xFF)
  {
    // they have been set
    _OutputFlags = (chk << 8) | EEPROM.read(idx++);
    _InputFlags = EEPROM.read(idx++) << 8;
    _InputFlags |= EEPROM.read(idx++);
    _AutoVerb = EEPROM.read(idx++);
    _AutoNoun = EEPROM.read(idx++);
    _RefNewMoonSeconds = 0UL;
    for (int b = 0; b < 4; b++)
    {
      _RefNewMoonSeconds <<= 8;
      _RefNewMoonSeconds |= EEPROM.read(idx++);
    }
  }
}

void Config::Save()
{
  Write(&_OutputFlags);
  int idx = Write(&_InputFlags);
  EEPROM.write(idx++, _AutoVerb);
  EEPROM.write(idx++, _AutoNoun);
  for (int b = 0; b < 4; b++)
  {
    EEPROM.write(idx++, _RefNewMoonSeconds >> (8*(3-b)));
  }
}

void Config::Reset()
{
  // init to defaults:
  _OutputFlags = OutputDefaults;
  _InputFlags = InputDefaults;
  _AutoVerb = 0xFF;
  _AutoNoun = 0xFF;
  _RefNewMoonSeconds = 0UL;
}

int Config::Write(word* flags)
{
  int idx = (flags == &_OutputFlags)?0:2;
  EEPROM.write(idx++, (*flags) >> 8);
  EEPROM.write(idx++, *flags);
  return idx;
}

word Config::_OutputFlags = Config::OutputDefaults;
word Config::_InputFlags = Config::InputDefaults;
byte Config::_AutoVerb = 0xFF;
byte Config::_AutoNoun = 0xFF;
unsigned long  Config::_RefNewMoonSeconds = 0UL;
