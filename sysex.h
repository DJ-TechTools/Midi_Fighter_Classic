// SysEx handling functions for DJTechTools Midifighter
//
//   Copyright (C) 2009 Robin Green
//
//   This file is part of the Midifighter Firmware.
//
//   The Midifighter Firmware is free software: you can redistribute it
//   and/or modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation, either version 3 of the
//   License, or (at your option) any later version.
//
//   The Midifighter Firmware is distributed in the hope that it will be
//   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License along
//   with the Midifighter Firmware.  If not, see
//   <http://www.gnu.org/licenses/>.
//
// rgreen 2009-05-22

#ifndef _SYSEX_H_INCLUDED
#define _SYSEX_H_INCLUDED

#include <stdint.h>

#define STR_COMBINE(a,b) a##b
#define SYSEX_READ(n) if (index < SYSEX_MAX_PAYLOAD) {sysEx.data[index++] = STR_COMBINE(input_event.Data,n);} else {index = 0; continue;} do {} while (0)
#define SYSEX_END() sysEx.length = index; sysex_handle (&sysEx); index = 0

// SysEx constants -----------------------------------------------

#define SYSEX_MAX_PAYLOAD 32

// SysEx types     -----------------------------------------------

// SysEx message structure
typedef struct {
    union {
        struct {
            // SysEx header
            uint8_t header;
            // Manufacturer id
            uint8_t mid;
            uint8_t mid_ex1;
            uint8_t mid_ex2;
            // Command
            uint8_t command;
            // Message payload
            uint8_t payload[SYSEX_MAX_PAYLOAD];
        };
        uint8_t data[SYSEX_MAX_PAYLOAD + 5];
    };
    uint8_t length;
} SysEx_t;

// SysEx command handler function
typedef void (*SysExFn)(SysEx_t*, void*);

// SysEx functions -----------------------------------------------

#define sysex_install(cmd,fn) sysex_install_(cmd, (SysExFn)fn)
void sysex_install_ (uint8_t cmd, SysExFn fn);
void sysex_handle (SysEx_t* sysex);

#endif // _SYSEX_H_INCLUDED
