#include "uart.h"
//------------------------------------------------------------------------------------------------//
xdata uchar uart2_buffer[uart2_buffer_size];
data ushort uart2_idx1, uart2_idx2;

bit uart1_busy, uart2_busy;
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
    while (uart1_busy)
        continue;
    uart1_busy = true;
    SBUF = dat;
    while (uart1_busy)
        continue;
}
#endif

void uart2_send8(uchar dat)
{
    while (uart2_busy)
        continue;
    uart2_busy = true;
    S2BUF = dat;
    while (uart2_busy)
        continue;
}
void uart2_sendstr8(uchar *p)
{
	while(*p)
		uart2_send8(*p++);
}
//------------------------------------------------------------------------------------------------//