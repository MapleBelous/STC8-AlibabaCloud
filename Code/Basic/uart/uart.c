#include "uart.h"
//------------------------------------------------------------------------------------------------//
xdata uchar uart4_buffer[uart4_buffer_size];
data ushort uart4_idx1, uart4_idx2;

bit uart1_busy, uart4_busy;
//------------------------------------------------------------------------------------------------//
#if LOGRANK_UART1 >= 1
//-----------��дputchar����-----------//
//----����printf�����򴮿�1��������----//
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

void uart4_send8(uchar dat)
{
    while (uart4_busy)
        continue;
    uart4_busy = true;
    S4BUF = dat;
    while (uart4_busy)
        continue;
}
void uart4_sendstr8(uchar *p,uchar n)
{
	if(n)
		while(n--)
			uart4_send8(*p++);
	else
		while(*p)
			uart4_send8(*p++);
}
//------------------------------------------------------------------------------------------------//