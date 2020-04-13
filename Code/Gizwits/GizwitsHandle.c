#include "GizwitsHandle.h"
//------------------------------------------------------------------------------------------------//
xdata GizwitsActST GizwitsST; //Gizwits任务指示器

//GizwitsReceive处理内存
static pdata ushort GizwitsReceiveCount; //当前接收到的数据包长度
static pdata uchar GizwitsReceiveState;  //当前接收数据包进度
static pdata uchar GizwitsReceiveLast;   //上一个接收数据,用于过滤0x55
//GizwitsReceive - 数据包
pdata ushort GizwitsReceiveLen;                             //当前接收的数据包Len值
pdata uchar GizwitsReceiveCmd;                              //当前接收的数据包Cmd值
pdata uchar GizwitsReceiveSn;                               //当前接收的数据包Sn值
pdata ushort GizwitsReceiveFlags;                           //当前接收的数据包Flags值
xdata uchar GizwitsReceiveBuffer[GizwitsReceiveBufferSize]; //当前接收的数据包负载缓存区
pdata ushort GizwitsReceiveIdx;                             //当前接收的数据包负载长度
pdata uchar GizwitsReceiveSum;                              //当前接收的数据包Sum值

//GizwitsSend - 数据包
xdata uchar GizwitsSendBuffer[GizwitsSendBufferSize]; //发送数据包缓存区
pdata ushort GizwitsSendIdx;                          //发送数据包缓存区下标

//GizwitsSendOld - GizwitsSend数据包历史记录,Ans数据包GizwitsSendOldQSize个,末尾固定为Ask数据包
xdata uchar GizwitsSendOldBuffer[GizwitsSendOldQSize + 1][GizwitsSendBufferSize]; //发送数据包缓存区历史记录
xdata ushort GizwitsSendOldIdx[GizwitsSendOldQSize + 1];                          //发送数据包缓存区下标
xdata uchar GizwitsSendOldHead, GizwitsSendOldTail;                               //队列头尾下标

#ifdef GizwitsUseWiFiRealTime
xdata GizwitsTimeFromWiFi GizwitsRealTime; //从WiFi模组获取的现实时间
#endif
#ifdef GizwitsUseWiFiProperty
xdata GizwitsPropertyFromWiFi GizwitsWiFiProperty; //从WiFi获取WiFi的设备信息
#endif

//------------------------------------------------------------------------------------------------//
static void GizwitsReceive(void);     //从WiFi串口缓存区读取数据包
static void GizwitsReceiveInit(void); //初始化/重启Giawits串口接收
static uchar GizwitsSum(uchar);       //计算校验和
static uchar GizwitsSendOld(void);    //查找已发送数据包历史记录,找到则重新发送
//------------------------------------------------------------------------------------------------//

void GizwitsInit(void) //初始化Gizwits内存
{
    GizwitsReceiveInit();
    memset(&GizwitsST, 0, sizeof(GizwitsST));
    GizwitsSendIdx = 0;
    GizwitsSendOldHead = GizwitsSendOldTail = 0;
    GizwitsST.GizwitsIllegalCode = 0;
#if LOGRANK_UART1 >= 2
        printf("LOG#:GizwitsInit ok\r\n");
#endif
}
void GizwitsMainLoop(void) //Gizwits主循环函数
{
    GizwitsReceive();
    GizwitsAct();
}
void GizwitsSend(uchar Mode) //发送GizwitsSendBuffer缓存区内的数据包&记录已发送数据包
{
    pdata ushort i;
    ++GizwitsSendIdx;                                      //加上校验和长度1
    GizwitsSendBuffer[GizwitsSendIdx - 1] = GizwitsSum(1); //计算校验和
#if LOGRANK_UART1 >= 2
    //日志记录MCU即将发送数据包
    printf("LOG#:GizwitsSend   :%04x %02bx %02bx", GizwitsSendIdx, GizwitsSendBuffer[0], GizwitsSendBuffer[1]); //Len,Cmd
    i = GizwitsSendBuffer[2], i <<= 8, i |= GizwitsSendBuffer[3];
    printf(" %04x", i); //Flags
    for (i = 4; i != GizwitsSendIdx - 1; ++i)
        printf(" %02bx", GizwitsSendBuffer[i]); //负载
    printf(" %02bx\r\n", GizwitsSendBuffer[GizwitsSendIdx - 1]);
#endif
    uart4_send8(0xFF), uart4_send8(0xFF);                                          //发送固定包头
    uart4_send8((GizwitsSendIdx >> 8) & 0xFF), uart4_send8(GizwitsSendIdx & 0xFF); //发送数据包长度
    for (i = 0; i != GizwitsSendIdx; ++i)
        uart4_send8(GizwitsSendBuffer[i]); //发送数据包datas
    //添加至历史记录队列
    if (Mode == 1) //Ans 历史记录
    {
        if (GizwitsSendOldHead == GizwitsSendOldTail)
        {
            if (GizwitsSendOldHead + 1 == GizwitsSendOldQSize)
                GizwitsSendOldHead = 0;
            else
                ++GizwitsSendOldHead;
        }
        GizwitsSendOldIdx[GizwitsSendOldTail] = GizwitsSendIdx;
        for (i = 0; i != GizwitsSendIdx; ++i)
            GizwitsSendOldBuffer[GizwitsSendOldTail][i] = GizwitsSendBuffer[i];
        if (GizwitsSendOldTail + 1 == GizwitsSendOldQSize)
            GizwitsSendOldTail = 0;
        else
            ++GizwitsSendOldTail;
    }
    if (Mode == 2) //Ask 历史记录
    {
        GizwitsSendOldIdx[GizwitsSendOldQSize] = GizwitsSendIdx;
        for (i = 0; i != GizwitsSendIdx; ++i)
            GizwitsSendOldBuffer[GizwitsSendOldQSize][i] = GizwitsSendBuffer[i];
    }
}
static void GizwitsReceive(void) //从WiFi串口缓存区读取数据包
{
    pdata ushort Cidx1 = uart4_idx1, Cidx2 = uart4_idx2;
    //锁定串口缓存区当前可读取数据
    while (Cidx1 != Cidx2)
    {
        ++GizwitsReceiveCount; //收到的数据包长度+1
        if (GizwitsReceiveCount >= 4 &&
            GizwitsReceiveLast == 0xFF && uart4_buffer[Cidx1] == 0x55) //非固定包头阶段过滤0x55数据
            goto WhileEnd;
        if (GizwitsReceiveCount >= 3)
            GizwitsReceiveLast = uart4_buffer[Cidx1]; //更新历史数据
        switch (GizwitsReceiveState)
        {
        case 0: //固定包头接收阶段
            if (uart4_buffer[Cidx1] == 0xFF)
            {
                if (GizwitsReceiveCount == 2)
                    ++GizwitsReceiveState;
            }
            else //发现异常包头数据,重启Giawits串口接收
            {
#if LOGRANK_UART1 >= 1
                printf("ERR:GizwitsReceive can't find 0xFF\r\n");
#endif
                GizwitsReceiveInit();
            }
            break;
        case 1:
            GizwitsReceiveLen <<= 8;
            GizwitsReceiveLen |= uart4_buffer[Cidx1]; //读取数据包Len值
            if (GizwitsReceiveCount == 4)
            {
                ++GizwitsReceiveState;
                if (GizwitsReceiveLen - 5 > GizwitsReceiveBufferSize) //负载长度将导致数据包负载缓存区溢出
                {
#if LOGRANK_UART1 >= 1
                    printf("ERR:GizwitsReceive Len, but it will cause ReceiveBuffer overflow\r\n");
#endif
                    GizwitsReceiveInit(); //重启Giawits串口接收
                }
            }
            break;
        case 2:
            GizwitsReceiveCmd = uart4_buffer[Cidx1]; //读取数据包Cmd值
            ++GizwitsReceiveState;
            break;
        case 3:
            GizwitsReceiveSn = uart4_buffer[Cidx1]; ////读取数据包Sn值
            ++GizwitsReceiveState;
            break;
        case 4:
            GizwitsReceiveFlags <<= 8;
            GizwitsReceiveFlags |= uart4_buffer[Cidx1]; //读取数据包Flags值
            if (GizwitsReceiveCount == 8)
                if (GizwitsReceiveLen - 5 > 0) //判断是否含有负载
                    ++GizwitsReceiveState;
                else
                    GizwitsReceiveState += 2;
            break;
        case 5:
            GizwitsReceiveBuffer[GizwitsReceiveIdx++] = uart4_buffer[Cidx1]; //读取数据包
            if (GizwitsReceiveCount - 3 == GizwitsReceiveLen)                //数据包剩余长度为1,进入最后读取阶段
                ++GizwitsReceiveState;
            break;
        case 6:
            GizwitsReceiveSum = uart4_buffer[Cidx1]; ////读取数据包Sum值
#if LOGRANK_UART1 >= 1
            if (GizwitsReceiveLen + 4 != GizwitsReceiveCount) //发现数据包读取长度不足,未知原因
                printf("ERR:GizwitsReceive receive one packet, but len invalid\r\n");
            else
#endif
                if (GizwitsReceiveSum == GizwitsSum(0)) //校验和正确,接收到一个数据包
            {
                ushort ReCmdFlags = GizwitsAnalyseCmd(GizwitsReceiveCmd, GizwitsReceiveFlags); //分析数据包Cmd及检查Flags
                uchar ReCmd = ReCmdFlags & 0xFF, ReFlags = (ReCmdFlags >> 8) & 0xFF;
#if LOGRANK_UART1 >= 2
                //日志汇报MCU接收到格式及校验和正确的数据包
                ushort i;
                printf("LOG#:GizwitsReceive:%04x %02bx %02bx %04x", GizwitsReceiveLen,
                       GizwitsReceiveCmd, GizwitsReceiveSn, GizwitsReceiveFlags); //Len,Cmd,Sn,Flags
                for (i = 0; i != GizwitsReceiveIdx; ++i)
                    printf(" %02bx", GizwitsReceiveBuffer[i]); //负载
                printf(" %02bx\r\n", GizwitsReceiveSum);       //Sum
#endif
                if (ReCmd == 0x00)
                {
                    if (ReCmdFlags == ReCmd_FlagsERR) //数据包Flags异常
                    {
#if LOGRANK_UART1 >= 1
                        printf("ERR:Packet Flags error,Ans->0x12_0x03\r\n");
#endif
                        GizwitsST.GizwitsIllegalCode = 0x03;
                        GizwitsHandleWiFiAsk(0x12);
                    }
                    else if (ReCmdFlags == ReCmd_CmdERR) //数据包Cmd无法识别
                    {
#if LOGRANK_UART1 >= 1
                        printf("ERR:Packet Cmd invalid or error,Ans->0x12_0x02\r\n");
#endif
                        GizwitsST.GizwitsIllegalCode = 0x02;
                        GizwitsHandleWiFiAsk(0x12);
                    }
                    else if (ReCmdFlags == ReCmd_MSGERR) //WiFi回应对应包异常,需要重新发送
                    {
#if LOGRANK_UART1 >= 2
                        printf("LOG#:Ask one packet is illegal,MCU will sent again\r\n");
#endif
                        if (GizwitsSendOld() != 0)
                        {
#if LOGRANK_UART1 >= 1
                            printf("ERR: GizwitsSendOld Fail,Sn is not found,can't sent again\r\n");
#endif
                        }
                    }
                    else
                    {
#if LOGRANK_UART1 >= 1
                        printf("ERR:Packet unknown error,Ans->0x12_0x03\r\n");
#endif
                        GizwitsST.GizwitsIllegalCode = 0x03;
                        GizwitsHandleWiFiAsk(0x12);
                    }
                }
                else
                {
                    if (ReFlags == 0x00) //数据包是 Ask
                        GizwitsHandleWiFiAsk(ReCmd);
                    else //数据包是 Ans
                        GizwitsHandleWiFiAns(ReCmd);
                }
            }
            else //校验和错误
            {
#if LOGRANK_UART1 >= 1
                printf("ERR: Packet Sum check fail,Ans->0x12_0x01\r\n");
#endif
                GizwitsST.GizwitsIllegalCode = 0x01;
                GizwitsHandleWiFiAsk(0x12);
            }
            GizwitsReceiveInit(); //重启Giawits串口接收
            break;
        }
    WhileEnd:
        ++Cidx1; //读取锁定区域下1字节,释放1字节串口缓冲区空间
        if (Cidx1 == uart4_buffer_size)
            Cidx1 = 0, uart4_idx1 = 0;
        uart4_idx1 = Cidx1;
    }
}
static void GizwitsReceiveInit(void) //初始化/重启Giawits串口接收
{
    GizwitsReceiveCount = 0;
    GizwitsReceiveState = 0;
    GizwitsReceiveLast = 0;
    GizwitsReceiveLen = 0;
    GizwitsReceiveCmd = 0;
    GizwitsReceiveSn = 0;
    GizwitsReceiveFlags = 0;
    GizwitsReceiveIdx = 0;
    GizwitsReceiveSum = 0;
}
static uchar GizwitsSendOld(void) //查找已发送数据包历史记录,找到则重新发送
{
    //Ask等待Ans中且检查Sn与等待Ans的Ask是否一致
    if (GizwitsST.NeedAns = true && GizwitsReceiveSn == GizwitsSendOldBuffer[GizwitsSendOldQSize][1])
    {
        GizwitsReAsk(2); //立即重新发送Ask
        return 0;
    }
    else //不是Ask,查询Ans是否有对应Sn
    {
        pdata uchar idx = GizwitsSendOldHead;
        while (idx != GizwitsSendOldTail)
        {
            if (GizwitsReceiveSn == GizwitsSendOldBuffer[idx][1]) //成功在Ans中找到对应Sn
            {
                pdata ushort i;
                uart4_send8(0xFF), uart4_send8(0xFF); //发送固定包头
                uart4_send8((GizwitsSendOldIdx[idx] >> 8) & 0xFF);
                uart4_send8(GizwitsSendOldIdx[idx] & 0xFF); //发送数据包长度
                for (i = 0; i != GizwitsSendOldIdx[idx]; ++i)
                    uart4_send8(GizwitsSendOldBuffer[idx][i]); //发送数据包datas
#if LOGRANK_UART1 >= 2
                printf("LOG#:GizwitsSendOld Ans ok\r\n");
#endif
                return 0;
            }
            if (idx == GizwitsSendOldQSize)
                idx = 0;
            else
                ++idx;
        }
    }
    return 1; //历史记录中未找到对应Sn,重新发送失败
}
void GizwitsReAsk(uchar Mode) //延时等待WiFi模组Ans&重新发送MCU的Ask数据包
{
    pdata ushort i;
    if (GizwitsST.NeedAns_RstCount == 3) //重发Ask三次还未得到Ans,放弃等待Ans与重发
    {
        GizwitsST.NeedAns = 0;
        GizwitsST.NeedAns_RstCount = 0;
        return;
    }
    if (Mode == 1) //主函数任务模式,延时等待
    {
        if (GizwitsST.GizwitsSysMs - GizwitsST.NeedAns_Ms >= 200) //200ms未收到Ans,重发Ask
            ++GizwitsST.NeedAns_RstCount;                  //记录一次重发Ask次数
        else
            return; //继续等待Ans
    }
    if (Mode == 2) //WiFi请求重发Ask模式,立即发送并重置等待时间&次数
        GizwitsST.NeedAns_RstCount = 0;
    uart4_send8(0xFF), uart4_send8(0xFF); //发送固定包头
    uart4_send8((GizwitsSendOldIdx[GizwitsSendOldQSize] >> 8) & 0xFF);
    uart4_send8(GizwitsSendOldIdx[GizwitsSendOldQSize] & 0xFF); //发送数据包长度
    for (i = 0; i != GizwitsSendOldIdx[GizwitsSendOldQSize]; ++i)
        uart4_send8(GizwitsSendOldBuffer[GizwitsSendOldQSize][i]); //发送数据包datas
    GizwitsST.NeedAns_Ms = GizwitsST.GizwitsSysMs;                               //记录本次发送Ask的时间
#if LOGRANK_UART1 >= 2
    printf("LOG#:GizwitsReAsk Cnt-%bu\r\n", GizwitsST.NeedAns_RstCount); //日志记录重新发送Ask及已重发次数
#endif
}
static uchar GizwitsSum(uchar Mode) //计算校验和
{
    pdata uchar Sum = 0;
    pdata ushort i;
    switch (Mode)
    {
    case 0: //GizwitsReceive模式
        Sum += (GizwitsReceiveLen >> 8) & 0xFF;
        Sum += (GizwitsReceiveLen)&0xFF;
        Sum += GizwitsReceiveCmd;
        Sum += GizwitsReceiveSn;
        Sum += (GizwitsReceiveFlags >> 8) & 0xFF;
        Sum += (GizwitsReceiveFlags)&0xFF;
        for (i = 0; i != GizwitsReceiveIdx; ++i)
            Sum += GizwitsReceiveBuffer[i];
        break;
    case 1: //GizwitsSend模式
        Sum += (GizwitsSendIdx >> 8) & 0xFF;
        Sum += (GizwitsSendIdx)&0xFF;
        for (i = 0; i < GizwitsSendIdx - 1; ++i)
            Sum += GizwitsSendBuffer[i];
        break;
    }
    return Sum;
}
//------------------------------------------------------------------------------------------------//