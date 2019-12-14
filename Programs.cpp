#include <Arduino.h>
#include "Moon.h"
#include "LCD.h"
#include "Graphics.h"
#include "Config.h"
#include "Lights.h"
#include "Numerics.h"
#include "Keyboard.h"
#include "Font5x7.h"
#include "Programs.h"
#include "RTC.h"
#include "DSKY.h"

// All on
#define USE_ENTER_DATA_PROGRAM
#define USE_PLEASE_PERFORM_PROGRAM
#define USE_IGNITION_PROGRAM

// Turn things off selectively
// Something's gotta go if the graphics are enabled
#ifdef USE_MOON_GRAPHICS
#undef USE_PLEASE_PERFORM_PROGRAM
#undef USE_IGNITION_PROGRAM
#endif
// Make space for serial output of graphics
#ifdef SERIALIZE
#undef USE_ENTER_DATA_PROGRAM
#endif


enum Verbs
{
  DisplayOctal     = 5,
  DisplayDecimal   = 6,
  MonitorOctal     = 15,
  MonitorDecimal   = 16,
  MonitorGraphical = 18,
  EnterData        = 25,
  LampTest         = 35,
  PleasePerform    = 50,
  EngineIgnition   = 99
};

enum Nouns
{
  UpTime        = 24,
  MoonAge       = 28,
  AutoRun       = 29,
  OutputFlags   = 30,
  InputFlags    = 31,
  ClockTime     = 36,
  ClockDate     = 39,
  
  StackClr = 90, StackPush = 91, StackPop = 92, StackDup = 93, StackAdd = 94, 
  StackSub = 95, StackMult = 96, StackDiv = 97, StackNeg = 98, StackSwap = 99  
};

typedef long StackType;
const int kStackSize = 10;
const StackType MaxValue = +99999;
static int _TOS = -1;
static StackType _stack[kStackSize];

bool Programs::StartProgram(byte verb, byte noun, Program& program)
{
  // Try to start/execute the program/actions corresponding to VN
  // Returns true if VN is a recognised combo, and if the program started or executed OK
  // Sets program to point to the function to call if the program should continue running
  if (verb == DisplayOctal || verb == DisplayDecimal)
  {
    // display one time
    program = NULL;
    return MonitorProgram(verb, noun, 0);
  }
  else if (verb == MonitorOctal || verb == MonitorDecimal)
  {
    if (MonitorProgram(verb, noun, 0))
    {
      // keep displaying
      program = MonitorProgram;
      return true;
    }
  }
  else if (verb == EnterData)
  {
    program = NULL;
    return EnterDataProgram(verb, noun);
  }
  else if (verb == LampTest)
  {
    LampTestProgram();
    program = NULL;
    return true;
  }
  else if (verb == MonitorGraphical)
  {
    if (MonitorGraphicalProgram(verb, noun, 0))
    {
      // keep displaying
      program = MonitorGraphicalProgram;
      return true;
    }
  }
  else if (verb == PleasePerform)
  {
    program = NULL;
    return PleasePerformProgram(verb, noun);
  }
  else if (verb == EngineIgnition)
  {
    if (EngineIgnitionProgram(verb, noun, 0))
    {
      // keep displaying
      program = EngineIgnitionProgram;
      return true;
    }
  }
  program = NULL;
  return false;
}


void Programs::FlashCompActy()
{
  // flash COMP ACTY light
  numerics.CompActy(true);
  numerics.Update();
  delay(100);
  numerics.CompActy(false);
  numerics.Update();
}

void SetVNBlinking(bool blink)
{
  numerics.SetBlinking(Numerics::VERB, blink);
  numerics.SetBlinking(Numerics::NOUN, blink);
}

void Programs::LampTestProgram()
{
  // flash a bunch of things
  lights.SetFlag(Lights::lastIndicator, true);
  numerics.SetValue(Numerics::VERB, "88");
  numerics.SetValue(Numerics::NOUN, "88");
  SetVNBlinking(true);
  numerics.SetValue(Numerics::R1, "+88888");
  numerics.SetValue(Numerics::R2, "+88888");
  numerics.SetValue(Numerics::R3, "+88888");
  dsky.Update();
  
  unsigned long startTimeMS = millis();
  do
  {
    dsky.CheckBlink();
  } while (millis() - startTimeMS < 8000L);
  
  lights.SetFlag(Lights::lastIndicator, false);
  SetVNBlinking(false);
  numerics.Update();
}

int GetHour()
{
  int h;
  if (Config::_OutputFlags & Config::Mask24Hour)
  {
    h = rtc.m_Hour24;
  }
  else
  {
    h = rtc.m_Hour24;
    if (h > 12)
      h -= 12;
    if (h == 0)
      h = 12;
  }
  return h;
}

long Sinusoid(long amplitude, long period, long phase, unsigned long t)
{
  t %= period;
  return rcosx(amplitude, ((t*360L/period) + phase) % 360);
}
void Do1202Alarm()
{
    // 1202 alarm easter egg
    dsky.ShowVerbNoun(5, 9);
    numerics.SetValue(Numerics::PROG, "63");
    numerics.SetValue(Numerics::R1, "  1202");
    numerics.ClearDigits(Numerics::R2);
    numerics.ClearDigits(Numerics::R3);
    numerics.CompActy(false);
    numerics.Update();
    lights.SetFlag(Lights::PROG, true);
    lights.Update();
    delay(2500);
    dsky.ShowVerbNoun(dsky._CurrentProgramVerb, dsky._CurrentProgramNoun);
    numerics.SetValue(Numerics::PROG, "  ");
    lights.SetFlag(Lights::PROG, false);
    lights.Update();
}

bool Programs::MonitorProgram(byte verb, byte noun, unsigned long tick)
{
  // Display the thing corresponding to noun.  
  // Verb indicates dec/hex, and one-off (display) or recurring (monitor)
  static bool eggDone = false;
  long r1;
  long r2;
  long r3;
  r1 = r2 = r3 = -1;
  int Base = (verb % 2)?OCT:DEC;
  if (tick == 0)
    eggDone = false;
  if (noun == UpTime)
  {
    // uptime
    long total = millis()/1000L - dsky._uptimeBaseSeconds;
    r3 = total % 60L;
    total /=  60L;
    r2 = total % 60L;
    r1 = total/60L;
  }
  else if (noun == ClockTime)
  {
    // SAMPLED LGC TIME
    rtc.ReadTime(true);
    r1 = GetHour();
    r2 = rtc.m_Minute;
    r3 = rtc.m_Second;
    if (r1 == 12 && r2 == 2)
    {
      if (!eggDone)
      {
        Do1202Alarm();
        eggDone = true;
      }
    }
    else
    {
      eggDone = false;
    }
  }
  else if (noun == ClockDate)
  {
    // DATE
    word fmt = Config::_OutputFlags & Config::MaskDateFmt;
    rtc.ReadTime(true);
    if (fmt == 1) // DMY
    {
      r1 = rtc.m_DayOfMonth;
      r2 = rtc.m_Month;
      r3 = rtc.m_Year + 2000;
    }
    else if (fmt == 2)  // MDY
    {
      r1 = rtc.m_Month;
      r2 = rtc.m_DayOfMonth;
      r3 = rtc.m_Year + 2000;
    }
    else  // YMD
    {
      r1 = rtc.m_Year + 2000;
      r2 = rtc.m_Month;
      r3 = rtc.m_DayOfMonth;
    }
  }
  else if (noun == OutputFlags || noun == InputFlags)
  {
    r1 = (noun == OutputFlags)?Config::_OutputFlags:Config::_InputFlags;
  }
  else if (noun == AutoRun)
  {
    r1 = Config::_AutoVerb;
    r2 = Config::_AutoNoun;
  }
  else if (noun == MoonAge)
  {
    if (Config::_RefNewMoonSeconds)
    {
      int AgeDays, Angle, DaysUntilPlus1;
      rtc.ReadTime(true);
      Moon::CalcAge(//Config::_RefNewMoonSeconds, rtc.m_DayOfMonth, rtc.m_Month, rtc.m_Year + 2000, rtc.m_Hour24, rtc.m_Minute, 
                    AgeDays, Angle, DaysUntilPlus1); 
      r1 = AgeDays; // age in days
    }
  }
  else if (StackClr <= noun && noun <= StackSwap)
  {
    // display/monitor for any stack noun displays the TOS
    if (_TOS >= 0)
      r1 = _stack[_TOS];
  }
  else if (verb == EngineIgnition)
  {
    // just fake some numbers and flash some lights, this is all arbitrary...
    Base = DEC;
    r1 = Sinusoid(9999L,   5000L, 0L, tick);
    r2 = Sinusoid(99L,    10000L, 100L, tick);
    r3 = Sinusoid(99999L,  2500L, 1000L, tick);
    int rnd = random(5000);
    if (rnd == 1202)
      Do1202Alarm();
    else if (rnd < 100)
      lights.SetFlag(Lights::GIMBAL_LOCK, true);
    else if (rnd < 200)
      lights.SetFlag(Lights::TEMP, true);
    else if (rnd < 300)
      lights.SetFlag(Lights::UPLINK_ACTY, true);
    else if (rnd < 400)
      lights.SetFlag(Lights::ALT, true);
    else if ((tick % 100) == 0)
      lights.SetFlag(Lights::lastIndicator, false);
  }
  
  if (r1 != -1)
  {
    // flash COMP ACTY periodically
    numerics.CompActy((tick % 10) < 2);
    numerics.SetValue(Numerics::R1, r1, Base);
    if (r2 != -1)
      numerics.SetValue(Numerics::R2, r2, Base);
    if (r3 != -1)
      numerics.SetValue(Numerics::R3, r3, Base);
    numerics.Update();
    return true;
  }
  return false;
}

static const char MoonPhaseNames[] PROGMEM ="New Moon\0Waxing Crescent\0First Quarter\0Waxing Gibbous\0Full Moon\0Waning Gibbous\0Last Quarter\0Waning Crescent\0\0";

bool Programs::MonitorGraphicalProgram(byte , byte noun, unsigned long tick)
{
  // Display the thing corresponding to noun graphcally: only the moon phase!
  static int currentAngle = 0;
  if (noun == MoonAge && Config::_RefNewMoonSeconds)
  {
    int tx1, ty1;
    if (tick != 0 && lcd.getTouch(tx1, ty1))
    {
      keyboard.WaitForUp();
      // exit back to DSKY
      dsky.Init(true);
      dsky.Draw();
      dsky.ShowVerbNoun(MonitorGraphical, MoonAge);
      return false;
    }
    if (tick == 0)
    {
      currentAngle = -1;
    }
    else if (tick % 100)
    {
      // update less frequently
      return true;
    }
    int scale = 2;
    char buff[20];
    int AgeDays, Angle, DaysUntilPlus1;
    rtc.ReadTime(true);
    // do the moon info
    Moon::CalcAge(//Config::_RefNewMoonSeconds, rtc.m_DayOfMonth, rtc.m_Month, rtc.m_Year + 2000, rtc.m_Hour24, rtc.m_Minute, 
                  AgeDays, Angle, DaysUntilPlus1);
    if (currentAngle != Angle)
    {
      // clear:
      lcd.fillByte(lcd.beginFill(0, 0, LCD_WIDTH, LCD_HEIGHT), 0);  // low on program space, could just update but don't
      currentAngle = Angle;
      // phase graphics
      Moon::DrawPhase(Angle);
      
      // days until
      if (DaysUntilPlus1 > 0) // full is next
        strcpy(buff, graphics.LookupLabel(4, MoonPhaseNames));
      else  // new is next
        strcpy(buff, graphics.LookupLabel(0, MoonPhaseNames));
      int DaysUntil = abs(DaysUntilPlus1) - 1;
      if (DaysUntil)
      {
        strcat(buff, " in ~");
        Numerics::ToNNString(DaysUntil, buff + strlen(buff), false);
        strcat(buff, "d");
        Font5x7::drawText((lcd._width - (FONT5x7_WIDTH + 1)*strlen(buff)*scale)/2, lcd._height - (FONT5x7_HEIGHT + 2)*scale, buff, 0xFFFF, 0x0000, scale, false, true);
      }
      
      // phase name
      // divide by 22.5deg
      Angle *= 10;
      int idx = Angle/225;
      if ((Angle % 225) >= (225/2)) // round up
        idx++;
      char* pName = graphics.LookupLabel(idx % 8, MoonPhaseNames);
      Font5x7::drawText((lcd._width - (FONT5x7_WIDTH + 1)*strlen(pName)*scale)/2, lcd._height - (FONT5x7_HEIGHT + 2)*scale*((DaysUntil)?2:1), pName, 0xFFFF, 0x0000, scale, false, true);
    }

    // time
    Numerics::ToNNString(GetHour(), buff, false);
    strcat(buff, ":");
    Numerics::ToNNString(rtc.m_Minute, buff + strlen(buff));
    Font5x7::drawText((lcd._width - (FONT5x7_WIDTH + 1)*5*scale)/2, 2, buff, 0xFFFF, 0x0000, scale, false, true);
    
    return true;
  }
  return false;
}

bool Programs::EnterDataProgram(byte , byte noun)
{
#ifndef USE_ENTER_DATA_PROGRAM
// free up program space temporarily
  (void)noun;
  return false;
#else  
  // Enter 1, 2 or 3 things, via the registers, true if valid & completed
  bool ran = false;
  int lastReg = Numerics::lastLabel;
  if (noun == ClockTime || noun == ClockDate || noun == UpTime || noun == MoonAge)
    lastReg = Numerics::R3; // want all three
  else if (noun == OutputFlags || noun == InputFlags)
    lastReg = Numerics::R1; // just want 1
  else if (noun == AutoRun)
    lastReg = Numerics::R2; // verb & noun
  
  int btn = Keyboard::noButton;
  if (lastReg != Numerics::lastLabel)
  {
    SetVNBlinking(true);
    for (int r = Numerics::R1; r <= lastReg; r++)
    {
      btn = numerics.EnterRegister(r);
      if (btn != Keyboard::ENTR)
      {
        // ended on a different btn press, unget it so it's handled as normal
        keyboard.UnGetButton(btn);
        break;
      }
    }
  }

  if (btn == Keyboard::ENTR)
  {
    // ended on an enter, all the registers are there, use 'em
    if (noun == ClockTime || noun == ClockDate)
    {
      if (noun == ClockTime)
      {
        // hour, min sec
        long h = numerics.GetDigits(Numerics::R1);
        long m = numerics.GetDigits(Numerics::R2);
        long s = numerics.GetDigits(Numerics::R3);
        if (0 <= h && h <= 23 && 0 <= m && m <= 59 && 0 <= s && s <= 59)
        {
          rtc.ReadTime(true);
          rtc.m_Hour24 = h;
          rtc.m_Minute = m;
          rtc.m_Second = s;
          ran = true;
        }
      }
      else
      {
        // year, month, day
        long y = numerics.GetDigits(Numerics::R1);
        long m = numerics.GetDigits(Numerics::R2);
        long d = numerics.GetDigits(Numerics::R3);
        if (2001 <= y && y <= 2099 && 1 <= m && m <= 12 && 1 <= d && d <= Time::DaysInMonth(m, y))
        {
          rtc.ReadTime(true);
          rtc.m_Year       = y - 2000;
          rtc.m_Month      = m;
          rtc.m_DayOfMonth = d;
          ran = true;
        }
      }
      if (ran) // write the time
        rtc.WriteTime();
      ran = true;
    }
    else if (noun == OutputFlags)
    {
      // flagword
      Config::_OutputFlags = numerics.GetDigits(Numerics::R1) & 077777;
      Config::Write(&Config::_OutputFlags);
      ran = true;
    }
    else if (noun == InputFlags)
    {
      // flagword
      Config::_InputFlags = numerics.GetDigits(Numerics::R1) & 077777;
      Config::Write(&Config::_InputFlags);
      ran = true;
    }
    else if (noun == AutoRun)
    {
      // verb, noun
      Config::_AutoVerb = numerics.GetDigits(Numerics::R1);
      Config::_AutoNoun = numerics.GetDigits(Numerics::R2);
      if (Config::_AutoVerb <= 99 && Config::_AutoNoun <= 99)
      {
        ran = true;
        Config::Save();
      }
    }
    else if (noun == UpTime)
    {
      // hours, minutes, seconds
      long h = numerics.GetDigits(Numerics::R1);
      long m = numerics.GetDigits(Numerics::R2);
      long s = numerics.GetDigits(Numerics::R3);
      dsky._uptimeBaseSeconds = millis()/1000L - (h*60L*60L + m*60L + s);
      ran = true;
    }
    else if (noun == MoonAge)
    {
      // day, hour, minute (new in d days at h:m)
      long d = numerics.GetDigits(Numerics::R1);
      long h = numerics.GetDigits(Numerics::R2);
      long m = numerics.GetDigits(Numerics::R3);
      if (-29 <= d && d <= +29 && 0 <= h && h <= 23 && 0 <= m && m <= 59)
      {
        rtc.ReadTime(true);
        Config::_RefNewMoonSeconds = Time::MakeSeconds(rtc.m_DayOfMonth, rtc.m_Month, rtc.m_Year + 2000, d*24 + h, m);
        Config::Save();
        ran = true;
      }
    }
  }
  
  if (ran)
  {
    FlashCompActy();
  }
  else
  {
    lights.SetFlag(Lights::OPR_ERR, true);
    lights.Update();
  }
  SetVNBlinking(false);
  numerics.Update();
  dsky.Update();
  return ran;
#endif  
}

bool Programs::PleasePerformProgram(byte , byte noun)
{
  // Only performs stack (RPN calc) operations
  bool error = false;
#ifndef USE_PLEASE_PERFORM_PROGRAM
  (void)noun;
#else
  switch (noun)
  {
    case StackClr:
      _TOS = -1;
      break;
    case StackPush:
    {
      if (_TOS < kStackSize)
      {
        SetVNBlinking(true);
        int btn = numerics.EnterRegister(Numerics::R1);
        SetVNBlinking(false);
        if (btn == Keyboard::ENTR)
        {
          _stack[++_TOS] = numerics.GetDigits(Numerics::R1);;
          break;
        }
        else
          keyboard.UnGetButton(btn);
      }
      error = true;
      break;
    }
    case StackDup:
      if (_TOS < kStackSize && _TOS >= 0)
        {_TOS++;_stack[_TOS] = _stack[_TOS - 1];}
      else  
        error = true;
      break;
    case StackPop:
      if (_TOS >= 0)
        _TOS--;
      else  
        error = true;
      break;
    case StackAdd:
      if (_TOS >= 1)
        {_TOS--;_stack[_TOS] += _stack[_TOS + 1];}
      else  
        error = true;
      break;
    case StackSub:
      if (_TOS >= 1)
        {_TOS--;_stack[_TOS] -= _stack[_TOS + 1];}
      else  
        error = true;
      break;
    case StackMult:
      if (_TOS >= 1)
        {_TOS--;_stack[_TOS] *= _stack[_TOS + 1];}
      else  
        error = true;
      break;
    case StackDiv:
      if (_TOS >= 1 && _stack[_TOS])
        {_TOS--;_stack[_TOS] /= _stack[_TOS + 1];}
      else  
        error = true;
      break;
    case StackNeg:
      if (_TOS >= 0)
        _stack[_TOS] = -_stack[_TOS];
      else  
        error = true;
      break;
    case StackSwap:
      if (_TOS >= 1)
      {
        StackType temp = _stack[_TOS];
        _stack[_TOS] = _stack[_TOS - 1];
        _stack[_TOS - 1] = temp;
      }
      else  
        error = true;
      break;
    default:
      error = true;
      break;
  }

  if (!error)
  {
    if (_TOS != -1)
    {
      if (abs(_stack[_TOS]) > MaxValue)
        error = true;
     else
      {
        FlashCompActy();
        numerics.SetValue(Numerics::R1, _stack[_TOS]);
      }
    }
    else
    {
      numerics.ClearDigits(Numerics::R1);
    }
    numerics.Update();
  }
  if (error)
  {
    lights.SetFlag(Lights::OPR_ERR, true);
    lights.Update();
  }
#endif  
  return !error;
}

bool Programs::EngineIgnitionProgram(byte verb, byte , unsigned long tick)
{
#ifndef USE_PLEASE_PERFORM_PROGRAM
  (void)verb;
  (void)tick;
  return false;
#else
  return MonitorProgram(verb, 1, tick);
#endif
}

bool Programs::GetNotAvailable(char* buff)
{
  // list programs ifdef'd out for lack of memory
  bool NA = false;
#ifndef USE_ENTER_DATA_PROGRAM
  strcat(buff, "N25, ");
  NA = true;
#endif  
#ifndef USE_PLEASE_PERFORM_PROGRAM
  strcat(buff, "N50, V9x, "); // RPN
  NA = true;
#endif  
#ifndef USE_IGNITION_PROGRAM
  strcat(buff, "N99, ");
  NA = true;
#endif
#ifndef USE_MOON_GRAPHICS
  strcat(buff, "Moon image, ");
  NA = true;
#endif
  if (NA)
    buff[strlen(buff)-2] = '\0';
  return NA;
}
