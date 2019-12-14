#pragma once

// undefine this to turn moon image off and increase program space (by about 9k)
#define USE_MOON_GRAPHICS

// no, these doesn't really belong here :-) but there are already too many files
extern long rcosx(long r, long x);
class Time
{
  public:
    static int DaysInMonth(int Month, int Year);
    static long DayOfCentury(int Date, int Month, int Year);
    static unsigned long MakeSeconds(int Date, int Month, int Year, int Hour, int Minute);
};

class Moon
{
  public:
    // NOT passing these parameters (getting them from Config & rtc) saves 120bytes program space
    static void CalcAge(//unsigned long referenceNewMoonSeconds, int Day, int Month, int Year, int Hour, int Minute, 
                        int& AgeDays, int& Angle, int& DaysUntilPlus1);
    static void CreatePalette(word* palette);
    static void DrawPhase(int phaseAngle);
    static void DrawGore(int startAngle, int endAngle, word* palette, bool wax);
    static void DrawLandingSite();
};
