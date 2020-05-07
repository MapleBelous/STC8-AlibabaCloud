#include "uart.h"
//------------------------------------------------------------------------------------------------//
bool uart1Busy, uart2Busy, uart4Busy;

xdata uchar uart2Buffer[uart2_buffer_size];
data ushort uart2Idx1, uart2Idx2;
xdata uchar uart4Buffer[uart4_buffer_size];
data uchar uart4Idx1,uart4Idx2;
//------------------------------------------------------------------------------------------------//
#if LOGRANK_UART1 >= 1
//-----------重写putchar函数-----------//
//----用于printf函数向串口1发送数据----//
char putchar(char ch)
{
    uart1_send8(ch);
    return 1;
}
void uart1_send8(uchar dat)
{
    while (uart1Busy)
        continue;
    uart1Busy = true;
    SBUF = dat;
    while (uart1Busy)
        continue;
}
#endif

void uart2_send8(uchar dat)
{
    while (uart2Busy)
        continue;
    uart2Busy = true;
    S2BUF = dat;
    while (uart2Busy)
        continue;
}
void uart2_sendstr8(uchar *p)
{
	while(*p)
		uart2_send8(*p++);
}

void uart4_send8(uchar dat)
{
    while (uart4Busy)
        continue;
    uart4Busy = true;
    S4BUF = dat;
    while (uart4Busy)
        continue;
}
//------------------------------------------------------------------------------------------------//