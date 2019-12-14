#pragma once

class DSKY
{
  public:
    enum States
    {
      Ready,
      Verb,
      Noun,
    };
    void Init(bool reinit = false);
    void Draw();
    void Start();
    void Loop();
    void Update();
    void CheckBlink();
    void CheckAutoRun();
    void ShowVerbNoun(int verb, int noun);
    unsigned long _uptimeBaseSeconds = 0L;
    byte _CurrentProgramVerb = 0;
    byte _CurrentProgramNoun = 0;
    
  private:
    void ProcessKey(int btn);
    void ProcessEnter();
    void CheckCurrentProgram();
    void EndCurrentProgram();
    void DoSystemClick();
    States _state;
    unsigned long _CurrentProgramTick = 0;
    Programs::Program _pCurrentProgram = NULL;
    unsigned long _BlinkTimerMS = 0L;
    unsigned long _CurrentProgramTimerMS = 0L;
};

extern DSKY dsky;
