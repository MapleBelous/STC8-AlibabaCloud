#ifndef _uart_H_
#define _uart_H_
#include "MCUdef.h"
#include "pindef.h"
//------------------------------------------------------------------------------------------------//
//串口接收缓冲区大小
#define uart2_buffer_size 600
#define uart4_buffer_size 32
//------------------------------------------------------------------------------------------------//
extern bool uart1Busy,uart2Busy,uart4Busy;//串口发送正忙标志

extern xdata uchar uart2Buffer[];//串口2接收缓冲区
extern data ushort uart2Idx1,uart2Idx2;//串口缓冲区索引
extern xdata uchar uart4Buffer[];//串口4接收缓冲区
extern data uchar uart4Idx1,uart4Idx2;//串口缓冲区索引
//------------------------------------------------------------------------------------------------//
void uart1_send8(uchar dat);
void uart2_send8(uchar dat);
void uart2_sendstr8(uchar *p);
void uart4_send8(uchar dat);
//------------------------------------------------------------------------------------------------//
#endif