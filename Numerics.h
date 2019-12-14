#pragma once

// the numeric display panel on the right of the DSKY
class Numerics
{
  public:
    enum Labels
    {
      firstLabel = 0,
      COMP_ACTY = firstLabel,
      PROG,
      VERB,
      NOUN,
      lastLabel,
      
      R1,
      R2,
      R3
    };
  
    void Init(bool reinit = false);
    void Draw();
    void Update();
    void Blink();
    void ClearDigits(int lbl);
    bool AddDigit(int lbl, int btn);
    long GetDigits(int lbl);
    char* GetValue(int lbl);
    void SetValue(int lbl, const char* value);
    void SetValue(int lbl, long value, int base = DEC);
    void SetBlinking(int lbl, bool on);
    void CompActy(bool on);
    int EnterRegister(int lbl);
    static void ToNNString(long value, char* pBuffer, bool leading0=true);

    static const int PartialValue = -1;
    static const int EmptyValue = -2;

  private:
    void DrawIndicator(int x, int y, int lbl, int h = Config::NumericLabelCellHeight);
    void DrawRegister(int x, int& y, int lbl, word foreground, word background);
    void DrawStr(int x, int y, const char* str, word foreground, word background, int lbl);
    void DrawDigit(int x, int y, char ch, word foreground, word background);
    bool Changed(int lbl) { return (_changedFlags & (0x0001 << lbl)) != 0; }
    bool Blinking(int lbl) { return (_blinkFlags & (0x0001 << lbl)) != 0; }
    void Change(int lbl)  { _changedFlags |= (0x0001 << lbl); }
    word _statusFlags;
    word _changedFlags;
    byte _blinkFlags;
    char _program[3];
    char _verb[3];
    char _noun[3];
    char _registers[3][7];  // +NNNNN
    bool _blink;
    
    int _originX = 0;
    int _originY = 0;
  
    static const char _labels[];
};

extern Numerics numerics;
