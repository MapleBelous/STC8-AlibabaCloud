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
static bool CloudSend(uchar);      //发送命令到串口
static void CloudReSend(uchar);        //重新发送命令
static bool CloudReport(uchar);  //设备上报
static void CloudHandleReceive(void); //处理CloudReceive收到的一条WiFi信息
//------------------------------------------------------------------------------------------------//
void CloudLoop(void) //Cloud主循环
{
    //-----------------------------WiFi连接类任务任务-----------------------------//
	if(CloudAct.DisConectWiFi==false)//WiFi是否为可用状态
	{
		CloudReceive(); //##接收并处理串口缓冲区数据
		CloudReSend(6); //##等待回复600ms超时,重新发送命令,从此往下都有可能处于WiFi断开连接的状态
		//------DS18B20#汇报高低温报警------//
		if (CloudAct.NeedReport_WaterTemperatureLow == true && CloudReport(1) == 0)
			CloudAct.NeedReport_WaterTemperatureLow = false;
		if (CloudAct.NeedReport_WaterTemperatureHigh == true && CloudReport(2) == 0)
			CloudAct.NeedReport_WaterTemperatureHigh = false;
		//------DS18B20#汇报当前参数------//
		if (CloudAct.NeedReport == true && CloudReport(0) == 0)
			CloudAct.NeedReport = false;
	}
    //-----------------------------传感器类任务-----------------------------//
	//------DS18B20#开始转换温度&读取温度值------//
    if (CloudAct.NeedReadDS18B20 == true && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode] && DS18B20GetTemperature() == EXIT_SUCCESS) //成功执行读取温度值
        CloudAct.NeedReadDS18B20 = false, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
    if (CloudAct.NeedReadDS18B20 == false && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20NeedReadMs && DS18B20ConvertTemperature() == EXIT_SUCCESS) //需要读取DS18B20温度值,且温度转换指令成功
        CloudAct.NeedReadDS18B20 = true, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
}
void CloudInit(void) //初始化Cloud
{
    memset(&CloudAct, 0, sizeof(CloudAct));                //清零任务指示器
    CloudSendDataIdx = CloudSendIdx = CloudReceiveIdx = 0; //各缓冲区下标清零
//------------------------------------------------//
    CloudAct.Cmd = AT_REBOOT; //重启模组
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(20);//2,000ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-Reboot ok\r\n"); //日志记录模块重启完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_WJAPQ; //断开当前WiFi连接
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(100);//10,000ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiDisConect ok\r\n"); //日志记录模块断开当前WiFi连接
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_WJAP; //连接目标WiFi
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(150);//15,000ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiConect ok\r\n"); //日志记录目标WiFi已连接
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTAUTH; //设置MQTT用户名密码
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_UserSet ok\r\n"); //日志记录MQTT用户名和密码设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSOCK; //设置MQTT主机和端口
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HostSet ok\r\n"); //日志记录MQTT主机和端口设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTCID; //设置MQTT客户端ID
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_ClintIDSet ok\r\n"); //日志记录MQTT客户端ID设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTKEEPALIVE; //设置MQTT心跳周期
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HeartSet ok\r\n"); //日志记录MQTT心跳时间设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTRECONN; //设置MQTT是否自动重连
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoReConect Set ok\r\n"); //日志记录MQTT自动重连设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTAUTOSTART; //设置MQTT是否上电自动开启
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoStartSet ok\r\n"); //日志记录MQTT上电自动开启设置完毕
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSTART; //开启MQTT
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(40);//4,000ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_Start ok\r\n"); //日志记录MQTT开启成功
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTPUB; //设置MQTT发布-默认为参数发布PubCode=0
    CloudAct.PubCode_t = 0;
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms等待
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend放弃发送
		{
			CloudAct.DisConectWiFi=true;//初始化失败,放弃连接WiFi模组,转为本地工作模式
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //日志记录Cloud初始化失败
#endif
			return;
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
    data ushort idx1 = uart2_idx1, idx2 = uart2_idx2;
    while (idx1 != idx2)
    {
        switch (CloudReceiveState)
        {
        case 0: //等待中
            if (uart2_buffer[idx1] == 0x0A)
                CloudReceiveState = 1; //等待/r
            break;
        case 1:                             //读取中
            if (uart2_buffer[idx1] == 0x0D) //发现\n,结束读取,由CloudHandleReceive函数处理
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
                CloudReceiveBuffer[CloudReceiveIdx++] = uart2_buffer[idx1];
			break;
        }
        if (idx1 + 1 == uart2_buffer_size)
            idx1 = 0;
        else
            ++idx1;
        uart2_idx1 = idx1;
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
    uart2_sendstr8(CloudSendBuffer);       //发送准备好的命令字符串
    CloudAct.NeedAns = true;                  //开始等待应答
    CloudAct.NeedAns_Time = CloudAct.SysTime; //记录发送命令时间
	CloudAct.NeedAns_Count = 0;
    return 0;
}
static void CloudReSend(uchar Time)
{
	xdata ushort Timex=Time*100;
	if(CloudAct.NeedAns==false)//不是等待应答状态
		return;
	if(CloudAct.SysTime-CloudAct.NeedAns_Time>=Timex)//等待应答超过500ms
	{
		if(CloudAct.NeedAns_Count==2)//已经重新发送两次
		{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend abandon\r\n");
#endif
			CloudAct.NeedAns=false;//放弃重新发送,取消等待应答状态
			++CloudAct.NeedAns_FailCount;
			if(CloudAct.NeedAns_FailCount==3)//失败3次,开始验证模组是否在线
			{
				ushort TempT;
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline?\r\n");
#endif
				CloudAct.Cmd = 0xFF;//防止阻塞ok
				uart2_sendstr8("\rAT\r");//发送AT验证
				TempT = CloudAct.SysTime; //记录命令发送时间
				CloudAct.NeedAns=true;
				while (CloudAct.NeedAns == true)
				{
					CloudReceive();
					if (CloudAct.SysTime - TempT >= 3500) //3500ms没有回复
					{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline - No,DisConect Now\r\n");
#endif
						CloudAct.DisConectWiFi=true;
						IE2 &= ~ES2;//关闭串口2中断
						return;//模组断开连接,不再连接模组
					}
				}
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline - Yes,Restart MCU\r\n");
#endif
				MCURST();//通过验证,重启MCU试图重新匹配模组状态
			}
		}
		else
		{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend$%s,Cnt$%bu\r\n", CloudSendBuffer,CloudAct.NeedAns_Count+1);
#endif
			uart2_sendstr8(CloudSendBuffer);//重新发送
			CloudAct.NeedAns_Time = CloudAct.SysTime;//更新发送时间
			++CloudAct.NeedAns_Count;//计数重新发送次数
		}
	}
}
static bool CloudReport(uchar Code) //设备上报
{
    if (CloudAct.NeedAns||CloudAct.DisConectWiFi==true) //尚有信息未应答,或模组断开状态
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
			pdata uchar TempFailCount = CloudAct.NeedAns_FailCount;
            CloudAct.Cmd = AT_MQTTPUB;
            CloudAct.PubCode_t = Code;       //标志已经开始切换
            CloudSend(0);                    //切换到设备属性Pub
            while (CloudAct.NeedAns == true)
			{
				CloudReceive();
				CloudReSend(5);//500ms等待
				if(CloudAct.NeedAns_FailCount==TempFailCount+1)//CloudReSend放弃发送
					return 1;//Pub转换失败
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