// Lightweight Arduino DSKY, Mark Wilson 2019
// Replaces the complexity of buttons, LEDs and 7-segment displays with a touch screen shield.
// Get something like the look and feel of the original.
// This is in *NO WAY* an accurate simulation of the DSKY, it's not trying to be.
// Check out the simulator here: http://svtsim.com/moonjs/agc.html
#include <Arduino.h>
// Libraries:
#include <SoftwareI2C.h>

#include "LCD.h"
#include "Config.h"
#include "Graphics.h"
#include "Lights.h"
#include "Numerics.h"
#include "Keyboard.h"
#include "Programs.h"
#include "DSKY.h"
#include "Font5x7.h"
#include "Splash.h"
#include "RTC.h"

// DSKY has three parts:  
//   a panel with an array of indicator lights (top left), (Lights.h)
//   a panel with numeric displays (top right) and (Numeric.h)
//   a calculator-style keyboard (bottom). (Keyboard.h)

// NOTE:
// *Program space for additional features can be created by undefining USE_MOON_GRAPHICS in Moon.h
//   This will simplify the moon graphics to just a white disk. See also the other USE_ macros in Programs.cpp
// *The digitiser parameters may need adjusting for different LCD/Touch modules (so you can touch near the edges)
//   See the XC4630_TOUCH* constants in XC4630d.h

// Revisions:
// Dec 19: Initial

void setup() 
{
  //Serial.begin(38400);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Config::Load();
  rtc.Setup();
  SERIALISE_ON(true);
  lcd.init();
  SERIALISE_ON(false);
  DoSplash();
  dsky.Init();
  dsky.Draw();
  dsky.Start();
  dsky.CheckAutoRun();
}

void loop() 
{
  dsky.Loop();
}
