#ifndef _MCUdef_H_
#define _MCUdef_H_
//------------------------------------------------------------------------------------------------//
//Basic System Head File
#include <STC8.h>
#include <intrins.h>

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
//------------------------------------------------------------------------------------------------//
//Basic User Head File
#include "pindef.h"
//------------------------------------------------------------------------------------------------//

//Clock (hz)
#define _CPU_CLOCK_HZ_ (22118400UL)

//------------------------------------------------------------------------------------------------//
//---------------中断ISR------------------//

#define _ISR1_  //定时器0
#define _ISR4_  //串口1
#define _ISR15_ //串口4

//------------------------------------------------------------------------------------------------//

//串口1打印日志级别,0:无日志,1:ERR,2:ERR+LOG#,3:ERR+LOG#+LOG
#define LOGRANK_UART1 3

//------------------------------------------------------------------------------------------------//

//代替stdbool.h
#ifndef __bool_true_false_are_defined
#define true 1
#define false 0
typedef bit bool; //位寻址寄存器专用,不支持C99,严禁用于构建结构体
#endif

//Type Def,代替stdint.h
typedef unsigned char uchar;   //uchar->u8
typedef unsigned short ushort; //ushort->u16
typedef unsigned int uint;     //uint->u16[u32]
typedef unsigned long ulong;   //uchar->u32
//备用
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;

//BOOL EXIT 
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

//Necessary Define Function
#define ioread_b(x) ((x) = true, _nop_, _nop_, (x)) //准双向模式,外置传感器读1引脚
#define ioread_p(x) ((x) = 0xFF, _nop_, _nop_, (x)) //准双向模式,外置传感器读8引脚
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#ifdef __STC8F_H_
#define MCURST()                                \
    do                                          \
    {                                           \
        IAP_CONTR &= ~SWBS, IAP_CONTR |= SWRST; \
    } while (false) //采用STC8内置复位寄存器复位
#else
#define MCURST_OLD() (*((void(code *)(void))0x0000))() //通用函数指针复位
#endif
//------------------------------------------------------------------------------------------------//
#endif