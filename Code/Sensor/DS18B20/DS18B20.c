#include "DS18B20.h"
//------------------------------------------------------------------------------------------------//
xdata DS18B20ActST DS18B20ST;
code ushort DS18B20ConvertTMaxTime[4] = {95, 190, 378, 753};       //温度转换最大等待时间(ms)
code float DS18B20ReTransfrom[4] = {0.5F, 0.25F, 0.125F, 0.0625F}; //恢复Float值所需系数
//------------------------------------------------------------------------------------------------//
static bool DS18B20Init(void);            //初始化程序
static void DS18B20WriteByte(uchar Byte); //写入一个字节数据
static uchar DS18B20ReadByte(void);       //读取一个字节数据
//------------------------------------------------------------------------------------------------//
void DS18B20STInit(void) //初始化DS18B20内存&读取当前DS18B20分辨率
{
    uchar Temp, TH, TL;
    memset(&DS18B20ST, 0, sizeof(DS18B20ActST)); //初始化后的缓存温度值应为DS18B20MinT
    if (DS18B20Init() == EXIT_FAILURE)           //DS18B20初始化失败
    {
#if LOGRANK_UART1 >= 1
        printf("ERR:DS18B20Init fail when DS18B20STInit!\r\n");
#endif
        return; //放弃读取DS18B20分辨率.并日志记录ERR信息
    }
    DS18B20WriteByte(DS18B20SkipROM);                   //写入跳过ROM操作
    DS18B20WriteByte(DS18B20ReadRAM);                   //写入读取RAM指令
    Temp = DS18B20ReadByte(), Temp = DS18B20ReadByte(); //跳过温度,此时不能趁机读取温度,是无效值
    TH = DS18B20ReadByte(), TL = DS18B20ReadByte();     //跳过TH/TL
    Temp = DS18B20ReadByte();                           //读取配置寄存器
    DS18B20ST.ResolutionMode = (Temp >> 5) & 0x03;      //存入DS18B20任务指示器中
    DS18B20ST.TemperatureHighData1 = TemperatureH;      //确定当前Mode下高温报警阈值
    DS18B20ST.TemperatureHighData2 = DS18B20ST.TemperatureHighData1 - TemperatureT;
    DS18B20ST.TemperatureLowData1 = TemperatureL; //确定当前Mode下低温报警阈值
    DS18B20ST.TemperatureLowData2 = DS18B20ST.TemperatureLowData1 + TemperatureT;
    DS18B20ConvertTemperature();      //初始化-开始转换温度#忽略返回值
    switch (DS18B20ST.ResolutionMode) //对应Mode延时
    {
    case 0:
        delay_ms(100);
        break;
    case 1:
        delay_ms(200);
        break;
    case 2:
        delay_ms(385);
        break;
    case 3:
        delay_ms(765);
        break;
    }
    DS18B20GetTemperature(); //初始化-读取温度值#忽略返回值
#if LOGRANK_UART1 >= 2
    printf("LOG#:DS18B20Init ok\r\n");
#endif
#if LOGRANK_UART1 >= 3
    printf("LOG:TH-%bu,TL-%bu,Mode-%bu,H1-%u,H2-%u,L1-%u,L2-%u\r\n",
           TH, TL, DS18B20ST.ResolutionMode,
           DS18B20ST.TemperatureHighData1, DS18B20ST.TemperatureHighData2,
           DS18B20ST.TemperatureLowData1, DS18B20ST.TemperatureLowData2);
#endif
}
bool DS18B20Set(char TH, char TL, uchar Mode) //设置并保存TH/TL/Mode
{
    if (DS18B20Init() == EXIT_FAILURE) //DS18B20初始化失败,返回错误码EXIT_FAILURE
        return EXIT_FAILURE;
    DS18B20WriteByte(DS18B20SkipROM);  //写入跳过ROM操作
    DS18B20WriteByte(DS18B20WriteRAM); //写入写入RAM指令
    DS18B20WriteByte(TH);
    DS18B20WriteByte(TL);
    DS18B20WriteByte(Mode << 5);
    if (DS18B20Init() == EXIT_FAILURE) //DS18B20初始化失败,返回错误码EXIT_FAILURE
        return EXIT_FAILURE;
    DS18B20WriteByte(DS18B20SkipROM); //写入跳过ROM操作
    DS18B20WriteByte(DS18B20CopyRAM); //写入保存RAM到E2CROM
    return EXIT_SUCCESS;
}
bool DS18B20ConvertTemperature(void) //开始转换温度值
{
    if (DS18B20Init() == EXIT_FAILURE) //DS18B20初始化失败,返回错误码EXIT_FAILURE
        return EXIT_FAILURE;
    DS18B20WriteByte(DS18B20SkipROM);  //写入跳过ROM操作
    DS18B20WriteByte(DS18B20ConvertT); //写入开始转换温度指令
    return EXIT_SUCCESS;
}
bool DS18B20GetTemperature(void) //读取温度值
{
    pdata uchar H, L;
    pdata ushort T, Base = DS18B20MinT;
    Base = -Base, Base <<= (1 + DS18B20ST.ResolutionMode); //Gizwit传输基准值
    if (DS18B20Init() == EXIT_FAILURE)                     //DS18B20初始化失败,返回错误码DS18B20InitFail
        return EXIT_FAILURE;
    DS18B20WriteByte(DS18B20SkipROM);             //写入跳过ROM操作
    DS18B20WriteByte(DS18B20ReadRAM);             //写入读取RAM指令
    L = DS18B20ReadByte(), H = DS18B20ReadByte(); //读取温度寄存器
    T = H, T <<= 8, T |= L;                       //合并读取到的温度
    T >>= (3 - (DS18B20ST.ResolutionMode));       //修正为当前分辨率
    if ((T & 0x1000) && (T & 0x0800))             //温度值为负数,以两个符号位确定
    {
        T = ~T, ++T;  //取原码
        T = Base - T; //以基准值计算最终无符号类型温度值
    }
    else
        T += Base; //以基准值计算最终无符号类型温度值
    if (T != DS18B20ST.TemperatureData)
    {
        DS18B20ST.TemperatureData = T; //更新温度数据
        CloudAct.NeedReport = true;    //与历史温度值不同,立即挂起设备上报任务
        if (DS18B20ST.TemperatureLow == false && T <= (DS18B20ST.TemperatureLowData1))
            DS18B20ST.TemperatureLow = true, CloudAct.NeedReport_WaterTemperatureLow = true; //低温报警
        if (DS18B20ST.TemperatureLow == true && T >= (DS18B20ST.TemperatureLowData2))
            DS18B20ST.TemperatureLow = false, CloudAct.NeedReport_WaterTemperatureLow = true; //低温报警取消
        if (DS18B20ST.TemperatureHigh == false && T >= (DS18B20ST.TemperatureHighData1))
            DS18B20ST.TemperatureHigh = true, CloudAct.NeedReport_WaterTemperatureHigh = true; //高温报警
        if (DS18B20ST.TemperatureHigh == true && T <= (DS18B20ST.TemperatureHighData2))
            DS18B20ST.TemperatureHigh = false, CloudAct.NeedReport_WaterTemperatureHigh = true; //高温报警取消
    }
    return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------------------------//
static bool DS18B20Init(void) //初始化程序
{
    pdata uchar i = 20;
    DS18B20_DATA = 1, delay_us(60), DS18B20_DATA = 0;
    delay_us(485), DS18B20_DATA = 1; //延时等待后上拉总线
    do
    {
        delay_us(10);
    } while (DS18B20_DATA == 1 && (--i)); //等待DS18B20应答
    if (i == 0)
    {
#if LOGRANK_UART1 >= 1
        printf("ERR:DS18B20Init fail\r\n"); //日志记录DS18B20初始化未收到应答
#endif
        return EXIT_FAILURE;
    }
    DS18B20_DATA = 1; //发送已经就位
    delay_us(130);    //等待DS18B20就位,实测150以上绝对稳定
    return EXIT_SUCCESS;
}
static void DS18B20WriteByte(uchar Byte) //写入一个字节数据
{
    pdata uchar i;
    for (i = 0; i != 8; ++i, Byte >>= 1)
    {
        DS18B20_DATA = 0;
        delay_us(1);
        DS18B20_DATA = Byte & 0x01;
        delay_us(60);
        DS18B20_DATA = 1;
        delay_us(1);
    }
}
static uchar DS18B20ReadByte(void) //读取一个字节数据
{
    pdata uchar i, re;
    DS18B20_DATA = 1;
    delay_us(10);
    for (i = 0, re = 0; i != 8; ++i)
    {
        re >>= 1;
        DS18B20_DATA = 0;
        delay_us(1);
        DS18B20_DATA = 1;
        delay_us(1);
        if (DS18B20_DATA == 1)
            re |= 0x80;
        delay_us(60);
    }
    return re;
}
//------------------------------------------------------------------------------------------------//