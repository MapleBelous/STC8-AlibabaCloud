#ifndef _GY_25_H_
#define _GY_25_H_
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
#include "CloudHandle.h"
//------------------------------------------------------------------------------------------------//
typedef struct GY_25ActST
{
	short HeadAngle,PitchAngle,RollAngle;//航向角,俯仰角,横滚角
}GY_25ActST;
extern xdata GY_25ActST GY_25ST;
//------------------------------------------------------------------------------------------------//
extern void GY_25Init(void);//初始化GY_25
extern bool GY_25SetCmd(uchar);//向GY_25发送命令
extern bool GY_25GetAzimuth(void);//读取缓冲区内的方位角数据,适用于二进制输出
//------------------------------------------------------------------------------------------------//
//常用命令
#define GY_25Query 0x51 //查询：二进制输出,格式为8字节.[0xAA 航向角16位 俯仰角16位 横滚角16位 0x55]
//------------------------------------------------------//
#define GY_25AutoBin 0x52 //自动：二进制输出,格式同查询
#define GY_25AutoASCII 0x53 //自动：ASCII输出,格式为31字节.[0x0D 0x0A #YPR=-080.62,+018.42,-141.26,]
#define GY_25Correction1 0x54 //校正：俯仰角和横滚角0度——水平时可用
#define GY_25Correction2 0x55 //校正：航向角0度
//------------------------------------------------------------------------------------------------//
#define GY_25Head 0xA5 //指令头
//------------------------------------------------------------------------------------------------//
#endif