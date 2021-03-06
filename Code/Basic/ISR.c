#include "CloudHandle.h"
#include "MCUdef.h"
#include "delay.h"
#include "uart.h"
//------------------------------------------------------------------------------------------------//
#ifdef _ISR0_
void INT0_ISR(void) interrupt 0
{
}
#endif
#ifdef _ISR1_
void TM0_ISR(void) interrupt 1
{
     ++CloudAct.SysTime;//更新系统时间 1ms
}
#endif
#ifdef _ISR2_
void INT1_ISR(void) interrupt 2
{
}
#endif
#ifdef _ISR3_
void TM1_ISR(void) interrupt 3
{
}
#endif
#ifdef _ISR4_
void UART1_ISR(void) interrupt 4
{
    if (TI)
        TI = 0, uart1Busy = false;
}
#endif
#ifdef _ISR5_
void ADC_ISR(void) interrupt 5
{
}
#endif
#ifdef _ISR6_
void LVD_ISR(void) interrupt 6
{
}
#endif
#ifdef _ISR7_
void PCA_ISR(void) interrupt 7
{
}
#endif
#ifdef _ISR8_
void UART2_ISR(void) interrupt 8
{
	if (S2CON & S2RI)
    {
		bool isFull = false;
        S2CON &= ~S2RI;
		if(uart2Idx2 + 1 == uart2_buffer_size)
		{
			if(uart2Idx1 == 0)
				isFull = true;
		}
		else if(uart2Idx1 == uart2Idx2 + 1)
			isFull = true;
		else
			LED_GREEN = 1;
		if(isFull == true)//串口缓冲区已满,放弃当前数据
        {                                                       //不能打印日志,可能导致堆栈错误
            LED_GREEN = 0;                                      //亮绿灯表示串口2缓冲区已满
            return;
        }
        uart2Buffer[uart2Idx2] = S2BUF;
        if (uart2Idx2 + 1 == uart2_buffer_size)
            uart2Idx2 = 0;
        else
            ++uart2Idx2;
    }
    else
        S2CON &= ~S4TI, uart2Busy = false;
}
#endif
#ifdef _ISR9_
void SPI_ISR(void) interrupt 9
{
}
#endif
#ifdef _ISR10_
void INT2_ISR(void) interrupt 10
{
}
#endif
#ifdef _ISR11_
void INT3_ISR(void) interrupt 11
{
}
#endif
#ifdef _ISR12_
void TM2_ISR(void) interrupt 12
{
}
#endif
#ifdef _ISR13_
void INT4_ISR(void) interrupt 16
{
}
#endif
#ifdef _ISR14_
void UART3_ISR(void) interrupt 17
{
}
#endif
#ifdef _ISR15_
void UART4_ISR(void) interrupt 18
{
	if(S4CON & S4RI)
	{
		bool isFull = false;
        S4CON &= ~S4RI;
		if(uart4Idx2 + 1 == uart4_buffer_size)
		{
			if(uart4Idx1 == 0)
				isFull = true;
		}
		else if(uart4Idx1 == uart4Idx2 + 1)
			isFull = true;
		else
			LED_BLUE = 1;
		if(isFull == true)//串口缓冲区已满,放弃当前数据
        {                                                       //不能打印日志,可能导致堆栈错误
            LED_BLUE = 0;                                      //亮蓝灯表示串口4缓冲区已满
            return;
        }
        uart4Buffer[uart4Idx2] = S4BUF;
        if (uart4Idx2 + 1 == uart4_buffer_size)
            uart4Idx2 = 0;
        else
            ++uart4Idx2;
	}
	else
		S4CON &= ~S4TI, uart4Busy = false;
}
#endif
#ifdef _ISR16_
void TM3_ISR(void) interrupt 19
{
}
#endif
#ifdef _ISR17_
void TM4_ISR(void) interrupt 20
{
}
#endif
#ifdef _ISR18_
void CMP_ISR(void) interrupt 21
{
}
#endif
#ifdef _ISR19_
void PWM_ISR(void) interrupt 22
{
}
#endif
#ifdef _ISR20_
void PWMFD_ISR(void) interrupt 23
{
}
#endif
#ifdef _ISR21_
void I2C_ISR(void) interrupt 24
{
}
//------------------------------------------------------------------------------------------------//
#endif
