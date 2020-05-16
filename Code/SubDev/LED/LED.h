#ifndef _LED_H_
#define _LED_H_
#include "MCUdef.h"
#include "CloudHandle.h"
#include <string.h>
#include "EEPROM.h"
//------------------------------------------------------------------------------------------------//
void LEDInit(void);//初始化LED
uchar LEDRead(uchar);//读取LED状态
bool LEDWrite(uchar,uchar);//写入LED状态
//------------------------------------------------------------------------------------------------//
#define LEDClose 1
#define LEDOpen 0
//------------------------------------------------------------------------------------------------//
#endif