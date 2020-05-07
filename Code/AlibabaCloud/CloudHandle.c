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
		CloudReSend(6); //##等待回复600ms超时,重新发送命令
		//------#汇报事件------//
		if (CloudAct.NeedReport_Event_1 == true && CloudReport(1) == EXIT_SUCCESS)
			CloudAct.NeedReport_Event_1 = false;
		if (CloudAct.NeedReport_Event_2 == true && CloudReport(2) == EXIT_SUCCESS)
			CloudAct.NeedReport_Event_2 = false;
		//------#汇报参数------//
		if (CloudAct.NeedReport == true && CloudReport(0) == EXIT_SUCCESS)
			CloudAct.NeedReport = false,memset(&CloudAct.NeedReportT,0,sizeof(CloudAct.NeedReportT));
		//------#应答服务------//
		if (CloudAct.NeedReport_Service1 == true && CloudReport(200) == EXIT_SUCCESS)
			CloudAct.NeedReport_Service1 = false;
		if (CloudAct.NeedReport_Service2 == true && CloudReport(201) == EXIT_SUCCESS)
			CloudAct.NeedReport_Service2 = false;
		if (CloudAct.NeedReport_Service3 == true && CloudReport(202) == EXIT_SUCCESS)
			CloudAct.NeedReport_Service3 = false;
	}
	//--------------------------------执行服务类任务-------------------------------//
	if(CloudAct.ServiceMCUOffline == true||CloudAct.ServiceMCURst == true)
	{
		static bool issend = false;
		if(CloudAct.NeedAns==false && issend == false)//*不在等待应答状态*,且执行一次
		{
			CloudAct.Cmd = AT_MQTTCLOSE;
			if(CloudSend(2)==EXIT_SUCCESS)
				issend = true;
		}
		if(CloudAct.DisConectWiFi == true)//已离线
		{
			CloudAct.ServiceMCUOffline=false;//完成MCU离线任务
			if(CloudAct.ServiceMCURst == true)//执行重启MCU任务
			{
#if LOGRANK_UART1 >= 2
	printf("LOG#:CloudLoop Service:MCURST!\r\n"); //日志记录将会重启MCU
#endif
				MCURST();
			}
		}
	}
	//--------------------------------子设备类任务---------------------------------//
	//------传感器读数------//
    if (CloudAct.NeedReadDS18B20 == true 
		&& CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode]
		&& DS18B20GetTemperature() == EXIT_SUCCESS)//需要读取DS18B20温度值
		CloudAct.NeedReadDS18B20 = false,CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
	if(CloudAct.NeedReadGY_25 == true && GY_25GetAzimuth() == EXIT_SUCCESS)//已发送查询指令,开始读取方位角数据
		CloudAct.NeedReadGY_25 = false, CloudAct.NeedReadGY_25_Ms = CloudAct.SysTime;//读取成功
	//------传感器准备------//
    if (CloudAct.NeedReadDS18B20 == false && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20NeedReadMs 
		&& DS18B20ConvertTemperature() == EXIT_SUCCESS) //需要开始温度转换
        CloudAct.NeedReadDS18B20 = true, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
	if(CloudAct.NeedReadGY_25 == false && CloudAct.SysTime - CloudAct.NeedReadGY_25_Ms >= GY_25NeedReadMs 
		&& GY_25SetCmd(GY_25Query) == EXIT_SUCCESS)//发送查询指令
		CloudAct.NeedReadGY_25 = true;
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
		CloudReSend(30);//3,000ms等待
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
    CloudAct.Cmd = AT_MQTTSUB; //设置MQTT订阅-服务1,SubCode=2
    CloudAct.SubCode = 2;
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
	CloudAct.SubisBusy|=0x04;       //2号Sub通道已占用
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_SUB_Service1 ok\r\n"); //日志记录MQTT订阅服务1成功
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSUB; //设置MQTT订阅-服务2,SubCode=3
    CloudAct.SubCode = 3;
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
	CloudAct.SubisBusy|=0x08;       //3号Sub通道已占用
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_SUB_Service2 ok\r\n"); //日志记录MQTT订阅服务2成功
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSUB; //设置MQTT订阅-服务3,SubCode=4
    CloudAct.SubCode = 4;
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
	CloudAct.SubisBusy|=0x10;       //4号Sub通道已占用
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_SUB_Service3 ok\r\n"); //日志记录MQTT订阅服务3成功
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
    data ushort idx1 = uart2Idx1, idx2 = uart2Idx2;
    while (idx1 != idx2)
    {
        switch (CloudReceiveState)
        {
        case 0: //等待中
            if (uart2Buffer[idx1] == 0x0A)
                CloudReceiveState = 1; //等待/r
            break;
        case 1:                             //读取中
            if (uart2Buffer[idx1] == 0x0D) //发现\n,结束读取,由CloudHandleReceive函数处理
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
                CloudReceiveBuffer[CloudReceiveIdx++] = uart2Buffer[idx1];
			break;
        }
        if (idx1 + 1 == uart2_buffer_size)
            idx1 = 0;
        else
            ++idx1;
        uart2Idx1 = idx1;
    }
}
static bool CloudSend(uchar op) //发送命令到串口
{
    pdata uchar i = 0;
    if (CloudAct.NeedAns||CloudAct.DisConectWiFi==true) //尚有信息未应答或连接已断开
        return EXIT_FAILURE;
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
		{
			pdata uchar *P;
			switch(CloudAct.SubCode)
			{
			case 2://订阅云端调用设备服务1
				P=Service_1;break;
			case 3://订阅云端调用设备服务2
				P=Service_2;break;
			case 4://订阅云端调用设备服务3
				P=Service_3;break;
			}
			sprintf(CloudSendBuffer + CloudSendIdx, "=" SubscribeSet4 "\r",
				CloudAct.SubCode,ProductKey, DeviceName,P);
		}
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
            case 200://设置响应服务调用1,Service_1:LCD1602Display
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet3 "\r",ProductKey, DeviceName, Service_1);
                break;
			case 201://设置响应服务调用2,Service_2:GY25Correction
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet3 "\r",ProductKey, DeviceName, Service_2);
                break;
			case 202://设置响应服务调用3,Service_3:MCUControl
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet3 "\r",ProductKey, DeviceName, Service_3);
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
			case 200://设置响应服务调用1,Service_1:LCD1602Display
			case 201://设置响应服务调用2,Service_2:GY25Correction
			case 202://设置响应服务调用3,Service_3:MCUControl
				sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet3 "\r",
                        SendSet3Len + CloudAct.SubIdLen + CloudSendDataIdx,
						CloudAct.SubId,CloudAct.NeedReport_ServiceReCode,CloudSendData);
				break;
            }
			if(CloudAct.PubCode<200)//非响应服务调用
				++CloudAct.MQTTSENDid;//本地id号更新
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
    return EXIT_SUCCESS;
}
static void CloudReSend(uchar Time)
{
	xdata ulong Timex=Time;
	Timex*=100;
	if(CloudAct.NeedAns==false || CloudAct.DisConectWiFi==true)//不是等待应答状态或连接已断开
		return;
	if(CloudAct.SysTime-CloudAct.NeedAns_Time>=Timex)//等待应答超时
	{
		if(CloudAct.NeedAns_Count==2)//已经重新发送两次
		{
			CloudAct.NeedAns=false;//放弃重新发送,取消等待应答状态
			++CloudAct.NeedAns_FailCount;
#if LOGRANK_UART1 >= 2
	printf("LOG#:CloudReSend abandon,FailCount:%bu\r\n",CloudAct.NeedAns_FailCount);
#endif
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
        return EXIT_FAILURE;
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
            return EXIT_FAILURE;
        }
		CloudSendDataIdx = 0;
		if(CloudAct.NeedReportT.GY_25)
			CloudSendDataIdx += sprintf(CloudSendData, "\"HeadAngle\":%.2f,\"PitchAngle\":%.2f,\"RollAngle\":%.2f",
			GY_25ST.HeadAngle/100.0,GY_25ST.PitchAngle/100.0,GY_25ST.RollAngle/100.0);
		if(CloudAct.NeedReportT.DS18B20)
		{
			if(CloudSendDataIdx)
				CloudSendData[CloudSendDataIdx++]=',';
			CloudSendDataIdx += sprintf(CloudSendData, "\"WaterTemperature\":%.3f",
                                   (DS18B20ST.TemperatureData) * DS18B20ReTransfrom[DS18B20ST.ResolutionMode] + (float)DS18B20MinT);
		}
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
					return EXIT_FAILURE;//Pub转换失败
			}
        }
        switch (Code)
        {
        case 1://设备事件上报,Event_1:WaterTemperatureLow
            CloudSendDataIdx = sprintf(CloudSendData, "\"Error\":%bu",DS18B20ST.TemperatureLow);break;
        case 2://设备事件上报,Event_2:WaterTemperatureHigh
            CloudSendDataIdx = sprintf(CloudSendData, "\"Error\":%bu",DS18B20ST.TemperatureHigh);break;
		case 200://设置响应服务调用1,Service_1:LCD1602Display
		case 201://设置响应服务调用2,Service_2:GY25Correction
		case 202://设置响应服务调用3,Service_3:MCUControl	
			CloudSendDataIdx = sprintf(CloudSendData, "");//没有返回参数
            break;
        }
    }
    CloudAct.Cmd = AT_MQTTSEND; //已写入设备待上报属性,准备发布
    CloudSend(0);
    return EXIT_SUCCESS;
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
        if (CloudAct.Cmd != AT_MQTTSTART && CloudAct.Cmd != AT_MQTTSEND 
			&& CloudAct.Cmd != AT_MQTTSUB && CloudAct.Cmd != AT_WJAP
			&& CloudAct.Cmd != AT_MQTTCLOSE)
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
			pdata uchar *P=CloudReceiveBuffer+16;//从这里开始查找id和params
			pdata uchar i=0;
			P=strstr(P,"\"id\":\""),P+=6;
			while(*P!='"')  //选用"作为结束标志
				CloudAct.SubId[i++]=*P++;
			CloudAct.SubIdLen=i;
            switch (CloudReceiveBuffer[10]) //判断订阅的通道
            {
            case '0': //云端响应属性上报,云端响应事件上报-常闭通道
                break;
            case '1': //云端设置设备属性
                break;
            case '2': //云端调用设备服务:1,Service_1:LCD1602Display
			{
				pdata uchar *Pend=strstr(P,"},\"version\":\"1.0.0\"}");//找寻末尾}
				bool isRow2;
				pdata uchar str[17]="                ",readcnt=0;//保证16个字符,刷新剩余部分为空白
				P=strstr(P,"\"params\":{"),P+=10;
				while(P!=Pend)
				{
					++P;//跳过"
					if(strncmp(P,"Text",4)==0)
					{
						pdata uchar i=0;
						P+=7;
						while(*P!='"'&&i!=16)
						{
							if(*P=='\\')  //无脑过滤转义字符
								++P;
							str[i++]=*P++;
						}
						while(*P!='"')
							++P;
						++P;//跳过"
					}
					else
					{
						P+=8;
						isRow2=(*P++)-'0';
					}
					++readcnt;//读到一个参数
					if(*P==',')
						++P;
				}
				if(readcnt==2)//参数读到两个,参数完整
				{
					LCD1602WriteLine(str,isRow2);//执行服务调用
					CloudAct.NeedReport_ServiceReCode=200;//请求成功
				}
				else
					CloudAct.NeedReport_ServiceReCode=460;//请求参数错误
				CloudAct.NeedReport_Service1=true;//Service1需要回复
			}
                break;
            case '3': //云端调用设备服务:2,Service_2:GY25Correction
				P=strstr(P,"{\"object\":"),P+=10;
				switch(*P)
				{
				case '1'://校正俯仰角和横滚角
					GY_25SetCmd(GY_25Correction1);break;
				case '2'://校正航向角
					GY_25SetCmd(GY_25Correction2);break;
				}
				CloudAct.NeedReport_ServiceReCode=200;//请求成功
				CloudAct.NeedReport_Service2=true;//Service2需要回复
                break;
            case '4': //云端调用设备服务:3,Service_3:MCUControl
				P=strstr(P,"{\"ID\":"),P+=6;
				switch(*P)
				{
				case '0'://离线
					CloudAct.ServiceMCUOffline=true;break;
				case '1'://离线后重启MCU
					CloudAct.ServiceMCURst=true;break;
				}
				CloudAct.NeedReport_ServiceReCode=200;//请求成功
				CloudAct.NeedReport_Service3=true;//Service3需要回复
                break;
            case '5': //云端调用设备服务
                break;
            case '6': //云端调用设备服务
                break;
            }
        }
        else if (strncmp(CloudReceiveBuffer + 1, "MQTTEVENT", 9) == 0)
        {
            if (CloudAct.Cmd == AT_MQTTSTART && (strncmp(CloudReceiveBuffer + 11, "CONNECT", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功连接
            if (CloudAct.Cmd == AT_MQTTSUB && (strncmp(CloudReceiveBuffer + 13, "SUBSCRIBE", 9) == 0))
                if (strncmp(CloudReceiveBuffer + 23, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功设置订阅
            if (CloudAct.Cmd == AT_MQTTSEND && (strncmp(CloudReceiveBuffer + 11, "PUBLISH", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //成功发布
            if (CloudAct.Cmd == AT_MQTTCLOSE && (strncmp(CloudReceiveBuffer + 11, "CLOSE", 5) == 0))
                if (strncmp(CloudReceiveBuffer + 17, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false,CloudAct.DisConectWiFi=true; //成功断开连接
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