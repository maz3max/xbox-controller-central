/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#pragma once
#pragma pack(push,1)

struct xbox_controller_report
{
        uint16_t lstick_x;
        uint16_t lstick_y;
        uint16_t rstick_x;
        uint16_t rstick_y;
        uint16_t lt;
        uint16_t rt;

        union {
                uint8_t raw;
                enum {
                        NEUTRAL = 0,
                        UP = 1,
                        UP_RIGHT,
                        RIGHT,
                        DOWN_RIGHT,
                        DOWN,
                        DOWN_LEFT,
                        LEFT,
                        UP_LEFT
                } val;
        } dpad;

        unsigned a:1;
        unsigned b:1;
        unsigned padding_0:1;
        unsigned x:1;
        unsigned y:1;
        unsigned padding_1:1;
        unsigned lb :1;
        unsigned rb :1;

        unsigned padding_2:1;
        unsigned padding_3:1;
        unsigned select:1;
        unsigned start:1;
        unsigned system:1;
        unsigned lstick_btn:1;
        unsigned rstick_btn:1;
        unsigned padding_4:1;

        uint8_t padding_5;
};

struct xbox_controller_report_output
{
//  uint8_t  reportId;                                 // Report ID = 0x03 (3)
                                                     // Collection: CA:GamePad CL:SetEffectReport
  uint8_t  DcEnableActuators : 4; // Usage 0x000F0097: DC Enable Actuators, Value = 0 to 1
  uint8_t  : 4;                                      // Pad
  uint8_t  Magnitude[4];   // Usage 0x000F0070: Magnitude, Value = 0 to 100
  uint8_t  Duration;       // Usage 0x000F0050: Duration, Value = 0 to 255, Physical = Value in 10⁻² s units
  uint8_t  StartDelay;     // Usage 0x000F00A7: Start Delay, Value = 0 to 255, Physical = Value in 10⁻² s units
  uint8_t  LoopCount;      // Usage 0x000F007C: Loop Count, Value = 0 to 255
};

int request_rumble(struct xbox_controller_report_output *report);

//--------------------------------------------------------------------------------
// Button Page inputReport 01 (Device --> Host)
//--------------------------------------------------------------------------------

typedef struct
{
  uint8_t  reportId;                                 // Report ID = 0x01 (1)
                                                     // Collection: CA:GamePad
  uint8_t  BTN_GamePadButton1 : 1;                   // Usage 0x00090001: Button 1 Primary/trigger, Value = 0 to 1
  uint8_t  BTN_GamePadButton2 : 1;                   // Usage 0x00090002: Button 2 Secondary, Value = 0 to 1
  uint8_t  BTN_GamePadButton3 : 1;                   // Usage 0x00090003: Button 3 Tertiary, Value = 0 to 1
  uint8_t  BTN_GamePadButton4 : 1;                   // Usage 0x00090004: Button 4, Value = 0 to 1
  uint8_t  BTN_GamePadButton5 : 1;                   // Usage 0x00090005: Button 5, Value = 0 to 1
  uint8_t  BTN_GamePadButton6 : 1;                   // Usage 0x00090006: Button 6, Value = 0 to 1
  uint8_t  BTN_GamePadButton7 : 1;                   // Usage 0x00090007: Button 7, Value = 0 to 1
  uint8_t  BTN_GamePadButton8 : 1;                   // Usage 0x00090008: Button 8, Value = 0 to 1
  uint8_t  BTN_GamePadButton9 : 1;                   // Usage 0x00090009: Button 9, Value = 0 to 1
  uint8_t  BTN_GamePadButton10 : 1;                  // Usage 0x0009000A: Button 10, Value = 0 to 1
  uint8_t  BTN_GamePadButton11 : 1;                  // Usage 0x0009000B: Button 11, Value = 0 to 1
  uint8_t  BTN_GamePadButton12 : 1;                  // Usage 0x0009000C: Button 12, Value = 0 to 1
  uint8_t  BTN_GamePadButton13 : 1;                  // Usage 0x0009000D: Button 13, Value = 0 to 1
  uint8_t  BTN_GamePadButton14 : 1;                  // Usage 0x0009000E: Button 14, Value = 0 to 1
  uint8_t  BTN_GamePadButton15 : 1;                  // Usage 0x0009000F: Button 15, Value = 0 to 1
  uint8_t  BTN_GamePadButton16 : 1;                  // Usage 0x00090010: Button 16, Value = 0 to 1
  uint8_t  GD_GamePadX;                              // Usage 0x00010030: X, Value = 0 to 255
  uint8_t  GD_GamePadY;                              // Usage 0x00010031: Y, Value = 0 to 255
  uint8_t  GD_GamePadZ;                              // Usage 0x00010032: Z, Value = 0 to 255
  uint8_t  GD_GamePadRx;                             // Usage 0x00010033: Rx, Value = 0 to 255
  uint8_t  GD_GamePadRy;                             // Usage 0x00010034: Ry, Value = 0 to 255
  uint8_t  GD_GamePadRz;                             // Usage 0x00010035: Rz, Value = 0 to 255
  uint8_t  GD_GamePadHatSwitch : 4;                  // Usage 0x00010039: Hat switch, Value = 1 to 8, Physical = (Value - 1) x 45 in degrees
  uint8_t  : 4;                                      // Pad
} inputReport01_t;


//--------------------------------------------------------------------------------
// Physical Interface Device Page outputReport 03 (Device <-- Host)
//--------------------------------------------------------------------------------

typedef struct
{
  uint8_t  reportId;                                 // Report ID = 0x03 (3)
                                                     // Collection: CA:GamePad CL:SetEffectReport
  uint8_t  PID_GamePadSetEffectReportDcEnableActuators : 4; // Usage 0x000F0097: DC Enable Actuators, Value = 0 to 1
  uint8_t  : 4;                                      // Pad
  uint8_t  PID_GamePadSetEffectReportMagnitude[4];   // Usage 0x000F0070: Magnitude, Value = 0 to 100
  uint8_t  PID_GamePadSetEffectReportDuration;       // Usage 0x000F0050: Duration, Value = 0 to 255, Physical = Value in 10⁻² s units
  uint8_t  PID_GamePadSetEffectReportStartDelay;     // Usage 0x000F00A7: Start Delay, Value = 0 to 255, Physical = Value in 10⁻² s units
  uint8_t  PID_GamePadSetEffectReportLoopCount;      // Usage 0x000F007C: Loop Count, Value = 0 to 255
} outputReport03_t;

#pragma pack(pop)