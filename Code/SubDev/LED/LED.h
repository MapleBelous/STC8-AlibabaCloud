#ifndef _LED_H_
#define _LED_H_
#include "MCUdef.h"
#include "CloudHandle.h"
#include <string.h>
#include "EEPROM.h"
//------------------------------------------------------------------------------------------------//
void LEDInit(void);//��ʼ��LED
uchar LEDRead(uchar);//��ȡLED״̬
bool LEDWrite(uchar,uchar);//д��LED״̬
//------------------------------------------------------------------------------------------------//
#define LEDClose 1
#define LEDOpen 0
//------------------------------------------------------------------------------------------------//
#endif