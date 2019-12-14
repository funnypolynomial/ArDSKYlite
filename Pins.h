#pragma once

// LCD is
// 8-bit data bus D2, D3, D4, D5, D6, D7,  D8, D9
// Control A0, A1, A2, A3, A4 
//        (RD, WR, RS, CS, RST)
// SD card D10, D11, D12, D13
// Touch   A2+D8 (x) A3+D9  (y)

// Unused A5

// SD pins are called out on "P2" connector (Not used)
//                  ~10  12
//  o  o  o  o  o  o  o  o [o]
//  o  o  o  o  o  o  o  o  o  "P2"
//                   13 ~11 GND


// Using ICSP breakout
#define PIN_RTC_SDA 12
#define PIN_RTC_SCL 11
