#ifndef _delay_H_
#define _delay_H_
//delay function H-file 可选的延时函数
#include "MCUdef.h"
//------------------------------------------------------------------------------------------------//
//模板: _DELAY_1us_				定义函数

#define _DELAY_765ms_ //DS18B20STInit from DS18B20.c
#define _DELAY_385ms_ //DS18B20STInit from DS18B20.c
#define _DELAY_200ms_ //DS18B20STInit from DS18B20.c
#define _DELAY_100ms_ //DS18B20STInit from DS18B20.c
#define _DELAY_1ms_ //LCD1602LoadWord from LCD1602.c

#define _DELAY_485us_ //DS18B20Init from DS18B20.c
#define _DELAY_130us_ //DS18B20Init from DS18B20.c
#define _DELAY_60us_  //DS18B20Init&DS18B20WriteByte&DS18B20ReadByte from DS18B20.c
#define _DELAY_45us_ //DS18B20Init from DS18B20.c
#define _DELAY_40us_ //LCD1602ReadIdx&LCD1602ReadData&LCD1602Wait from LCD1602.c
#define _DELAY_10us_  //DS18B20Init from DS18B20.c
#define _DELAY_1us_  //DS18B20WriteByte&DS18B20ReadByte from DS18B20.c

//------------------------------------------------------------------------------------------------//
//延时_宏函数,x为整数:调用函数
#define delay_us(x) Delay##x##us()
#define delay_ms(x) Delay##x##ms()
#define delay_s(x) Delay##x##s()
//------------------------------------------------------------------------------------------------//
//延时_宏函数,x为整数:函数原型
#define _DELAY_us_(x) void Delay##x##us(void)
#define _DELAY_ms_(x) void Delay##x##ms(void)
#define _DELAY_s_(x) void Delay##x##s(void)
#include "delay_YX.h"
//------------------------------------------------------------------------------------------------//
//delay源文件专用宏;生成函数
#define _DELAY_fun_(sec, mode, ind, nop, x, y, z, p) \
    void Delay##sec##mode##(void)                    \
    {                                                \
        _DELAY_##ind##_(x, y, z, p);                 \
        _DELAY_NOP##nop##_;                          \
        _DELAY_WHILE##ind##_;                        \
    }
#define _DELAY_NOP0_
#define _DELAY_NOP1_ _nop_()
#define _DELAY_NOP2_ _nop_(), _nop_()
#define _DELAY_NOP3_ _nop_(), _nop_(), _nop_()
#define _DELAY_NOP4_ _nop_(), _nop_(), _nop_(), _nop_()
#define _DELAY_NOP5_ _nop_(), _nop_(), _nop_(), _nop_(), _nop_()
#define _DELAY_NOP6_ _nop_(), _nop_(), _nop_(), _nop_(), _nop_(), _nop_()
#define _DELAY_0_(x, y, z, p)
#define _DELAY_1_(x, y, z, p) uchar i = x
#define _DELAY_2_(x, y, z, p) uchar i = x, j = y
#define _DELAY_3_(x, y, z, p) uchar i = x, j = y, k = z
#define _DELAY_4_(x, y, z, p) uchar i = x, j = y, k = z, l = p
#define _DELAY_WHILE0_
#define _DELAY_WHILE1_ while (--i)
#define _DELAY_WHILE2_ \
    do                 \
    {                  \
        while (--i)    \
            continue;  \
    } while (--j)
#define _DELAY_WHILE3_    \
    do                    \
    {                     \
        do                \
        {                 \
            while (--i)   \
                continue; \
        } while (--j);    \
    } while (--k)
#define _DELAY_WHILE4_        \
    do                        \
    {                         \
        do                    \
        {                     \
            do                \
            {                 \
                while (--i)   \
                    continue; \
            } while (--j);    \
        } while (--k);        \
    } while (--l)
//------------------------------------------------------------------------------------------------//
#endif