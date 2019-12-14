#include <Arduino.h>
#include "Graphics.h"
#include "Config.h"
#include "Lights.h"
#include "Numerics.h"
#include "Keyboard.h"
#include "Font5x7.h"
#include "Programs.h"
#include "Help.h"
#include "DSKY.h"

// the main class
DSKY dsky;

void DSKY::Init(bool reinit)
{
  // reinit everything
  _state = Ready;
  lights.Init(reinit);
  numerics.Init(reinit);
  keyboard.Init();
  _CurrentProgramTimerMS = _BlinkTimerMS = millis();
  if (!reinit)
    _uptimeBaseSeconds = _CurrentProgramTimerMS/1000L;
}

void DSKY::Draw()
{
  // draw everything from scratch
  lcd.fillColour(lcd.beginFill(0, 0, LCD_WIDTH, LCD_HEIGHT), Config::BackgroundColour);
  // screw details. corners
  graphics.DrawDetail(0, 0, 0x101193D3D1902FCULL, Config::ForegroundColour, Config::BackgroundColour);
  graphics.DrawDetail(LCD_WIDTH-8, LCD_HEIGHT-8, 0x101193D3D1902FCULL, Config::ForegroundColour, Config::BackgroundColour, true);
  graphics.DrawDetail(LCD_WIDTH-8, 0, 0x808098BCBC98403FULL, Config::ForegroundColour, Config::BackgroundColour);
  graphics.DrawDetail(0, LCD_HEIGHT-8, 0x808098BCBC98403FULL, Config::ForegroundColour, Config::BackgroundColour, true);
  // midline
  int midLineY = Config::PanelRectOffsetY + Config::PanelRectHeight + 1;
  graphics.DrawDetail(0, midLineY, 0xFC02193D3D1902FCULL, Config::ForegroundColour, Config::BackgroundColour);
  graphics.DrawDetail((LCD_WIDTH-8)/2, midLineY, 0x3C4299BDBD99423CULL, Config::ForegroundColour, Config::BackgroundColour);
  graphics.DrawDetail(LCD_WIDTH-8, midLineY, 0xFC02193D3D1902FCULL, Config::ForegroundColour, Config::BackgroundColour, true);
  // mid top
  graphics.DrawDetail((LCD_WIDTH-8)/2, 0, 0x818199BDBD99423CULL, Config::ForegroundColour, Config::BackgroundColour);

  Update();
}

void DSKY::Loop()
{
  // update blinking, check btn presses, time-slice the running program
  if (_pCurrentProgram == Programs::MonitorGraphicalProgram)
  {
    // this is a bit of a hack, a graphical program takes over
    CheckCurrentProgram();
    return;
  }
  CheckBlink();
  int btn;
  if (keyboard.CheckButtonPress(btn))
  {
    ProcessKey(btn);
  }
  else
  {
    CheckCurrentProgram();
  }
}

void DSKY::Start()
{
  delay(1000);
  lights.SetFlag(Lights::lastIndicator, false);
}

void DSKY::Update()
{
  // the three main elements
  lights.Draw();
  numerics.Draw();
  keyboard.Draw();
  _BlinkTimerMS = millis();
}

void DSKY::CheckBlink()
{
  // check if it's time to blink
  unsigned long NowMS = millis();
  if (NowMS - _BlinkTimerMS > 750UL)
  {
    lights.Blink();
    numerics.Blink();
    _BlinkTimerMS = NowMS;
  }
}

void DSKY::ProcessKey(int btn)
{
  // process a key-press
  switch (btn)
  {
    case keyboard.VERB:
    {
      _state = Verb;
      numerics.ClearDigits(numerics.VERB);
      _CurrentProgramVerb = _CurrentProgramNoun = 0;
      break;
    }
    case keyboard.NOUN:
    {
      _state = Noun;
      numerics.ClearDigits(numerics.NOUN);
      _CurrentProgramVerb = _CurrentProgramNoun = 0;
      break;
    }
    case keyboard.CLR:
    {
      lights.SetFlag(Lights::lastIndicator, false);
      break;
    }
    case keyboard.PRO:
    {
      EndCurrentProgram();
      break;
    }
    case keyboard.KEY_REL:
    {
      lights.SetFlag(Lights::KEY_REL, false);
      break;
    }
    case keyboard.ENTR:
    {
      ProcessEnter();
      break;
    }
    case keyboard.RSET:
    {
      lights.SetFlag(Lights::lastIndicator, false); // clear all
      EndCurrentProgram();
      break;
    }
    case keyboard.SystemAccess:
    {
      DoSystemClick();
      break;
    }
    default:  // digits
    {
      if (keyboard.IsDigit(btn))
      {
        if (_state == Verb)
        {
          if (numerics.AddDigit(numerics.VERB, btn))
            _state = Ready;
        }
        else if (_state == Noun)
        {
          if (numerics.AddDigit(numerics.NOUN, btn))
            _state = Ready;
        }
      }
      break;
    }
  }  
}

void DSKY::ProcessEnter()
{
  // handle ENTR
  int verb = _CurrentProgramVerb?_CurrentProgramVerb:numerics.GetDigits(numerics.VERB);
  int noun = _CurrentProgramNoun?_CurrentProgramNoun:numerics.GetDigits(numerics.NOUN);
  numerics.CompActy(false);
  numerics.Update();
  if (lights.GetFlag(Lights::OPR_ERR)) // OPR ERR needs to be cleared before anything will run
    return;
  if (Programs::StartProgram(verb, noun, _pCurrentProgram))
  {
    // program is good
    _CurrentProgramVerb = verb;
    _CurrentProgramNoun = noun;
    _CurrentProgramTick = 0;
  }
  else
  {
    // program is bad
    _pCurrentProgram = NULL;
    _CurrentProgramVerb = _CurrentProgramNoun = 0;
    lights.SetFlag(Lights::OPR_ERR, true);
    lights.Update();
  }
}


void DSKY::CheckCurrentProgram()
{
  // is it time to call the running program
  unsigned long NowMS = millis();
  if ((NowMS - _CurrentProgramTimerMS) > 100UL)
  {
    _CurrentProgramTimerMS = NowMS;
    if (_pCurrentProgram)
    {
      _CurrentProgramTick++;
      if (_CurrentProgramTick == 0)
        _CurrentProgramTick = 1;  // only ever see 0 once, to indicate first time run
      if (!_pCurrentProgram(_CurrentProgramVerb, _CurrentProgramNoun, _CurrentProgramTick))
      {
        _pCurrentProgram = NULL;
      }
    }
  }
}

void DSKY::EndCurrentProgram()
{
  numerics.CompActy(false);
  numerics.Update();
  _pCurrentProgram = NULL;
}

void DSKY::DoSystemClick()
{
  // special non-DSKY click goes to help
  Help::Draw();
  int x,y;
  keyboard.WaitForDown(x, y);
  keyboard.WaitForUp();
  Init(true); 
  Draw(); 
}

void DSKY::CheckAutoRun()
{
  // is there a saved VN pair to exec on startup?
  if (1 <= Config::_AutoVerb && Config::_AutoVerb <= 99 && 
      1 <= Config::_AutoNoun && Config::_AutoNoun <= 99)
  {
    _CurrentProgramVerb = Config::_AutoVerb;
    _CurrentProgramNoun = Config::_AutoNoun;
    ShowVerbNoun(_CurrentProgramVerb, _CurrentProgramNoun); 
    ProcessEnter();
  }
}

void DSKY::ShowVerbNoun(int verb, int noun)
{
  numerics.SetValue(Numerics::VERB, verb);
  numerics.SetValue(Numerics::NOUN, noun);
  numerics.Update();
}
