#pragma once

// Configures the sizes, positions etc of graphic elements, etc
// Also some hehaviour
class Config
{
  public:
    // flag is set if non-zero
    // Output flags is octal 0wxyz w=Northern/Southern hemisphere,x=24h/24h y=leading 0’s, z=YMD/DMY/MDY
    static const word OutputDefaults  = 001110;
    static const word MaskNorthern    = 007000;  // else Southern
    static const word Mask24Hour      = 000700;  // else 12h
    static const word MaskLeading0    = 000070;  // else spaces, decimal only
    static const word MaskDateFmt     = 000007;  // 0:YMD, 1:DMY, 2:MDY
    // Input flags is octal 0000z, z=flash keyface
    static const word InputDefaults   = 000001;
    static const word MaskFlash       = 000007;  // flash btns
    
    static void Load();
    static void Save();
    static void Reset();
    static int  Write(word* flags);
    
    static word _OutputFlags;
    static word _InputFlags;
    static byte _AutoVerb;
    static byte _AutoNoun;
    static unsigned long _RefNewMoonSeconds;
    
    // whole LCD dims
    static const int DisplayWidth = 240;
    static const int DisplayHeight = 320;
    
    // keyboard
    static const int ButtonsAcross = 7;
    static const int ButtonWidth = 31;
    static const int ButtonHeight = 31;
    static const int ButtonGapX = 3;
    static const int ButtonGapY = 4;
    static const int ButtonOffsetBottom = 6;
    static const int ButtonOffsetLeft = (DisplayWidth - ButtonsAcross*ButtonWidth - (ButtonsAcross - 1)*ButtonGapX)/2;

    // the two panels
    static const int PanelRectWidth = 112;
    static const int PanelRectHeight = 191;
    static const int PanelRectOffsetX = 5;
    static const int PanelRectOffsetY = 10;
    static const int PanelRectRadius = 0;
    static const int PanelRectThickness = 1;
    static const int PanelInnerOffset = 3;

    // lights panel
    static const int LightsRows = 7; 
    static const int LightsCols = 2; 
    static const int LightsSideOffset = 2;
    static const int LightsTopOffset = 1;
    static const int LightsColGap = 2;
    static const int LightsRowGap = 2;
    static const int LightsSpaceWidth  = PanelRectWidth - 2*(PanelInnerOffset + PanelRectThickness + LightsSideOffset);
    static const int LightsSpaceHeight = PanelRectHeight - 2*(PanelInnerOffset + PanelRectThickness + LightsTopOffset);
    static const int LightsCellWidth  = (LightsSpaceWidth - LightsColGap)/LightsCols;
    static const int LightsCellHeight = (LightsSpaceHeight - (LightsRows - 1)*LightsRowGap)/LightsRows;

    // numeric panel
    static const int NumericSideOffset = 2;
    static const int NumericTopOffset = 1;
    static const int NumericSpaceWidth  = PanelRectWidth - 2*(PanelInnerOffset + PanelRectThickness + NumericSideOffset);
    static const int NumericSpaceHeight = PanelRectHeight - 2*(PanelInnerOffset + PanelRectThickness + NumericTopOffset);
    static const int NumericDigitCellWidth = 15 + 1;
    static const int NumericDigitCellHeight = 25 + 4;
    static const int NumericUnderlineOffset = 3;  // from cell
    static const int NumericUnderlineThickness = 2;
    static const int NumericLabelCellHeight = 10;
    static const int NumericLabelCellWidth = 2*NumericDigitCellWidth;
    
    
    static const int LabelMaxLen = 20;
    static const word KeyColour        = 0xFFFF;
    static const word ForegroundColour = 0x0000;
    static const word BackgroundColour = 0x4208;//0x8410;
    static const word NumericColour    = 0x07E0;
    static const word IndicatorOffColour  = 0x8410;//0x4208;
    static const word IndicatorYellow     = 0xFFE0;
    static const word IndicatorWhite      = 0xFFFF;
    static const word SegmentOffColour  = BackgroundColour;
    
};
