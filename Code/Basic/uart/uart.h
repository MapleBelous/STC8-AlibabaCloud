#ifndef _uart_H_
#define _uart_H_
#include "MCUdef.h"
#include "pindef.h"
//------------------------------------------------------------------------------------------------//
//串口接收缓冲区大小
#define uart2_buffer_size 600
//------------------------------------------------------------------------------------------------//
extern xdata uchar uart2_buffer[];//串口接收缓冲区
extern data ushort uart2_idx1,uart2_idx2;//串口缓冲区索引
extern bit uart1_busy,uart2_busy;//串口发送正忙标志
//------------------------------------------------------------------------------------------------//
void uart1_send8(uchar dat);
void uart2_send8(uchar dat);
void uart2_sendstr8(uchar *p);
//------------------------------------------------------------------------------------------------//
#endif