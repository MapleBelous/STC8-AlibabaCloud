#include "CloudHandle.h"
//------------------------------------------------------------------------------------------------//
//Cloud任务#状态指示器
xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
//各函数缓冲区
static xdata uchar CloudReceiveBuffer[CloudReceiveBufferSize]; //串口数据处理缓冲区-CloudReceive函数
static xdata ushort CloudReceiveIdx;                           //串口数据处理缓冲区下标-CloudReceive函数
static data uchar CloudReceiveState = 0; 					   //接收状态-CloudReceive函数
static xdata uchar CloudSendBuffer[CloudSendBufferSize];       //生成AT指令缓冲区-CloudSend函数
static xdata ushort CloudSendIdx;                              //生成AT指令缓冲区下标-CloudSend函数
static xdata uchar CloudSendData[CloudSendDataSize];           //上报设备属性计算缓冲区-CloudReport函数
static xdata ushort CloudSendDataIdx;                          //记录缓冲区写入大小-CloudReport函数
//------------------------------------------------------------------------------------------------//
static void CloudReceive(void);       //接收串口缓冲区数据
static bool CloudSend(uchar op);      //发送命令到串口
static bool CloudReport(uchar Code);  //设备上报
static void CloudHandleReceive(void); //处理CloudReceive收到的一条WiFi信息
//------------------------------------------------------------------------------------------------//
void CloudLoop(void) //Cloud主循环
{
    CloudReceive(); //##接收并处理串口缓冲区数据
    //注册任务
    if (CloudAct.NeedReadDS18B20 == false && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20NeedReadMs && DS18B20ConvertTemperature() == EXIT_SUCCESS) //需要读取DS18B20温度值,且温度转换指令成功
        CloudAct.NeedReadDS18B20 = true, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;                                                                         //等待读取温度值
                                                                                                                                                                 //待执行任务
    if (CloudAct.NeedReport_WaterTemperatureLow == true && CloudReport(1) == 0)
        CloudAct.NeedReport_WaterTemperatureLow = false;
    if (CloudAct.NeedReport_WaterTemperatureHigh == true && CloudReport(2) == 0)
        CloudAct.NeedReport_WaterTemperatureHigh = false;
    if (CloudAct.NeedReport == true && CloudReport(0) == 0)
        CloudAct.NeedReport = false;

    if (CloudAct.NeedReadDS18B20 == true && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode] && DS18B20GetTemperature() == EXIT_SUCCESS) //成功执行读取温度值
        CloudAct.NeedReadDS18B20 = false, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
}
void CloudInit(void) //初始化Cloud
{
    xdata ulong TempT;
    memset(&CloudAct, 0, sizeof(CloudAct));                //清零任务指示器
    CloudSendDataIdx = CloudSendIdx = CloudReceiveIdx = 0; //各缓冲区下标清零
//------------------------------------------------//
T0:
    CloudAct.Cmd = AT_REBOOT; //重启模组
    CloudSend(2);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T0;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-Reboot ok\r\n"); //日志记录模块重启完毕
#endif
//------------------------------------------------//
T0x:
    CloudAct.Cmd = AT_WJAPQ; //断开当前WiFi连接
    CloudSend(2);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 3000) //3000ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T0x;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiDisConect ok\r\n"); //日志记录模块断开当前WiFi连接
#endif
//------------------------------------------------//
T1:
    CloudAct.Cmd = AT_WJAP; //连接目标WiFi
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >=10000) //10000ms没有完成命令,WiFI信号弱时可能连接很慢
        {
			CloudAct.NeedAns=false;
			goto T1;//重发指令
		}	
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiConect ok\r\n"); //日志记录目标WiFi已连接
#endif
//------------------------------------------------//
T2:
    CloudAct.Cmd = AT_MQTTAUTH; //设置MQTT用户名密码
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T2;//重发指令
		}	
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_UserSet ok\r\n"); //日志记录MQTT用户名和密码设置完毕
#endif
//------------------------------------------------//
T3:
    CloudAct.Cmd = AT_MQTTSOCK; //设置MQTT主机和端口
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T3;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HostSet ok\r\n"); //日志记录MQTT主机和端口设置完毕
#endif
//------------------------------------------------//
T4:
    CloudAct.Cmd = AT_MQTTCID; //设置MQTT客户端ID
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T4;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_ClintIDSet ok\r\n"); //日志记录MQTT客户端ID设置完毕
#endif
//------------------------------------------------//
T5:
    CloudAct.Cmd = AT_MQTTKEEPALIVE; //设置MQTT心跳周期
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T5;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HeartSet ok\r\n"); //日志记录MQTT心跳时间设置完毕
#endif
//------------------------------------------------//
T6:
    CloudAct.Cmd = AT_MQTTRECONN; //设置MQTT是否自动重连
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T6;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoReConect Set ok\r\n"); //日志记录MQTT自动重连设置完毕
#endif
//------------------------------------------------//
T7:
    CloudAct.Cmd = AT_MQTTAUTOSTART; //设置MQTT是否上电自动开启
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T7;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoStartSet ok\r\n"); //日志记录MQTT上电自动开启设置完毕
#endif
//------------------------------------------------//
T8:
    CloudAct.Cmd = AT_MQTTSTART; //开启MQTT
    CloudSend(2);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T8;//重发指令
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_Start ok\r\n"); //日志记录MQTT开启成功
#endif
//------------------------------------------------//
T9:
    CloudAct.Cmd = AT_MQTTPUB; //设置MQTT发布-默认为参数发布PubCode=0
    CloudAct.PubCode_t = 0;
    CloudSend(0);
    TempT = CloudAct.SysTime; //记录命令发送时间
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500ms没有完成命令
        {
			CloudAct.NeedAns=false;
			goto T9;//重发指令
		}
    }
    //------------------------------------------------//
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ok\r\n"); //日志记录Cloud初始化完毕
#endif
}
//------------------------------------------------------------------------------------------------//
static void CloudReceive(void) //接收串口缓冲区数据
{
    data ushort idx1 = uart4_idx1, idx2 = uart4_idx2;
    while (idx1 != idx2)
    {
        switch (CloudReceiveState)
        {
        case 0: //等待中
            if (uart4_buffer[idx1] == 0x0A)
                CloudReceiveState = 1; //等待/r
            break;
        case 1:                             //读取中
            if (uart4_buffer[idx1] == 0x0D) //发现\n,结束读取,由CloudHandleReceive函数处理
            {
                if (CloudReceiveIdx >= 2)                      //命令空隙,回到State=1;
                {                                              //###此处决定过滤长度不足2的命令,主要针对MQTTSEND的>
                    CloudReceiveBuffer[CloudReceiveIdx++] = 0; //放置字符串末尾,方便处理
                    CloudHandleReceive();                      //处理收到的信息
                }
                CloudReceiveState = 0; //开始新的接收
                CloudReceiveIdx = 0;
                CloudReceiveBuffer[0] = 0;
            }
            else
                CloudReceiveBuffer[CloudReceiveIdx++] = uart4_buffer[idx1];
			break;
        }
        if (idx1 + 1 == uart4_buffer_size)
            idx1 = 0;
        else
            ++idx1;
        uart4_idx1 = idx1;
    }
}
static bool CloudSend(uchar op) //发送命令到串口
{
    pdata uchar i = 0;
    if (CloudAct.NeedAns) //尚有信息未应答
        return 1;
    CloudSendIdx = sprintf(CloudSendBuffer, "AT+%s", ATCmd[CloudAct.Cmd]);
    switch (op)
    {
    case 0: //设置参数
        switch (CloudAct.Cmd)
        {
        case AT_MQTTAUTH:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s,%s\r", MQTTUser, MQTTPassword);
            break;
        case AT_MQTTSOCK:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s,%s\r", MQTTHost, MQTTPort);
            break;
        case AT_MQTTCID:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s\r", MQTTClientID);
            break;
        case AT_MQTTKEEPALIVE:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s\r", MQTTHeartBeat);
            break;
        case AT_MQTTRECONN:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s\r", MQTTAutoReConect);
            break;
        case AT_MQTTAUTOSTART:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s\r", MQTTAutoStart);
            break;
        case AT_MQTTSUB:
            break;
        case AT_MQTTPUB:
            switch (CloudAct.PubCode_t) //目标Pub
            {
            case 0: //设备属性上报
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet1 "\r",ProductKey, DeviceName);
                break;
            case 1: //设备事件上报,Event_1:WaterTemperatureLow
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet2 "\r",ProductKey, DeviceName, Event_1);
                break;
            case 2: //设备事件上报,Event_2:WaterTemperatureHigh
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet2 "\r",ProductKey, DeviceName, Event_2);
                break;
            case 200:
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet3 "\r",ProductKey, DeviceName, Service_1);
                break;
            }
            break;
        case AT_MQTTSEND:
            switch (CloudAct.PubCode) //当前Pub
            {
            case 0: //设备属性上报
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet1 "\r",
                        SendSet1Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData);
                break;
            case 1: //设备事件上报,Event_1:WaterTemperatureLow
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet2 "\r",
                        SendSet2Len + Event_1_Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData, Event_1);
                break;
            case 2: //设备事件上报,Event_2:WaterTemperatureHigh
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet2 "\r",
                        SendSet2Len + Event_2_Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData, Event_2);
                break;
            }
            ++CloudAct.MQTTSENDid;
            break; //MQTTSEND
        case AT_WJAP:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s,%s\r", WiFiSSID, WiFiPassword);
            break;
        default:
            break;
        }
        break;
    case 3:
        CloudSendBuffer[CloudSendIdx++] = '='; //查询用户设置的参数
    case 1:
        CloudSendBuffer[CloudSendIdx++] = '?'; //查询系统参数
    case 2:
        CloudSendBuffer[CloudSendIdx++] = '\r'; //执行命令
        CloudSendBuffer[CloudSendIdx++] = 0;
        break;
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudSend$%s\r\n", CloudSendBuffer);
#endif
    uart4_sendstr8(CloudSendBuffer, 0);       //发送准备好的命令字符串
    CloudAct.NeedAns = true;                  //开始等待应答
    CloudAct.NeedAns_Time = CloudAct.SysTime; //记录发送命令时间
    return 0;
}
static bool CloudReport(uchar Code) //设备上报
{
    if (CloudAct.NeedAns) //尚有信息未应答
        return 1;
    if (Code == 0)
    {
        if (CloudAct.PubCode != 0) //Pub未正确切换,切换后返回
        {
            if (CloudAct.PubCode_t != 0) //Pub未开始开始切换到Pub=0
            {
                CloudAct.Cmd = AT_MQTTPUB;
                CloudAct.PubCode_t = 0; //标志已经开始切换
                CloudSend(0);           //切换到设备属性Pub
            }
            return 1;
        }
        CloudSendDataIdx = sprintf(CloudSendData, "\"WaterTemperature\":%.3f",
                                   (DS18B20ST.TemperatureData) * DS18B20ReTransfrom[DS18B20ST.ResolutionMode] + (float)DS18B20MinT);
    }
    else
    {
        if (CloudAct.PubCode != Code) //Pub未正确切换
        {
            xdata ulong TempT;
        TX:
            CloudAct.Cmd = AT_MQTTPUB;
            CloudAct.PubCode_t = Code;       //标志已经开始切换
            CloudSend(0);                    //切换到设备属性Pub
            TempT = CloudAct.SysTime;        //记录命令发送时间
            while (CloudAct.PubCode != Code) //等待,直到Pub正确
            {
                CloudReceive();
                if (CloudAct.SysTime - TempT >= 300) //300ms没有完成命令
				{
					CloudAct.NeedAns=false;
					goto TX;//重发指令
				}
            }
        }
        switch (Code)
        {
        case 1:
            CloudSendDataIdx = sprintf(CloudSendData, "\"Error\":%bu",DS18B20ST.TemperatureLow);
            break;
        case 2:
            CloudSendDataIdx = sprintf(CloudSendData, "\"Error\":%bu",DS18B20ST.TemperatureHigh);
            break;
        }
    }
    CloudAct.Cmd = AT_MQTTSEND; //已写入设备待上报属性,准备发布
    CloudSend(0);
    return 0;
}
static void CloudHandleReceive(void)
{
#if LOGRANK_UART1 >= 3
    printf("LOG:HandleReceive$%s\r\n", CloudReceiveBuffer);
#endif
    if (strncmp(CloudReceiveBuffer, "ERROR", 5) == 0) //命令执行异常
    {
        //目前不处理,待完善
    }
    else if (strncmp(CloudReceiveBuffer, "OK", 2) == 0) //命令执行正常
    {
        if (CloudAct.Cmd != AT_MQTTSTART && CloudAct.Cmd != AT_MQTTSEND && CloudAct.Cmd != AT_MQTTSUB && CloudAct.Cmd != AT_WJAP)
        {
            CloudAct.NeedAns = false;
            if (CloudAct.Cmd == AT_MQTTPUB)
                CloudAct.PubCode = CloudAct.PubCode_t; //切换Pub完毕
        }
    }
    else if (CloudReceiveBuffer[0] == '+') //读取详细信息
    {
        if (strncmp(CloudReceiveBuffer + 1, "MQTTRECV", 8) == 0) //收到订阅信息
        {
            switch (CloudReceiveBuffer[10]) //判断订阅的通道
            {
            case '0': //属性设置通道
                break;
            case '1': //设备服务调用
                break;
            case '3': //云端响应属性上报
                break;
            case '4': //云端响应事件上报
                break;
            }
        }
        else if (strncmp(CloudReceiveBuffer + 1, "MQTTEVENT", 9) == 0)
        {
            if (CloudAct.Cmd == AT_MQTTSTART && (strncmp(CloudReceiveBuffer + 11, "CONNECT", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功连接
            if (CloudAct.Cmd == AT_MQTTSUB && (strncmp(CloudReceiveBuffer + 11, "SUBSCRIBE", 9) == 0))
                if (strncmp(CloudReceiveBuffer + 21, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功设置订阅
            if (CloudAct.Cmd == AT_MQTTSEND && (strncmp(CloudReceiveBuffer + 11, "PUBLISH", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功发布
        }
        else if (strncmp(CloudReceiveBuffer + 1, "WEVENT", 6) == 0)
        {
            if (CloudAct.Cmd == AT_WJAP && (strncmp(CloudReceiveBuffer + 8, "STATION_UP", 10) == 0))
                CloudAct.NeedAns = false; //成功连接
        }
    }
    else //读取附加信息
    {
        //目前用不到
    }
}
//------------------------------------------------------------------------------------------------//