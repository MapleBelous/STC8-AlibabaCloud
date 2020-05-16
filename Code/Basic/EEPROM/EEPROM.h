#ifndef _EEPROM_H_
#define _EEPROM_H_
#include "MCUdef.h"
//------------------------------------------------------------------------------------------------//
//IAP(EEPROM) OFFSET
#define EEPROM_OFFSET (0xFE00)
//------------------------------------------------------------------------------------------------//
uchar EEPROMRead_MOVEC(ushort);
uchar EEPROMRead(ushort);
void EEPROMWrite(ushort, uchar);
void EEPROMErase(ushort);
//------------------------------------------------------------------------------------------------//

#define EEPROM_IsStorageLED 0x0000
#define EEPROM_LED1 0x0001
#define EEPROM_LED2 0x0002
#define EEPROM_LED3 0x0003





//------------------------------------------------------------------------------------------------//
#define EEPROMCmdRead 0x01
#define EEPROMCmdWrite 0x02
#define EEPROMCmdErase 0x03
//------------------------------------------------------------------------------------------------//
#if _CPU_CLOCK_HZ_ >= 30000000UL
#define EEPROMWT 0x80
#elif _CPU_CLOCK_HZ_ >= 24000000UL
#define EEPROMWT 0x81 
#elif _CPU_CLOCK_HZ_ >= 20000000UL
#define EEPROMWT 0x82 
#elif _CPU_CLOCK_HZ_ >= 12000000UL
#define EEPROMWT 0x83 
#elif _CPU_CLOCK_HZ_ >= 6000000UL
#define EEPROMWT 0x84 
#elif _CPU_CLOCK_HZ_ >= 3000000UL
#define EEPROMWT 0x85 
#elif _CPU_CLOCK_HZ_ >= 2000000UL
#define EEPROMWT 0x86
#else
#define EEPROMWT 0x87
#endif
//------------------------------------------------------------------------------------------------//
#define EEPROMOff() IAP_CONTR = 0
#define EEPROMTRIG() IAP_TRIG = 0x5a,IAP_TRIG = 0xa5,_nop_()
#define EEPROMADDR(x) IAP_ADDRL = x,IAP_ADDRH = x >> 8
//------------------------------------------------------------------------------------------------//
#endif