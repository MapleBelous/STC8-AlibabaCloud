#include "GizwitsMSG.h"
//------------------------------------------------------------------------------------------------//
static void GizwitsSetFifiControl(void); //从GizwitsReceiveBuffer中读取控制数据,立即执行
static void GizwitsGetDevState(void);    //向GizwitsReceiveBuffer中写入设备状态信息

//属于GizwitsHandleWiFiAsk的子函数,Cmd=0x0E,用于读取WiFi模组工作状态
static void GizwitsWiFiAsk0x0E(void);
//属于GizwitsHandleWiFiAns的子函数,Cmd=0x17且使用WiFi的RealTime信息时提取数据
static void GizwitsWiFiAns0x17(void);
//属于GizwitsHandleWiFiAns的子函数,Cmd=0x21且使用WiFi的设备信息时提取数据
static void GizwitsWiFiAns0x21(void);
//属于GizwitsHandleMCUAsk的子函数,Cmd=0x09且GizwitsConfigMode=0x04时写入SendBuffer中WiFi配置信息
static void GizwitsMCUAsk0x09(void);

//------------------------------------------------------------------------------------------------//
void GizwitsAct(void) //处理当前任务
{
    //Gizwits系统定时挂起任务
#if GizwitsNeedReportMs != 0
    if (GizwitsST.NeedReport == false && GizwitsST.GizwitsSysMs - GizwitsST.NeedReport_Ms >= GizwitsNeedReportMs)
        GizwitsST.NeedReport = true; //需要上报设备状态
#endif
    //Gizwits系统待执行任务
    if (GizwitsST.NeedReport == true && GizwitsHandleMCUAsk(0x05) == 0) //上报完成,更新上报时间
        GizwitsST.NeedReport = false, GizwitsST.NeedReport_Ms = GizwitsST.GizwitsSysMs;
    if (GizwitsST.NeedAns == true) //等待Ans中
        GizwitsReAsk(1);
    if (GizwitsST.NeedRstMCU == true && (GizwitsST.GizwitsSysMs - GizwitsST.NeedRstMCU_Ms >= 600))
        MCURST(); //600ms延时后重启MCU

        //Sensor定时挂起任务
#if DS18B20NeedReadMs != 0
    if (GizwitsST.NeedReadDS18B20 == false && GizwitsST.GizwitsSysMs - GizwitsST.NeedReadDS18B20_Ms >= DS18B20NeedReadMs && DS18B20ConvertTemperature() == EXIT_SUCCESS) //需要读取DS18B20温度值,且温度转换指令成功
        GizwitsST.NeedReadDS18B20 = true, GizwitsST.NeedReadDS18B20_Ms = GizwitsST.GizwitsSysMs;                                                                         //等待读取温度值
#endif
    //Sensor待执行任务
    if (GizwitsST.NeedReadDS18B20 == true && GizwitsST.GizwitsSysMs - GizwitsST.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode] && DS18B20GetTemperature() == EXIT_SUCCESS) //成功执行读取温度值
        GizwitsST.NeedReadDS18B20 = false, GizwitsST.NeedReadDS18B20_Ms = GizwitsST.GizwitsSysMs;
}
void GizwitsHandleWiFiAsk(uchar Cmd) //参数Cmd: MCU待应答命令
{
#if LOGRANK_UART1 >= 2
    printf("LOG#:Handle WiFi Ask[%02bx]\r\n", Cmd); //日志记录MCU处理WiFi Ask,Cmd为MCU待发送Ans
#endif
    GizwitsSendIdx = 0;
    GizwitsSendBuffer[GizwitsSendIdx++] = Cmd;
    GizwitsSendBuffer[GizwitsSendIdx++] = GizwitsReceiveSn;
    switch (Cmd)
    {
    case 0x02: //WiFi模组请求设备信息
    {
        pdata ushort i;
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        for (i = 0; i != 8; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = SerialProtocolVer[i];
        for (i = 0; i != 8; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = BusinessProtocolVer[i];
        for (i = 0; i != 8; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = HardwareVer[i];
        for (i = 0; i != 8; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = SoftwareVer[i];
        for (i = 0; i != 32; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = ProductKey[i];
        GizwitsSendBuffer[GizwitsSendIdx++] = (BindingStateSec >> 8) & 0xFF;
        GizwitsSendBuffer[GizwitsSendIdx++] = (BindingStateSec)&0xFF;
        for (i = 0; i != 8; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = DeviceProperties[i];
        for (i = 0; i != 32; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = ProductSecret[i];
        GizwitsSendBuffer[GizwitsSendIdx++] = (DataLen >> 8) & 0xFF;
        GizwitsSendBuffer[GizwitsSendIdx++] = (DataLen)&0xFF;
        for (i = 0; i != DataLen; ++i)
            GizwitsSendBuffer[GizwitsSendIdx++] = Data[i];
		GizwitsST.WiFiConect=true;//发送Ans=0x02完毕,可以开始Ask
    }
    break;
    case 0x04:                                      //WiFi模组控制设备&读取设备的当前状态
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        {
            uchar Action = GizwitsReceiveBuffer[0];
            if (Action == 0x01) //WiFi模组控制设备
            {
                GizwitsSetFifiControl();     //从GizwitsReceiveBuffer中读取控制数据,立即执行
                GizwitsST.NeedReport = true; //需要上报设备状态
            }
            else //WiFi模组读取设备的当前状态
            {
                GizwitsSendBuffer[GizwitsSendIdx++] = 0x03;
                GizwitsGetDevState();         //向GizwitsReceiveBuffer中写入设备状态信息
                GizwitsST.NeedReport = false; //相当于执行了上报数据
            }
        }
        break;
    case 0x08:                                      //心跳
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        break;
    case 0x0E:                                      //WiFi推送模组工作状态
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsWiFiAsk0x0E(); //调用子程序处理WiFi模组工作状态信息
        break;
    case 0x10:                                      //WiFi模组请求重启MCU
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsST.NeedRstMCU = true;
        break;
    case 0x12:                                      //应答WiFi模组数据包非法
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsSendBuffer[GizwitsSendIdx++] = GizwitsST.GizwitsIllegalCode;
        break;
    default:
#if LOGRANK_UART1 >= 1
        printf("ERR:GizwitsHandleWiFiAsk can't Ans this AnsCmd[%02bx]\r\n", Cmd);
#endif
        GizwitsSendIdx = 0; //清空发送缓冲区
        return;
    }
    GizwitsSend(1); //发送 Ans 数据包
}
void GizwitsHandleWiFiAns(uchar Cmd) //参数Cmd: MCU已请求命令
{
    if (Cmd != GizwitsSendOldBuffer[GizwitsSendOldQSize][0] || GizwitsReceiveSn != GizwitsSendOldBuffer[GizwitsSendOldQSize][1])
    { //发现未知的错误,当前Ans与MCU等待应答的Ask,Sn或Cmd不符
#if LOGRANK_UART1 >= 1
        printf("ERR:GizwitsHandleWiFiAns found AskCmd[%02bx|%02bx]or Sn[%02bx|%02bx]illegal,Ans->0x12_0x02\r\n",
               Cmd, GizwitsSendOldBuffer[GizwitsSendOldQSize][0], GizwitsReceiveSn, GizwitsSendOldBuffer[GizwitsSendOldQSize][1]);
#endif
        GizwitsST.GizwitsIllegalCode = 0x02;
        GizwitsHandleWiFiAsk(0x12); //转为GizwitsHandleWiFiAsk应答数据包非法
        return;
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:Handle WiFi Ans[%02bx]\r\n", Cmd); //日志记录MCU处理WiFi Ans,Cmd为MCU已发送Ask
#endif
    GizwitsST.NeedAns = false; //取消等待WiFi应答标志,成功执行对应AskCmd
    switch (Cmd)               //处理WiFi Ans信息
    {
    case 0x05: //MCU向WiFi模组主动上报当前状态
    case 0x09: //MCU通知WiFi模组进入配置模式
    case 0x0B: //重置WiFi模组
    case 0x15: //通知WiFi模组进入可绑定模式
    case 0x29: //MCU重启通讯模组
    case 0x13: //MCU请求WiFi模组进入产测模式
        break; //没有信息需要处理
    case 0x17: //获取网络时间
#ifdef GizwitsUseWiFiRealTime
        GizwitsWiFiAns0x17();
#else
#if LOGRANK_UART1 >= 1
        printf("ERR:Unable GizwitsUseWiFiRealTime Def,ignore Ans datas\r\n");
#endif
#endif
        break;
    case 0x21: //获取通讯模组的信息
#ifdef GizwitsUseWiFiProperty
        GizwitsWiFiAns0x21();
#else
#if LOGRANK_UART1 >= 1
        printf("ERR:Unable GizwitsUseWiFiProperty Def,ignore Ans datas\r\n");
#endif
#endif
        break;
    default: //未知命令,严重错误,可能发生了意外的SendBuffer+ReceiveBuffer错误
#if LOGRANK_UART1 >= 1
        printf("ERR:MCU AskCmd for WiFi Ans is unknown,serious ERR!\r\n");
#endif
		break;
    }
}
uchar GizwitsHandleMCUAsk(uchar Cmd) //MCU发送Cmd命令Ask数据包
{
    static pdata uchar AskSn = 0; //本地Sn
    if (GizwitsST.NeedAns == 1||GizwitsST.WiFiConect==false)   //上次发送的Ask还未应答或还未与WiFi确认,取消发送
        return 1;
#if LOGRANK_UART1 >= 2
    printf("LOG#:Handle MCU Ask[%02bx]\r\n", Cmd); //日志记录MCU处理待发送Ask Cmd
#endif
    GizwitsSendIdx = 0;                          //初始化SendBuffer下标,准备生成Ask
    GizwitsSendBuffer[GizwitsSendIdx++] = Cmd;   //写入Cmd
    GizwitsSendBuffer[GizwitsSendIdx++] = AskSn; //写入Sn
    switch (Cmd)
    {
    case 0x05:                                      //MCU向WiFi模组主动上报当前状态
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x04; //action
        GizwitsGetDevState();
        break;
    case 0x09:                                      //MCU通知WiFi模组进入配置模式
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsSendBuffer[GizwitsSendIdx++] = GizwitsST.GizwitsConfigMode;
        if (GizwitsST.GizwitsConfigMode == 0x04) //MCU直接写入配置信息模式
            GizwitsMCUAsk0x09();                 //执行子函数写入本地WiFi配置信息
        break;
    case 0x0B:                                      //重置WiFi模组
    case 0x15:                                      //通知WiFi模组进入可绑定模式
    case 0x29:                                      //MCU重启通讯模组
    case 0x13:                                      //MCU请求WiFi模组进入产测模式
    case 0x17:                                      //获取网络时间
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        break;
    case 0x21:                                      //获取通讯模组的信息
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //Flags
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00;
        GizwitsSendBuffer[GizwitsSendIdx++] = 0x00; //type,固定为0x00,返回基本信息
        break;
    default: //未知命令,无法执行!
#if LOGRANK_UART1 >= 1
        printf("ERR:The AskCmd is unknown\r\n");
#endif
        GizwitsSendIdx = 0; //重置SendBuffer
        return 2;
    }
    GizwitsSend(2);
    GizwitsST.NeedAns = true;                      //进入等待Ans状态
    GizwitsST.NeedAns_RstCount = 0;                //初始化等待Ans,已重发Ask次数
    GizwitsST.NeedAns_Ms = GizwitsST.GizwitsSysMs; //初始化等待
    ++AskSn;                                       //更新AskSn值
    return 0;
}
//参数:数据包Cmd和Flags值
//1.若Cmd是WiFi->MCU(请求)
//检查Flags是否符合要求,正常返回0x00|ReCmd;
//2.若Cmd是WiFi->MCU(应答)
//检查Flags是否符合要求,正常返回0xFF|ReCmd;
//3.若Cmd是MCU->WiFi(请求|应答)
//检查Flags是否符合要求,正常返回0x0100;
//*.Flags检查异常,Re = 0xF000
//*.Cmd无法识别,Re = 0xF100
ushort GizwitsAnalyseCmd(uchar Cmd, ushort Flags)
{
    pdata ushort Re = 0;
    switch (Cmd)
    {
    //基础通讯协议
    case 0x01:               //Ask: WiFi模组请求设备信息——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0x0002; //MCU->WiFi Ans Cmd
        break;
    case 0x02:               //Ans: WiFi模组请求设备信息——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;

    case 0x03:               //Ask: WiFi模组控制设备&读取设备的当前状态——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0x0004; //MCU->WiFi Ans Cmd
        break;
    case 0x04:               //Ans: WiFi模组控制设备&读取设备的当前状态——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;

    case 0x05:               //Ask: MCU向WiFi模组主动上报当前状态——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x06:               //Ans: MCU向WiFi模组主动上报当前状态——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF05; //MCU->WiFi Ask Cmd
        break;

    case 0x07:               //Ask: 心跳——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0x0008; //MCU->WiFi Ans Cmd
        break;
    case 0x08:               //Ans: 心跳——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;

    case 0x09:               //Ask: MCU通知WiFi模组进入配置模式——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x0A:               //Ans: MCU通知WiFi模组进入配置模式——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF09; //MCU->WiFi Ask Cmd
        break;

    case 0x0B:               //Ask: 重置WiFi模组——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x0C:               //Ans: 重置WiFi模组——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF0B; //MCU->WiFi Ask Cmd
        break;

    case 0x0D:               //Ask: WiFi推送模组工作状态——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0x000E; //MCU->WiFi Ans Cmd
        break;
    case 0x0E:               //Ans: WiFi推送模组工作状态——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;

    case 0x11:               //ERR: 非法数据包通知——WiFi->MCU
    case 0x12:               //ERR: 非法数据包通知——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MSGERR; //MCU->WiFi Ans Cmd
        break;

    case 0x15:               //Ask: 通知WiFi模组进入可绑定模式——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x16:               //Ans: 通知WiFi模组进入可绑定模式——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF15; //MCU->WiFi Ask Cmd
        break;

    case 0x29:               //Ask: MCU重启通讯模组——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x2A:               //Ans: MCU重启通讯模组——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF29; //MCU->WiFi Ask Cmd
        break;

    //可选通讯协议
    case 0x0F:               //Ask: WiFi模组请求重启MCU——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0x0010; //MCU->WiFi Ans Cmd
        break;
    case 0x10:               //Ans: WiFi模组请求重启MCU——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;

    case 0x13:               //Ask: MCU请求WiFi模组进入产测模式——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x14:               //Ans: MCU请求WiFi模组进入产测模式——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF13; //MCU->WiFi Ask Cmd
        break;

    case 0x17:               //Ask: 获取网络时间——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x18:               //Ans: 获取网络时间——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF17; //MCU->WiFi Ask Cmd
        break;

    case 0x21:               //Ask: 获取通讯模组的信息——MCU->WiFi
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = ReCmd_MCU2WiFi; //Flags检查正常
        break;
    case 0x22:               //Ans: 获取通讯模组的信息——WiFi->MCU
        if (Flags != 0x0000) //该命令Flags异常
            Re = ReCmd_FlagsERR;
        else
            Re = 0xFF21; //MCU->WiFi Ask Cmd
        break;
        //大数据相关指令略

    default: //Cmd未登记,Cmd异常
        Re = ReCmd_CmdERR;
    }
    return Re;
}
static void GizwitsSetFifiControl(void) //从GizwitsReceiveBuffer中读取控制数据,立即执行
{
    pdata uchar MSGMask = GizwitsReceiveBuffer[1];
    pdata uchar MSGControl = GizwitsReceiveBuffer[2];
    if ((MSGMask & 0x01) != 0)
    {
        if ((MSGControl & 0x01) != 0)
            LED_RED = 0;
        else
            LED_RED = 1;
    }
    if ((MSGMask & 0x02) != 0)
    {
        if ((MSGControl & 0x02) != 0)
            LED_BLUE = 0;
        else
            LED_BLUE = 1;
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:WiFi Control ok[%02bx,%02bx]\r\n", MSGMask, MSGControl);
#endif
}
static void GizwitsGetDevState(void) //向GizwitsReceiveBuffer中写入设备状态信息
{
    pdata uchar MSGSTATE = 0;
    if (LED_RED == 0)
        MSGSTATE |= 0x01;
    if (LED_BLUE == 0)
        MSGSTATE |= 0x02;
    GizwitsSendBuffer[GizwitsSendIdx++] = MSGSTATE;
#if LOGRANK_UART1 >= 3
    printf("LOG:Get DevState[%02bx,%u,", MSGSTATE, DS18B20ST.TemperatureData);
#endif
    GizwitsSendBuffer[GizwitsSendIdx++] = (DS18B20ST.TemperatureData) >> 8; //写入缓存的温度值
    GizwitsSendBuffer[GizwitsSendIdx++] = (DS18B20ST.TemperatureData) & 0xFF;
    MSGSTATE = DS18B20ST.TemperatureLow, MSGSTATE <<= 1, MSGSTATE |= DS18B20ST.TemperatureHigh;
    GizwitsSendBuffer[GizwitsSendIdx++] = MSGSTATE;
#if LOGRANK_UART1 >= 3
    printf("%02bx]\r\n", MSGSTATE);
#endif
}
//属于GizwitsHandleWiFiAsk的子函数,Cmd=0x0E,用于读取WiFi模组工作状态
static void GizwitsWiFiAsk0x0E(void)
{
    if (GizwitsST.OpenSoftAP ^ (GizwitsReceiveBuffer[1] & 0x01)) //SoftAP模式发生变化
    {
        GizwitsST.OpenSoftAP = ~(GizwitsST.OpenSoftAP);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s SoftAP Mode\r\n", GizwitsST.OpenSoftAP == true ? "Enter" : "Exit");
#endif
    }
    GizwitsReceiveBuffer[1] >>= 1;
    if (GizwitsST.OpenStation ^ (GizwitsReceiveBuffer[1] & 0x01)) //Station模式发生变化
    {
        GizwitsST.OpenStation = ~(GizwitsST.OpenStation);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s Station Mode\r\n", GizwitsST.OpenStation == true ? "Enter" : "Exit");
#endif
    }
    GizwitsReceiveBuffer[1] >>= 1;
    if (GizwitsST.OpenOnBoarding ^ (GizwitsReceiveBuffer[1] & 0x01)) //配置(OnBoarding)模式发生变化
    {
        GizwitsST.OpenOnBoarding = ~(GizwitsST.OpenOnBoarding);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s OnBoarding Mode\r\n", GizwitsST.OpenOnBoarding ? "Enter" : "Exit");
        if (GizwitsST.OpenOnBoarding)
            printf("    %s Mode\r\n", GizwitsST.OpenSoftAP == true ? "SoftAP" : "AirLink");
#endif
    }
    GizwitsReceiveBuffer[1] >>= 1;
    if (GizwitsST.OpenBindMode ^ (GizwitsReceiveBuffer[1] & 0x01)) //绑定模式发生变化
    {
        GizwitsST.OpenBindMode = ~(GizwitsST.OpenBindMode);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s BindMode\r\n", GizwitsST.OpenBindMode == true ? "Enter" : "Exit");
#endif
    }
    GizwitsReceiveBuffer[1] >>= 1;
    if (GizwitsST.ConnectRoute ^ (GizwitsReceiveBuffer[1] & 0x01)) //与无线路由器连接发生变化
    {
        GizwitsST.ConnectRoute = ~(GizwitsST.ConnectRoute);
#if LOGRANK_UART1 >= 3
        printf("LOG:WireLess Route %s\r\n", GizwitsST.ConnectRoute == true ? "Connect" : "Disconnect");
#endif
    }
    GizwitsReceiveBuffer[1] >>= 1;
    if (GizwitsST.ConnectM2M ^ (GizwitsReceiveBuffer[1] & 0x01)) //与M2M服务器连接发生变化
    {
        GizwitsST.ConnectM2M = ~(GizwitsST.ConnectM2M);
#if LOGRANK_UART1 >= 3
        printf("LOG:M2M server %s\r\n", GizwitsST.ConnectM2M == true ? "Connect" : "Disconnect");
#endif
    }
    GizwitsST.RouteRSSI = GizwitsReceiveBuffer[0] & 0x07; //更新无线路由器信号强度
#if LOGRANK_UART1 >= 3
    if (GizwitsST.ConnectRoute)
        printf("LOG:Route RSSI-%bu\r\n", GizwitsST.RouteRSSI);
#endif
    if (GizwitsST.AppOnline ^ (GizwitsReceiveBuffer[0] & 0x08) >> 3) //App在线状态发生变化
    {
        GizwitsST.AppOnline = ~(GizwitsST.AppOnline);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s app online now\r\n", GizwitsST.AppOnline == true ? "Have" : "None");
#endif
    }
    if (GizwitsST.ProductTestMode ^ (GizwitsReceiveBuffer[0] & 0x10) >> 4) //产品测试模式发生变化
    {
        GizwitsST.ProductTestMode = ~(GizwitsST.ProductTestMode);
#if LOGRANK_UART1 >= 3
        printf("LOG:%s product test\r\n", GizwitsST.ProductTestMode == true ? "Enter" : "Exit");
#endif
    }
}
//属于GizwitsHandleWiFiAns的子函数,Cmd=0x17且使用WiFi的RealTime信息时提取数据
static void GizwitsWiFiAns0x17(void)
{
    GizwitsRealTime.Year = GizwitsReceiveBuffer[0];
    GizwitsRealTime.Year <<= 8;
    GizwitsRealTime.Year |= GizwitsReceiveBuffer[1];
    GizwitsRealTime.Month = GizwitsReceiveBuffer[2];
    GizwitsRealTime.Day = GizwitsReceiveBuffer[3];
    GizwitsRealTime.Hour = GizwitsReceiveBuffer[4];
    GizwitsRealTime.Minute = GizwitsReceiveBuffer[5];
    GizwitsRealTime.Second = GizwitsReceiveBuffer[6];
    GizwitsRealTime.NTP = GizwitsReceiveBuffer[7];
    GizwitsRealTime.NTP <<= 8;
    GizwitsRealTime.NTP |= GizwitsReceiveBuffer[8];
    GizwitsRealTime.NTP <<= 8;
    GizwitsRealTime.NTP |= GizwitsReceiveBuffer[9];
    GizwitsRealTime.NTP <<= 8;
    GizwitsRealTime.NTP |= GizwitsReceiveBuffer[10];
#if LOGRANK_UART1 >= 3
    printf("LOG:Get Real Time,%04u.%02bu.%02bu,%02bu:%02bu:%02bu,NTP:%lu\r\n",
           GizwitsRealTime.Year, GizwitsRealTime.Month, GizwitsRealTime.Day,
           GizwitsRealTime.Hour, GizwitsRealTime.Minute, GizwitsRealTime.Second, GizwitsRealTime.NTP);
#endif
}
//属于GizwitsHandleWiFiAns的子函数,Cmd=0x21且使用WiFi的设备信息时提取数据
static void GizwitsWiFiAns0x21(void)
{
    pdata uchar i, j = 0;
    GizwitsWiFiProperty.ModuleType = GizwitsReceiveBuffer[j++];
    for (i = 0; i != 8; ++i)
        GizwitsWiFiProperty.SerialProtocolVer[i] = GizwitsReceiveBuffer[j++];
    for (i = 0; i != 8; ++i)
        GizwitsWiFiProperty.HardwareVer[i] = GizwitsReceiveBuffer[j++];
    for (i = 0; i != 8; ++i)
        GizwitsWiFiProperty.SoftwareVer[i] = GizwitsReceiveBuffer[j++];
    for (i = 0; i == 0 || GizwitsReceiveBuffer[i - 1] != '\0'; ++i)
        GizwitsWiFiProperty.Mac[i] = GizwitsReceiveBuffer[j++];
    for (i = 0; i == 0 || GizwitsReceiveBuffer[i - 1] != '\0'; ++i)
        GizwitsWiFiProperty.Ip[i] = GizwitsReceiveBuffer[j++];
    for (i = 0; i != 8; ++i)
        GizwitsWiFiProperty.Property[i] = GizwitsReceiveBuffer[j++];
#if LOGRANK_UART1 >= 3
    printf("LOG:Get WiFi Property@\r\nModuleType:%02bu\r\n", GizwitsWiFiProperty.ModuleType);
    printf("SerialProtocolVer:%.8s\r\n", GizwitsWiFiProperty.SerialProtocolVer);
    printf("HardwareVer:%.8s\r\n", GizwitsWiFiProperty.HardwareVer);
    printf("SoftwareVer:%.8s\r\n", GizwitsWiFiProperty.SoftwareVer);
    printf("Mac:%s\r\n", GizwitsWiFiProperty.Mac);
    printf("Ip:%s\r\n", GizwitsWiFiProperty.Ip);
    printf("Property:");
    for (i = 0; i != 8; ++i)
        printf("%02bx", GizwitsWiFiProperty.Property[i]);
    printf("\r\n");
#endif
}
//属于GizwitsHandleMCUAsk的子函数,Cmd=0x09且GizwitsConfigMode=0x04时写入SendBuffer中WiFi配置信息
static void GizwitsMCUAsk0x09(void)
{
    pdata uchar i8, j8;
    pdata ulong i32, j32;
    j8 = GizwitsSendBuffer[GizwitsSendIdx++] = strlen(ConfigMode4Ssid);
    for (i8 = 0; i8 != j8; ++i8)
        GizwitsSendBuffer[GizwitsSendIdx++] = ConfigMode4Ssid[i8];
    j8 = GizwitsSendBuffer[GizwitsSendIdx++] = strlen(ConfigMode4Password);
    for (i8 = 0; i8 != j8; ++i8)
        GizwitsSendBuffer[GizwitsSendIdx++] = ConfigMode4Password[i8];
    j8 = GizwitsSendBuffer[GizwitsSendIdx++] = strlen(ConfigMode4Bssid);
    for (i8 = 0; i8 != j8; ++i8)
        GizwitsSendBuffer[GizwitsSendIdx++] = ConfigMode4Bssid[i8];
    j32 = GizwitsSendBuffer[GizwitsSendIdx++] = strlen(ConfigMode4Tz);
    for (i32 = 0; i32 != j32; ++i32)
        GizwitsSendBuffer[GizwitsSendIdx++] = ConfigMode4Tz[i32];
    j8 = GizwitsSendBuffer[GizwitsSendIdx++] = strlen(ConfigMode4ServerName);
    for (i8 = 0; i8 != j8; ++i8)
        GizwitsSendBuffer[GizwitsSendIdx++] = ConfigMode4ServerName[i8];
}
//------------------------------------------------------------------------------------------------//