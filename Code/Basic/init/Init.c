#include "Init.h"
//------------------------------------------------------------------------------------------------//
static void Init_rst(void);
static void Init_io(void);
static void Init_timer0(void);
static void Init_uart1(void);
static void Init_uart2(void);
static void Init_wkt(float,bool);
static void Init_interrupt(void);
//------------------------------------------------------------------------------------------------//

void Init(void)
{
    //�ڲ�
    Init_rst();       //��ѹ��λ&���Ź���ʼ��
    Init_io();        //IO�ڳ�ʼ��
    Init_timer0();    //��ʱ��0��ʼ��
#if LOGRANK_UART1 >= 1
    Init_uart1();     //����1-USB���Գ�ʼ��
#endif
    Init_uart2();     //����2-Wifi EMW3080��ʼ��
	Init_wkt(1.0,false);//���õ��绽�Ѽ�ʱ��,��λms.����0.5ms-min(0),16384ms-max(32767);ÿ������0.5ms
    Init_interrupt(); //�ж�ϵͳ��ʼ��-���� ���ȼ�
#if LOGRANK_UART1 >= 2
        printf("LOG#:Internal Init Finish\r\n");
#endif
	
    //�ⲿ
	CloudInit();
    DS18B20STInit(); //��ʼ��DS18B20�ڴ�&��ȡ��ǰDS18B20�ֱ���
	LCD1602Init();    ////��ʼ��LCD1602
#if LOGRANK_UART1 >= 2
    printf("LOG#:External Init Finish\r\n");
#endif
}
void Init_rst(void)
{
    //���Ź��Ĵ��� // WDT_FLAG - EN_WDT CLR_WDT IDL_WDT WDT_PS[2:0]
    WDT_CONTR = WDT_CONTR & ~EN_WDT;//���ؿ��Ź�
	//WDT_CONTR |= 0x07;//���Ź���Ƶϵ�����
    //��ѹ��λ/RST���� ���üĴ��� // - ENLVR - P54RST - - LVDS[1:0]
    RSTCFG = RSTCFG & ~0x40 & ~0x10;
    RSTCFG &= 0xFC; //LVDS[1:0] ��ѹ����ż���ѹ:00 2.2V/01 2.4V/10 2.7V/11 3.0V
}
void Init_io(void)
{
    P3M1 &= 0xFE, P3M0 &= 0xFE; //P3.0Ϊ׼˫���
    P3M1 &= 0xFD, P3M0 |= 0x02; //P3.1Ϊ�������

    P1M1 &= 0xFE, P1M0 &= 0xFE; //P1.0Ϊ׼˫���
    P1M1 &= 0xFD, P1M0 |= 0x02; //P1.1Ϊ�������

    P3M1 &= 0xBF, P3M0 |= 0x40; //P3.6Ϊ�������
}
void Init_timer0(void)//1ms
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0x9A;		//���ö�ʱ��ֵ
	TH0 = 0xA9;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
}
#if LOGRANK_UART1 >= 1
void Init_uart1(void)
{
	//�봮��2���ö�ʱ��2
    SCON = 0x40;                 // SM0/FE [SM1] SM2 REN TB8 RB8 TI RI
    PCON &= 0x3F;                // [~SMOD ~SMOD0] LVDF POF GF1 GF0 PD IDL
    AUXR = AUXR | S1ST2; // T0x12 T1x12 UART_M0x6 T2R T2_C/T T2x12 EXTRAM [S1ST2]
    uart1_busy = false;
}
#endif
void Init_uart2(void)
{
	//��ʱ��2
	ulong BTL = UART_16(115200, 1);
	AUXR |= T2x12;//T2�ٶ�x12
	T2L = BTL & 0xFF;
    T2H = BTL >> 8;
    S2CON = 0x10;   // [~S4SM0] [/] S4SM2 [S4REN] S4TB8 S4RB8 S4TI S4RI
	
    uart2_busy=false;
    uart2_idx1 = uart2_idx2 = 0;
	
    AUXR |= T2R; //������ʱ��2
}
void Init_wkt(float ms,bool en)//���õ��绽�Ѽ�ʱ��,��λms.����0.5ms-min(0),16384ms-max(32767);ÿ������0.5ms
{
	pdata ushort T;
	if(en==false)
		return;
	T = ((ms*2.0)-1);
	T &= 0x7FFF;
	WKTCL = T&0x00FF;
	WKTCH = (T>>8)&0x007F;
	WKTCH |= 0x80;//�������绽�Ѽ�ʱ��
}
void Init_interrupt(void)
{
    ET0 = 1;                 //������ʱ��0�ж�
    ES = 1, IE2 &= ~ET2;     //��������1�ж�,�رն�ʱ��2�ж�
    IE2 |= ES2; 				//��������2�ж�

    PT0 = 0, IPH |= PT0H; //��ʱ��0���ȼ�2,ϵͳʱ�価���ܾ�ȷ,������ȼ�
    PS = 0, IPH &= ~PSH;  //����1���ȼ�0
	IP2 |= PS2, IP2H |= PS2H;//����2���ȼ�3

    EA = 1; //�������жϿ���
}

//------------------------------------------------------------------------------------------------//