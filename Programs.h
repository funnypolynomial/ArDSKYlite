#pragma once

class Programs
{
  public:
    // typedef for program callback, tick is 10ths of seconds, return true if valid/continue
    typedef bool (*Program)(byte verb, byte noun, unsigned long tick);
    // start running the given program, true if VN valid, program non-null if should run again
    static bool StartProgram(byte verb, byte noun, Program& program);
    
    static bool MonitorGraphicalProgram(byte verb, byte noun, unsigned long tick);

    static bool GetNotAvailable(char* buff);
  private:
    static void FlashCompActy();
    static void LampTestProgram();
    static bool MonitorProgram(byte verb, byte noun, unsigned long tick);
    static bool EnterDataProgram(byte verb, byte noun);
    static bool PleasePerformProgram(byte verb, byte noun);
    static bool EngineIgnitionProgram(byte verb, byte noun, unsigned long tick);
};
