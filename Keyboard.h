#pragma once

// the DSKY keyboard
class Keyboard
{
  public:
    enum Buttons 
    {
      firstButton = 0,
      VERB = firstButton,
      NOUN,
      
      PLUS,
      MINUS,
      ZERO,

      SEVEN,
      FOUR,
      ONE,

      EIGHT,
      FIVE,
      TWO,

      NINE,
      SIX,
      THREE,

      CLR,
      PRO,
      KEY_REL,

      ENTR,
      RSET,

      lastButton,
      noButton,

      SystemAccess, // not really a button, click a special light for system access
    };

    void Init();
    void Draw();
    
    void GetButtonXY(int btn, int& x, int& y);
    bool CheckButtonPress(int& btn);
    bool IsDigit(int btn);
    bool AsDigit(int btn, char& digit);
    void UnGetButton(int btn);

    void WaitForDown(int& tx, int& ty);
    void WaitForUp();
  private:
    void DrawButton(int btn, bool pressed = false);
    int TouchedButton();
    int m_iPrevReading;
    int m_iPrevState;
    unsigned long m_iTransitionTimeMS;
    int m_UnGotButton = noButton;

    static const char _labels[];
};

extern Keyboard keyboard;
