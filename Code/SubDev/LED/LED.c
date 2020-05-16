#include "LED.h"
//------------------------------------------------------------------------------------------------//
void LEDInit(void)//³õÊ¼»¯LED
{
	if(EEPROMRead_MOVEC(EEPROM_IsStorageLED)==0xFF)
		LED1 = LEDClose,LED2 = LEDClose,LED3 = LEDClose;
	else
	{
		LED1 = EEPROMRead_MOVEC(EEPROM_LED1);
		LED2 = EEPROMRead_MOVEC(EEPROM_LED2);
		LED3 = EEPROMRead_MOVEC(EEPROM_LED3);
	}
	CloudAct.NeedReport = true;
	CloudAct.NeedReportT.LED1 = CloudAct.NeedReportT.LED2 = CloudAct.NeedReportT.LED3 = true;
	CloudAct.NeedReportT.IsStorageLED = true;
#if LOGRANK_UART1 >= 2
    printf("LOG#:LEDInit ok\r\n");
#endif
}
uchar LEDRead(uchar Idx)//¶ÁÈ¡LED×´Ì¬
{
	bool re;
	switch(Idx)
	{
		case 1:
			re = LED1 == LEDOpen;break;
		case 2:
			re = LED2 == LEDOpen;break;
		case 3:
			re = LED3 == LEDOpen;break;
		case 4:
			re = (EEPROMRead_MOVEC(EEPROM_IsStorageLED)==0x55);break;
	}
	return re;
}
bool LEDWrite(uchar Idx,uchar x)//Ð´ÈëLED×´Ì¬
{
	switch(Idx)
	{
	case 1:
		if(LED1 != x)
		{
			LED1 = x,CloudAct.NeedReport = true,CloudAct.NeedReportT.LED1 = true;
			if(EEPROMRead_MOVEC(EEPROM_IsStorageLED)==0x55)
			{
				EEPROMErase(EEPROM_IsStorageLED);
				EEPROMWrite(EEPROM_IsStorageLED,0x55);
				EEPROMWrite(EEPROM_LED2,LED2);
				EEPROMWrite(EEPROM_LED3,LED3);
				EEPROMWrite(EEPROM_LED1,x);
			}
		}
		return EXIT_SUCCESS;
	case 2:
		if(LED2 != x)
		{
			LED2 = x,CloudAct.NeedReport = true,CloudAct.NeedReportT.LED2 = true;
			if(EEPROMRead_MOVEC(EEPROM_IsStorageLED)==0x55)
			{
				EEPROMErase(EEPROM_IsStorageLED);
				EEPROMWrite(EEPROM_IsStorageLED,0x55);
				EEPROMWrite(EEPROM_LED1,LED1);
				EEPROMWrite(EEPROM_LED3,LED3);
				EEPROMWrite(EEPROM_LED2,x);
			}
		}
		return EXIT_SUCCESS;
	case 3:
		if(LED3 != x)
		{
			LED3 = x,CloudAct.NeedReport = true,CloudAct.NeedReportT.LED3 = true;
			if(EEPROMRead_MOVEC(EEPROM_IsStorageLED)==0x55)
			{
				EEPROMErase(EEPROM_IsStorageLED);
				EEPROMWrite(EEPROM_IsStorageLED,0x55);
				EEPROMWrite(EEPROM_LED1,LED1);
				EEPROMWrite(EEPROM_LED2,LED2);
				EEPROMWrite(EEPROM_LED3,x);
			}
		}
		return EXIT_SUCCESS;
	case 4:
		{
			data uchar tmp = EEPROMRead_MOVEC(EEPROM_IsStorageLED);
			if((tmp==0xFF && x == true)||(tmp==0x55&&x==false))
			{
				if(x==true)
				{
					EEPROMWrite(EEPROM_IsStorageLED,0x55);
					EEPROMWrite(EEPROM_LED1,LED1);
					EEPROMWrite(EEPROM_LED2,LED2);
					EEPROMWrite(EEPROM_LED3,LED3);
				}
				else
					EEPROMErase(EEPROM_IsStorageLED);
				CloudAct.NeedReport = true,CloudAct.NeedReportT.IsStorageLED = true;
			}
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}
//------------------------------------------------------------------------------------------------//