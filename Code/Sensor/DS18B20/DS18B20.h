#ifndef _DS18B20_H_
#define _DS18B20_H_
#include "CloudHandle.h"
#include "MCUdef.h"
#include "delay.h"
#include "string.h"
//DS18B20温度传感器,测量范围-55~125
//RAM 0~1:温度寄存器[2^-4,2^6]剩余为符号位
//    2~3:TH\TL高温低温报警阈值(有E2C),[2^0,2^6]剩余为符号位
//      4:配置寄存器(有E2C),0~4/7保留,5-R0,6-R1
//        Mode=00,9位,最大转换耗时93.75ms;Mode=01,10位,最大转换耗时187.5ms;
//        Mode=10,11位,最大转换耗时375ms;Mode=11,12位,最大转换耗时750ms;
//    5~7:保留
//      8:CRC校验
//------------------------------------------------------------------------------------------------//
//DS18B20任务指示器
typedef struct DS18B20ActST
{
    ushort TemperatureData : 12;                       //缓存最近一次温度值
    ushort TemperatureHighData1, TemperatureHighData2; //高温警告阈值,高温警告取消阈值
    ushort TemperatureLowData1, TemperatureLowData2;   //低温警告阈值,低温警告取消阈值
    uchar ResolutionMode : 2;                          //当前DS18B20分辨率
    uchar TemperatureHigh : 1;                         //高温警告状态
    uchar TemperatureLow : 1;                          //低温警告状态
} DS18B20ActST;
extern xdata DS18B20ActST DS18B20ST;
extern code ushort DS18B20ConvertTMaxTime[]; //温度转换最大等待时间(ms)
extern code float DS18B20ReTransfrom[];      //恢复Float值所需系数
//------------------------------------------------------------------------------------------------//
extern void DS18B20STInit(void);                      //初始化DS18B20内存&读取当前DS18B20分辨率
extern bool DS18B20Set(char TH, char TL, uchar Mode); //设置并保存TH/TL/Mode
extern bool DS18B20ConvertTemperature(void);          //开始转换温度值
extern bool DS18B20GetTemperature(void);              //读取温度值
//------------------------------------------------------------------------------------------------//
//DEF
#define DS18B20MaxT 125                                                              //DS18B20最高温度
#define DS18B20MinT -55                                                              //DS18B20最低温度
#define TemperatureH TemperatureWaring(30)                                           //高温报警温度
#define TemperatureL TemperatureWaring(22)                                           //低温报警温度
#define TemperatureT (1 << (DS18B20ST.ResolutionMode))                               //触发和终止的阈值差 0.5 度
#define TemperatureWaring(x) ((x - (DS18B20MinT)) << (1 + DS18B20ST.ResolutionMode)) //温度转换为无符号数

//ROM order
#define DS18B20ReadROM 0x33     //读64位ROM地址(单个DS18B20采用)
#define DS18B20MatchROM 0x55    //ROM匹配,在之后发出64位ROM编码,该地址编码的DS18B20响应
#define DS18B20SearchROM 0xF0   //确定总线上DS18B20个数和识别64位ROM地址
#define DS18B20SkipROM 0xCC     ////忽略64位ROM地址
#define DS18B20AlarmSearch 0xEC //温度越限的DS18B20响应

//RAM order
#define DS18B20ConvertT 0x44        //启动DS18B20温度转换
#define DS18B20ReadRAM 0xBE         //读9字节RAM
#define DS18B20WriteRAM 0x4E        //写入TH\TL\Mode
#define DS18B20CopyRAM 0x48         //保存RAM中TH\TL到E2PROM
#define DS18B20CopyEEPROM 0xB8      //从E2PROM中提取TH\TL到RAM
#define DS18B20ReadPowerSupply 0xB4 //读取当前供电模式,1:外接电源;0:寄生供电
//------------------------------------------------------------------------------------------------//
#endif