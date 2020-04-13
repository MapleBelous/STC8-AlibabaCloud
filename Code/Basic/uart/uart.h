#ifndef _uart_H_
#define _uart_H_
#include "MCUdef.h"
#include "pindef.h"
//------------------------------------------------------------------------------------------------//
//串口接收缓冲区大小
#define uart4_buffer_size 1000
//------------------------------------------------------------------------------------------------//
extern xdata uchar uart4_buffer[];//串口接收缓冲区
extern pdata ushort uart4_idx1,uart4_idx2;//串口缓冲区索引
extern bit uart1_busy,uart4_busy;//串口发送正忙标志
//------------------------------------------------------------------------------------------------//
void uart1_send8(uchar dat);
void uart4_send8(uchar dat);
void uart4_sendstr8(uchar *p,uchar n);
//------------------------------------------------------------------------------------------------//
#endif