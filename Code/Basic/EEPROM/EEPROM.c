#include "EEPROM.h"
//------------------------------------------------------------------------------------------------//
uchar EEPROMRead_MOVEC(ushort addr) 
{ 
	addr += EEPROM_OFFSET; //使用 MOVC 读取 EEPROM 需要加上相应的偏移
	return *(uchar code *)(addr);
}
uchar EEPROMRead(ushort addr)
{ 
	uchar dat; 
	IAP_CONTR = EEPROMWT; //使能IAP
	IAP_CMD = EEPROMCmdRead;
	EEPROMADDR(addr);
	EEPROMTRIG();
	dat = IAP_DATA; //读 IAP 数据
	EEPROMOff();
	return dat; 
}
void EEPROMWrite(ushort addr, uchar dat)
{
	IAP_CONTR = EEPROMWT; //使能IAP
	IAP_CMD = EEPROMCmdWrite;
	EEPROMADDR(addr);
	IAP_DATA = dat; //写 IAP 数据
	EEPROMTRIG();
	EEPROMOff();
}
void EEPROMErase(ushort addr)
{ 
	IAP_CONTR = EEPROMWT; //使能 IAP
	IAP_CMD = EEPROMCmdErase; //设置 IAP 擦除命令
	EEPROMADDR(addr);
	EEPROMTRIG();
	EEPROMOff();
}
//------------------------------------------------------------------------------------------------//











