#pragma once

// the panel of indicator lights on the left of the DSKY
class Lights
{
  public:
    enum Indicators
    {
      firstIndicator = 0,
      UPLINK_ACTY = firstIndicator,  
                    TEMP,
      NO_ATT,       GIMBAL_LOCK,
      STBY,         PROG,
      KEY_REL,      RESTART,
      OPR_ERR,      TRACKER,
      UNUSED_L6,    ALT,
      UNUSED_L7,    VEL,
      lastIndicator,
      
      SystemAccess = PROG
    };

    void Init(bool reinit = false);
    void Draw();
    void Update();
    void Blink();
    void SetFlag(int ind, bool on); // lastIndicator sets/clears ALL
    bool GetFlag(int ind);
    int Touched(int x, int y);
    void DrawIndicator(int ind, bool on);

  private:
    void DrawIndicator(int ind);
    word _statusFlags;
    word _blinkMask;
    word _blinkFlags;
    word _currentDisplay;
    word _currentBlink;
    int _originX;
    int _originY;
    static const char _labels[];
};

extern Lights lights;
