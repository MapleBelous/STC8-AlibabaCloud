#ifndef _LCD1602_H_
#define _LCD1602_H_
#include "MCUdef.h"
#include "delay.h"
//------------------------------------------------------------------------------------------------//
extern void LCD1602Init(void); //初始化LCD1602
extern void LCD1602WriteLine(uchar *Str,bool isLine2);//在1/2行写入字符串
extern uchar LCD1602ReadIdx(void); //读取当前DDRAM位置(光标位置)
extern uchar LCD1602ReadData(void); //读取DDRAM/CGRAM数据
extern void LCD1602SetDDRAM(uchar idx); //0x00-0x0F-0x27,0x40-0x4F-0x67,修改DDRAM位置(光标位置)
extern void LCD1602WriteCmdOrData(uchar CmdOrData, bool isData); //写入命令/DDRAM or CGRAM数据
//------------------------------------------------------------------------------------------------//
#define LCD1602CleanScr 0x01  //清理屏幕,光标返回地址00H位置
#define LCD1602CurReturn 0x02 //光标返回地址00H位置

#define LCD1602CurLeft 0x04  //读or写光标自动左移,屏幕不移动***当前实测光标左移无效
#define LCD1602CurLeftScrMov 0x05     //读or写光标自动左移,屏幕右移***当前实测光标左移无效
#define LCD1602CurRight 0x06 //读or写光标自动右移,屏幕不移动
#define LCD1602CurRightScrMov 0x07    //读or写光标自动右移,屏幕右移

#define LCD1602CloseDisplay 0x08 //关闭显示
#define LCD1602NotCur 0x0C       //打开显示-无光标
#define LCD1602CurNotBlink 0x0E  //打开显示-有光标-光标不闪烁
#define LCD1602CurBlink 0x0F     //打开显示-有光标-光标闪烁

#define LCD1602CurMovLeft 0x10  //光标左移一位
#define LCD1602CurMovRight 0x14 //光标右移一位
#define LCD1602ScrMovLeft 0x18  //屏幕左移一位
#define LCD1602ScrMovRight 0x1C //屏幕右移一位

#define LCD1602Data8Dis1Mat5x7 0x30  //8位数据口,1行显示,5*7点阵
#define LCD1602Data8Dis1Mat5x10 0x34 //8位数据口,1行显示,5*10点阵***实测5*10点阵无效
#define LCD1602Data8Dis2Mat5x7 0x38  //8位数据口,2行显示,5*7点阵
#define LCD1602Data8Dis2Mat5x10 0x3C //8位数据口,2行显示,5*10点阵***实测5*10点阵无效
//------------------------------------------------------------------------------------------------//
#endif