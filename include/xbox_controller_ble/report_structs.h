/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#pragma once

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