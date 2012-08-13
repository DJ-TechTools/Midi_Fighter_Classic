// Checks for and performs jump to boot loader
//
// Copyright (C) 2011 Michael Mitchell
//
// MichaelMitchell 2012-01-19

#include <avr/wdt.h>
  #include <avr/io.h>
  #include <util/delay.h>

  #include <LUFA/Common/Common.h>
  #include <LUFA/Drivers/USB/USB.h>

  uint32_t Boot_Key ATTR_NO_INIT;

  #define MAGIC_BOOT_KEY            0xDC42ACCA
  #define BOOTLOADER_START_ADDRESS  0x3000//-(FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)
  
  void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
  void Bootloader_Jump_Check(void)
  {
      // If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
      if (((MCUSR & (1<<WDRF))) && (Boot_Key == MAGIC_BOOT_KEY))
      {
		  
          Boot_Key = 0;
		  // Commented out code is original jump from Deans example
		  // This break when code size exceeds 24K
          //((void (*)(void))BOOTLOADER_START_ADDRESS)(); 
		  __asm ("jmp 0x3000"); 
      }
  }

  void Jump_To_Bootloader(void)
  {
      // If USB is used, detach from the bus
      USB_ShutDown();

      // Disable all interrupts
      cli();

      // Wait two seconds for the USB detachment to register on the host
      for (uint8_t i = 0; i < 128; i++)
        _delay_ms(16);

      // Set the bootloader key to the magic value and force a reset
      Boot_Key = MAGIC_BOOT_KEY;
      wdt_enable(WDTO_250MS);
      for (;;); 
  }