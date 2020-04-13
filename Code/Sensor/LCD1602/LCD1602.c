#include "LCD1602.h"
//------------------------------------------------------------------------------------------------//
//自制字模
code uchar LCD1602Word_Temperature[8]={0x10,0x06,0x09,0x08,0x08,0x09,0x06,0x00};//摄氏度单位,CGRAM=0
code uchar LCD1602Word_Heart[8]={0x00,0x0A,0x1F,0x1F,0x0E,0x04,0x00,0x00};//爱心,CGRAM=1
//------------------------------------------------------------------------------------------------//
static void LCD1602Wait(void); //等待LCD1602空闲
static void LCD1602LoadWord(uchar Idx,const uchar *Word);//加载字模到CGRAM
static void LCD1602SetCGRAM(uchar idx); //0x00-0x07,修改CGRAM位置(自制字模RAM位置)
//------------------------------------------------------------------------------------------------//
void LCD1602Init(void) //初始化LCD1602
{
	LCD1602_EN = 0;//保持不操作时拉低EN
    LCD1602WriteCmdOrData(LCD1602Data8Dis2Mat5x7, 0);
    LCD1602WriteCmdOrData(LCD1602NotCur, 0);
    LCD1602WriteCmdOrData(LCD1602CurRight, 0);
    LCD1602WriteCmdOrData(LCD1602CleanScr, 0);
	//加载自制字模
	LCD1602LoadWord(0,LCD1602Word_Temperature);
	LCD1602LoadWord(1,LCD1602Word_Heart);
	LCD1602WriteLine("LCD1602",false);
	LCD1602WriteLine("Ready",true);
#if LOGRANK_UART1 >= 2
    printf("LOG#:LCD1602Init ok\r\n");
#endif
}
void LCD1602WriteLine(uchar *Str,bool isLine2)//在1/2行写入字符串
{
	uchar i;
	isLine2?LCD1602SetDDRAM(0x40):LCD1602SetDDRAM(0x00);
	for(i=0;Str[i]&&i!=40;++i)
		LCD1602WriteCmdOrData(Str[i],1);
}
uchar LCD1602ReadIdx(void) //读取当前DDRAM位置(光标位置)
{
    uchar Idx;
    LCD1602Wait();
    LCD1602_RS = 0;
    LCD1602_RW = 1;
    LCD1602_EN = 1;
	LCD1602_DATA=0xFF;
	delay_us(40);//等待中
	Idx=LCD1602_DATA;
    LCD1602_EN = 0;
    return Idx;
}
uchar LCD1602ReadData(void) //读取DDRAM/CGRAM数据
{
    uchar Data;
    LCD1602Wait();
    LCD1602_RS = 1;
    LCD1602_RW = 1;
    LCD1602_EN = 1;
	LCD1602_DATA=0xFF;
	delay_us(40);//等待中
	Data=LCD1602_DATA;
	LCD1602_EN = 0;
    return Data;
}
void LCD1602SetDDRAM(uchar idx) //0x00-0x0F-0x27,0x40-0x4F-0x67,修改DDRAM位置(光标位置)
{
    idx |= 0x80;
    LCD1602WriteCmdOrData(idx, 0);
}
void LCD1602WriteCmdOrData(uchar CmdOrData, bool isData) //写入命令/DDRAM or CGRAM数据
{
    LCD1602Wait();
    LCD1602_RS = isData;
    LCD1602_RW = 0;
    LCD1602_DATA = CmdOrData;
    LCD1602_EN = 1;
    LCD1602_EN = 0;
}
//------------------------------------------------------------------------------------------------//
static void LCD1602LoadWord(uchar Idx,const uchar *Word)//加载字模到CGRAM
{
	uchar i;
	delay_ms(1);//延时保证命令正确传递
	LCD1602SetCGRAM(Idx);
	delay_ms(1);//延时保证LCD1602已经切换为CGRAM接收状态
	for(i=0;i!=8;++i)
		LCD1602WriteCmdOrData(Word[i],1);
}
static void LCD1602SetCGRAM(uchar idx) //0x00-0x07,修改CGRAM位置(自制字模RAM位置)
{
    idx &= 0x07, idx <<= 3, idx |= 0x40;
    LCD1602WriteCmdOrData(idx, 0);
}
static void LCD1602Wait(void) //等待LCD1602空闲
{
    bool isBusy;
    LCD1602_RS = 0;
    LCD1602_RW = 1;
    do
    {
        LCD1602_EN = 1;
		LCD1602_Busy=1;
		delay_us(40);//等待中
		isBusy=LCD1602_Busy;
		LCD1602_EN = 0;
    } while (isBusy);
}
//------------------------------------------------------------------------------------------------//