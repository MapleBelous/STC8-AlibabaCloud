#include "GY_25.h"
//------------------------------------------------------------------------------------------------//
xdata GY_25ActST GY_25ST;
//------------------------------------------------------------------------------------------------//
void GY_25Init(void)//��ʼ��GY_25
{
	memset(&GY_25ST,0xFF,sizeof(GY_25ActST));
	S4CON &= ~S4REN;//��ֹ���ڶ���
	IE2 |= ES4;//���������ж�
	GY_25SetCmd(GY_25Correction2);//У�������
	GY_25SetCmd(GY_25Correction1);//У��������&�����
	GY_25SetCmd(GY_25Query);
	S4CON |= S4REN;//�������ڶ���
	while(GY_25GetAzimuth()!=EXIT_SUCCESS)//��ȡ��λ������
		continue;
#if LOGRANK_UART1 >= 2
    printf("LOG#:GY_25Init ok\r\n");
#endif
}
bool GY_25SetCmd(uchar Cmd)//��GY_25��������
{
	uart4_send8(GY_25Head);
	uart4_send8(Cmd);
	return EXIT_SUCCESS;
}
bool GY_25GetAzimuth(void)//��ȡ�������ڵķ�λ������,�����ڶ��������
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
				CloudAct.NeedReport = true,CloudAct.NeedReportT.GY_25 = true;//�����Ҫ�ϱ���λ�ǲ���
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}