#include "GY_25.h"
//------------------------------------------------------------------------------------------------//
xdata GY_25ActST GY_25ST;
//------------------------------------------------------------------------------------------------//
void GY_25Init(void)//初始化GY_25
{
	memset(&GY_25ST,0xFF,sizeof(GY_25ActST));
	S4CON &= ~S4REN;//禁止串口读入
	IE2 |= ES4;//开启串口中断
	GY_25SetCmd(GY_25Correction2);//校正航向角
	GY_25SetCmd(GY_25Correction1);//校正俯仰角&横滚角
	GY_25SetCmd(GY_25Query);
	S4CON |= S4REN;//开启串口读入
	while(GY_25GetAzimuth()!=EXIT_SUCCESS)//读取方位角数据
		continue;
#if LOGRANK_UART1 >= 2
    printf("LOG#:GY_25Init ok\r\n");
#endif
}
bool GY_25SetCmd(uchar Cmd)//向GY_25发送命令
{
	uart4_send8(GY_25Head);
	uart4_send8(Cmd);
	return EXIT_SUCCESS;
}
bool GY_25GetAzimuth(void)//读取缓冲区内的方位角数据,适用于二进制输出
{
	static pdata uchar State = 0;
	static xdata ushort HeadAngle,PitchAngle,RollAngle;
	data uchar idx1 = uart4Idx1, idx2 = uart4Idx2;
	while(idx1!=idx2)
	{
		switch(State)
		{
		case 0:
			if(uart4Buffer[idx1]==0xAA)
				++State;
			break;
		case 1:case 2:
			HeadAngle<<=8;
			HeadAngle|=uart4Buffer[idx1];
			++State;
			break;
		case 3:case 4:
			PitchAngle<<=8;
			PitchAngle|=uart4Buffer[idx1];
			++State;
			break;
		case 5:case 6:
			RollAngle<<=8;
			RollAngle|=uart4Buffer[idx1];
			++State;
			break;
		case 7:
			if(uart4Buffer[idx1]==0x55)
				++State;
			break;
		}
		if (idx1 + 1 == uart4_buffer_size)
            idx1 = 0;
        else
            ++idx1;
        uart4Idx1 = idx1;
		if(State==8)
		{
			bool isp=false;
			State = 0;
			if(HeadAngle!=GY_25ST.HeadAngle)
				GY_25ST.HeadAngle=HeadAngle,isp=true;
			if(PitchAngle!=GY_25ST.PitchAngle)
				GY_25ST.PitchAngle=PitchAngle,isp=true;
			if(RollAngle!=GY_25ST.RollAngle)
				GY_25ST.RollAngle=RollAngle,isp=true;
			if(isp)
				CloudAct.NeedReport = true,CloudAct.NeedReportT.GY_25 = true;//标记需要上报方位角参数
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}