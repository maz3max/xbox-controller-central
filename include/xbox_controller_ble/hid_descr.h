/*
 * Copyright (c) 2023 Maximilian Deubel
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#pragma once

static const uint8_t hid_report_desc[] = {
	0x05, 0x01,  // Usage Page (Generic Desktop Ctrls)
	0x09, 0x05,  // Usage (Game Pad)
	0xA1, 0x01,  // Collection (Application)
	0x85, 0x01,  //   Report ID (1)
// Buttons
	0x05, 0x09,  //   Usage Page (Button)
	0x19, 0x01,  //   Usage Minimum (Button 1)
	0x29, 0x10,  //   Usage Maximum (Button 16)
	0x15, 0x00,  //   Logical Minimum (0)
	0x25, 0x01,  //   Logical Maximum (1)
	0x75, 0x01,  //   Report Size (1)
	0x95, 0x10,  //   Report Count (16)
	0x81, 0x02,  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// Analog things
	0x05, 0x01,  //   Usage Page (Generic Desktop Ctrls)
	0x15, 0x00,  //   Logical Minimum (0)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x09, 0x30,  //   Usage (X)
	0x09, 0x31,  //   Usage (Y)
	0x09, 0x32,  //   Usage (Z)
	0x09, 0x33,  //   Usage (Rx)
	0x09, 0x34,  //   Usage (Ry)
	0x09, 0x35,  //   Usage (Rz)
	0x75, 0x08,  //   Report Size (8)
	0x95, 0x06,  //   Report Count (6)
	0x81, 0x02,  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// HAT switch
	0x09, 0x39,        //   Usage (Hat switch)
	0x15, 0x01,        //   Logical Minimum (1)
	0x25, 0x08,        //   Logical Maximum (8)
	0x35, 0x00,        //   Physical Minimum (0)
	0x46, 0x3B, 0x01,  //   Physical Maximum (315)
	0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
	0x75, 0x04,        //   Report Size (4)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
	0x75, 0x04,        //   Report Size (4)
	0x95, 0x01,        //   Report Count (1)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x00,        //   Logical Maximum (0)
	0x35, 0x00,        //   Physical Minimum (0)
	0x45, 0x00,        //   Physical Maximum (0)
	0x65, 0x00,        //   Unit (None)
	0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// Rumble Controls
	0x05, 0x0F,        //   Usage Page (PID Page)
	0x09, 0x21,        //   Usage (0x21)
	0x85, 0x03,        //   Report ID (3)
	0xA1, 0x02,        //   Collection (Logical)
	0x09, 0x97,        //     Usage (0x97)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x75, 0x04,        //     Report Size (4)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x00,        //     Logical Maximum (0)
	0x75, 0x04,        //     Report Size (4)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x03,        //     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x70,        //     Usage (0x70)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x64,        //     Logical Maximum (100)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x04,        //     Report Count (4)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x50,        //     Usage (0x50)
	0x66, 0x01, 0x10,  //     Unit (System: SI Linear, Time: Seconds)
	0x55, 0x0E,        //     Unit Exponent (-2)
	0x15, 0x00,        //     Logical Minimum (0)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0xA7,        //     Usage (0xA7)
	0x15, 0x00,        //     Logical Minimum (0)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x65, 0x00,        //     Unit (None)
	0x55, 0x00,        //     Unit Exponent (0)
	0x09, 0x7C,        //     Usage (0x7C)
	0x15, 0x00,        //     Logical Minimum (0)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              //   End Collection
	0xC0,        // End Collection
};