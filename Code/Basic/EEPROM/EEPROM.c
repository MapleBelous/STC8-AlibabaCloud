#include "EEPROM.h"
//------------------------------------------------------------------------------------------------//
uchar EEPROMRead_MOVEC(ushort addr) 
{ 
	addr += EEPROM_OFFSET; //ʹ�� MOVC ��ȡ EEPROM ��Ҫ������Ӧ��ƫ��
	return *(uchar code *)(addr);
}
uchar EEPROMRead(ushort addr)
{ 
	uchar dat; 
	IAP_CONTR = EEPROMWT; //ʹ��IAP
	IAP_CMD = EEPROMCmdRead;
	EEPROMADDR(addr);
	EEPROMTRIG();
	dat = IAP_DATA; //�� IAP ����
	EEPROMOff();
	return dat; 
}
void EEPROMWrite(ushort addr, uchar dat)
{
	IAP_CONTR = EEPROMWT; //ʹ��IAP
	IAP_CMD = EEPROMCmdWrite;
	EEPROMADDR(addr);
	IAP_DATA = dat; //д IAP ����
	EEPROMTRIG();
	EEPROMOff();
}
void EEPROMErase(ushort addr)
{ 
	IAP_CONTR = EEPROMWT; //ʹ�� IAP
	IAP_CMD = EEPROMCmdErase; //���� IAP ��������
	EEPROMADDR(addr);
	EEPROMTRIG();
	EEPROMOff();
}
//------------------------------------------------------------------------------------------------//











