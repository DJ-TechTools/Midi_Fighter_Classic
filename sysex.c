
#include "sysex.h"
#include "constants.h"

#include "midi.h"

SysExFn sysExCommandMap[8] = {0,};

void sysex_handle (SysEx_t* sysex)
{
    if (sysex->mid == 0x0 &&
        sysex->mid_ex1 == (MANUFACTURER_ID >> 8) &&
        sysex->mid_ex2 == (MANUFACTURER_ID & 0x7f)) {
        // This message is meant for us.

        if (sysExCommandMap[sysex->command]) {
            sysExCommandMap[sysex->command](sysex, sysex->payload);
        }
    }
    else if (sysex->mid == 0x7e) { // Universal Non Real Time
        if (sysex->mid_ex2 == 0x06 && sysex->command == 0x01) { // Device identify request
			
            uint8_t payload[] = {0xf0, 0x7e, 0x7f, 0x06, 0x02,
                                       0x00, MANUFACTURER_ID >> 8, MANUFACTURER_ID & 0x7f,
                                       DEVICE_FAMILY,
                                       DEVICE_MODEL,
                                       (uint8_t)(((uint16_t)DEVICE_VERSION_YEAR) >> 8),
                                       DEVICE_VERSION_YEAR & 0x7f,
                                       DEVICE_VERSION_MONTH,
                                       DEVICE_VERSION_DAY,
                                       0xf7};
            midi_stream_sysex(/* 17 bytes */ sizeof(payload), payload);
        }
    }
}


void sysex_install_ (uint8_t cmd, SysExFn fn)
{
    sysExCommandMap[cmd] = fn;
}
