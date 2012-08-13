
#include <util/delay.h>
#include <string.h>
#include <avr/wdt.h>

#include "config.h"
#include "sysex.h"
#include "eeprom.h"

// For the settings
#include "led.h"
#include "key.h"
#include "midi.h"
#include "eeprom.h"
#include "expansion.h"
#include "combo.h"

// SysEx command constants
#define SYSEX_COMMAND_PUSH_CONF 0x1
#define SYSEX_COMMAND_PULL_CONF 0x2
#define SYSEX_COMMAND_SYSTEM    0x3

uint8_t g_auto_update = 0;

// Command structure
typedef struct {
        // Message data                TAG
        uint8_t midiChannel;        // 0
        uint8_t midiVelocity;       // 1
        uint8_t keypressLeds;       // 2
        uint8_t fourBanksMode;      // 3
        uint8_t expansionDigital;   // 4
        uint8_t expansionAnalog;    // 5
        uint8_t autoUpdate;         // 6
        uint8_t deviceMode;           // 7
        uint8_t combos;             // 8
        uint8_t multiplexer;        // 9
} tvtable_t;
#define TV_TABLE_SIZE 10

void tv_table_decode(tvtable_t* table, uint8_t* buffer, uint8_t size)
{
    uint8_t idx = 0;
    uint8_t tag;
    while (idx < size - 1) {
        tag = buffer[idx++];
        if (tag < TV_TABLE_SIZE) {
            ((uint8_t*)table)[tag] = buffer[idx];
        }
        ++idx;
    }
}


void sysExCmdPushConfig (SysEx_t* sysex, uint8_t* buffer)
{
	// First disable watch dog as EEPROM is slow
	wdt_disable();
	
    tvtable_t config;
    tv_table_decode(&config, buffer, sysex->length-5);

    // Change settings
    g_midi_channel        = config.midiChannel - 1;
    g_midi_velocity       = config.midiVelocity;
    g_led_keypress_enable = config.keypressLeds;
    g_key_fourbanks_mode  = config.fourBanksMode;
    g_exp_digital_read    = config.expansionDigital;
    g_exp_analog_read     = config.expansionAnalog;
    g_auto_update         = config.autoUpdate;
    g_device_mode         = config.deviceMode;
    g_combos_enable       = config.combos;
    g_multiplexer_enable  = config.multiplexer;

    // Save to EEPROM
    eeprom_save_edits();

    // Flash LEDs to signal new configuration
    uint16_t state = 0x000f;
    for (uint8_t i=4; i; --i) {
            led_set_state(state);
            _delay_ms(75);
            led_set_state(0x0000);
            _delay_ms(75);
            state <<= 4;
    }
	
	// Enable watchdog again
	wdt_enable(WDTO_120MS);
}

void send_config_data (void)
{
    uint8_t payload[] = {0xf0, 0x00, MANUFACTURER_ID >> 8, MANUFACTURER_ID & 0x7f,
                                SYSEX_COMMAND_PULL_CONF,
                                0x1, // 0x0 = request, 0x1 = response
                                0x0, g_midi_channel + 1,    // midi channel
                                0x1, g_midi_velocity,       // midi note velocity
                                0x2, g_led_keypress_enable, // led light on keypress
                                0x3, g_key_fourbanks_mode,  // four banks mode disabled, enabled internal or enabled external
                                0x4, g_exp_digital_read,    // should digital inputs be read
                                0x5, g_exp_analog_read,     // should analog inputs be read
                                0x6, g_auto_update,         // should the configuration tool keep this firmware up to date
                                0x7, g_device_mode,        // rotation
                                0x8, g_combos_enable,       // combos enabled or disabled
                                0x9, g_multiplexer_enable,  // analog input multiplexer on A1, using D1, D2, and D3 to select multiplexed input
                                0xf7};
    midi_stream_sysex(/* number of bytes in payload 26 */ sizeof(payload), payload);
}

void sysExCmdPullConfig (SysEx_t* sysex, uint8_t* command)
{
    // Change settings
    if (*command == 0x0) { // Received request
        send_config_data();
    }
}

void enter_bootloader_mode (void);
void enter_menu_mode (void);
void factory_reset (void);

void sysExCmdSystem (SysEx_t* sysex, uint8_t* command)
{
    if (*command == 0)
    {
        // Menu mode
		wdt_disable();
        enter_menu_mode();

    } else if (*command == 1)
    {
        // Bootloader mode
		wdt_disable();
        //enter_bootloader_mode();
		led_set_state(0xA5A5);
		Jump_To_Bootloader();
		
    } else if (*command == 2)
    {
        // Factory reset EEPROM
		wdt_disable();
        factory_reset();
		wdt_enable(WDTO_120MS);
    }
}


void config_setup (void)
{
    // Install SysEx command handlers
    sysex_install(SYSEX_COMMAND_PUSH_CONF, sysExCmdPushConfig);
    sysex_install(SYSEX_COMMAND_PULL_CONF, sysExCmdPullConfig);
    sysex_install(SYSEX_COMMAND_SYSTEM,    sysExCmdSystem);
}
