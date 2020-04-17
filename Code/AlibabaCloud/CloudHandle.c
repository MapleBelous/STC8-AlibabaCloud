#include "CloudHandle.h"
//------------------------------------------------------------------------------------------------//
//Cloud����#״ָ̬ʾ��
xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
//������������
static xdata uchar CloudReceiveBuffer[CloudReceiveBufferSize]; //�������ݴ�������-CloudReceive����
static xdata ushort CloudReceiveIdx;                           //�������ݴ��������±�-CloudReceive����
static data uchar CloudReceiveState = 0; 					   //����״̬-CloudReceive����
static xdata uchar CloudSendBuffer[CloudSendBufferSize];       //����ATָ�����-CloudSend����
static xdata ushort CloudSendIdx;                              //����ATָ������±�-CloudSend����
static xdata uchar CloudSendData[CloudSendDataSize];           //�ϱ��豸���Լ��㻺����-CloudReport����
static xdata ushort CloudSendDataIdx;                          //��¼������д���С-CloudReport����
//------------------------------------------------------------------------------------------------//
static void CloudReceive(void);       //���մ��ڻ���������
static bool CloudSend(uchar);      //�����������
static void CloudReSend(uchar);        //���·�������
static bool CloudReport(uchar);  //�豸�ϱ�
static void CloudHandleReceive(void); //����CloudReceive�յ���һ��WiFi��Ϣ
//------------------------------------------------------------------------------------------------//
void CloudLoop(void) //Cloud��ѭ��
{
    //-----------------------------WiFi��������������-----------------------------//
	if(CloudAct.DisConectWiFi==false)//WiFi�Ƿ�Ϊ����״̬
	{
		CloudReceive(); //##���ղ������ڻ���������
		CloudReSend(6); //##�ȴ��ظ�600ms��ʱ,���·�������,�Ӵ����¶��п��ܴ���WiFi�Ͽ����ӵ�״̬
		//------#�㱨�¼�------//
		if (CloudAct.NeedReport_Event_1 == true && CloudReport(1) == 0)
			CloudAct.NeedReport_Event_1 = false;
		if (CloudAct.NeedReport_Event_2 == true && CloudReport(2) == 0)
			CloudAct.NeedReport_Event_2 = false;
		//------#�㱨����------//
		if (CloudAct.NeedReport == true && CloudReport(0) == 0)
			CloudAct.NeedReport = false;
		//------#Ӧ�����------//
		if (CloudAct.NeedReport_Service1 == true && CloudReport(200) == 0)
			CloudAct.NeedReport_Service1 = false;
	}
	//--------------------------------���豸������---------------------------------//
	//------DS18B20#��ʼת���¶�&��ȡ�¶�ֵ------//
    if (CloudAct.NeedReadDS18B20 == true && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode] && DS18B20GetTemperature() == EXIT_SUCCESS) //�ɹ�ִ�ж�ȡ�¶�ֵ
        CloudAct.NeedReadDS18B20 = false, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
    if (CloudAct.NeedReadDS18B20 == false && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20NeedReadMs && DS18B20ConvertTemperature() == EXIT_SUCCESS) //��Ҫ��ȡDS18B20�¶�ֵ,���¶�ת��ָ��ɹ�
        CloudAct.NeedReadDS18B20 = true, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
}
void CloudInit(void) //��ʼ��Cloud
{
    memset(&CloudAct, 0, sizeof(CloudAct));                //��������ָʾ��
    CloudSendDataIdx = CloudSendIdx = CloudReceiveIdx = 0; //���������±�����
//------------------------------------------------//
    CloudAct.Cmd = AT_REBOOT; //����ģ��
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(20);//2,000ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-Reboot ok\r\n"); //��־��¼ģ���������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_WJAPQ; //�Ͽ���ǰWiFi����
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(30);//3,000ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiDisConect ok\r\n"); //��־��¼ģ��Ͽ���ǰWiFi����
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_WJAP; //����Ŀ��WiFi
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(150);//15,000ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiConect ok\r\n"); //��־��¼Ŀ��WiFi������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTAUTH; //����MQTT�û�������
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_UserSet ok\r\n"); //��־��¼MQTT�û����������������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSOCK; //����MQTT�����Ͷ˿�
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HostSet ok\r\n"); //��־��¼MQTT�����Ͷ˿��������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTCID; //����MQTT�ͻ���ID
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_ClintIDSet ok\r\n"); //��־��¼MQTT�ͻ���ID�������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTKEEPALIVE; //����MQTT��������
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HeartSet ok\r\n"); //��־��¼MQTT����ʱ���������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTRECONN; //����MQTT�Ƿ��Զ�����
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoReConect Set ok\r\n"); //��־��¼MQTT�Զ������������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTAUTOSTART; //����MQTT�Ƿ��ϵ��Զ�����
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoStartSet ok\r\n"); //��־��¼MQTT�ϵ��Զ������������
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSTART; //����MQTT
    CloudSend(2);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(40);//4,000ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_Start ok\r\n"); //��־��¼MQTT�����ɹ�
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTSUB; //����MQTT����-����1,SubCode=2
    CloudAct.SubCode = 2;
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
	CloudAct.SubisBusy|=0x04;       //2��Subͨ����ռ��
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_SUB_Service1 ok\r\n"); //��־��¼MQTT���ķ���1�ɹ�
#endif
//------------------------------------------------//
    CloudAct.Cmd = AT_MQTTPUB; //����MQTT����-Ĭ��Ϊ��������PubCode=0
    CloudAct.PubCode_t = 0;
    CloudSend(0);
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
		CloudReSend(22);//2,200ms�ȴ�
		if(CloudAct.NeedAns_FailCount==1)//CloudReSend��������
		{
			CloudAct.DisConectWiFi=true;//��ʼ��ʧ��,��������WiFiģ��,תΪ���ع���ģʽ
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ##[Fail]##\r\n"); //��־��¼Cloud��ʼ��ʧ��
#endif
			return;
		}
    }
    //------------------------------------------------//
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit ok\r\n"); //��־��¼Cloud��ʼ�����
#endif
}
//------------------------------------------------------------------------------------------------//
static void CloudReceive(void) //���մ��ڻ���������
{
    data ushort idx1 = uart2_idx1, idx2 = uart2_idx2;
    while (idx1 != idx2)
    {
        switch (CloudReceiveState)
        {
        case 0: //�ȴ���
            if (uart2_buffer[idx1] == 0x0A)
                CloudReceiveState = 1; //�ȴ�/r
            break;
        case 1:                             //��ȡ��
            if (uart2_buffer[idx1] == 0x0D) //����\n,������ȡ,��CloudHandleReceive��������
            {
                if (CloudReceiveIdx >= 2)                      //�����϶,�ص�State=1;
                {                                              //###�˴��������˳��Ȳ���2������,��Ҫ���MQTTSEND��>
                    CloudReceiveBuffer[CloudReceiveIdx++] = 0; //�����ַ���ĩβ,���㴦��
                    CloudHandleReceive();                      //�����յ�����Ϣ
                }
                CloudReceiveState = 0; //��ʼ�µĽ���
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
static bool CloudSend(uchar op) //�����������
{
    pdata uchar i = 0;
    if (CloudAct.NeedAns) //������ϢδӦ��
        return 1;
    CloudSendIdx = sprintf(CloudSendBuffer, "AT+%s", ATCmd[CloudAct.Cmd]);
    switch (op)
    {
    case 0: //���ò���
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
			switch(CloudAct.SubCode)
			{
			case 2://�����ƶ˵����豸����1
				sprintf(CloudSendBuffer + CloudSendIdx, "=" SubscribeSet4 "\r",
						CloudAct.SubCode,ProductKey, DeviceName,Service_1);
				break;
			}
            break;
        case AT_MQTTPUB:
            switch (CloudAct.PubCode_t) //Ŀ��Pub
            {
            case 0: //�豸�����ϱ�
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet1 "\r",ProductKey, DeviceName);
                break;
            case 1: //�豸�¼��ϱ�,Event_1:WaterTemperatureLow
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet2 "\r",ProductKey, DeviceName, Event_1);
                break;
            case 2: //�豸�¼��ϱ�,Event_2:WaterTemperatureHigh
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet2 "\r",ProductKey, DeviceName, Event_2);
                break;
            case 200://������Ӧ�������1,Service_1:LCD1602Display
                sprintf(CloudSendBuffer + CloudSendIdx, "=" PublishSet3 "\r",ProductKey, DeviceName, Service_1);
                break;
            }
            break;
        case AT_MQTTSEND:
            switch (CloudAct.PubCode) //��ǰPub
            {
            case 0: //�豸�����ϱ�
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet1 "\r",
                        SendSet1Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData);
                break;
            case 1: //�豸�¼��ϱ�,Event_1:WaterTemperatureLow
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet2 "\r",
                        SendSet2Len + Event_1_Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData, Event_1);
                break;
            case 2: //�豸�¼��ϱ�,Event_2:WaterTemperatureHigh
                sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet2 "\r",
                        SendSet2Len + Event_2_Len + CloudSendDataIdx, CloudAct.MQTTSENDid, CloudSendData, Event_2);
                break;
			case 200://������Ӧ�������1,Service_1:LCD1602Display
				sprintf(CloudSendBuffer + CloudSendIdx, "=%u\r" SendSet3 "\r",
                        SendSet3Len + CloudAct.SubIdLen + CloudSendDataIdx,
						CloudAct.SubId,CloudAct.NeedReport_ServiceReCode,CloudSendData);
				break;
            }
			if(CloudAct.PubCode<200)//����Ӧ�������
				++CloudAct.MQTTSENDid;//����id�Ÿ���
            break; //MQTTSEND
        case AT_WJAP:
            sprintf(CloudSendBuffer + CloudSendIdx, "=%s,%s\r", WiFiSSID, WiFiPassword);
            break;
        default:
            break;
        }
        break;
    case 3:
        CloudSendBuffer[CloudSendIdx++] = '='; //��ѯ�û����õĲ���
    case 1:
        CloudSendBuffer[CloudSendIdx++] = '?'; //��ѯϵͳ����
    case 2:
        CloudSendBuffer[CloudSendIdx++] = '\r'; //ִ������
        CloudSendBuffer[CloudSendIdx++] = 0;
        break;
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudSend$%s\r\n", CloudSendBuffer);
#endif
    uart2_sendstr8(CloudSendBuffer);       //����׼���õ������ַ���
    CloudAct.NeedAns = true;                  //��ʼ�ȴ�Ӧ��
    CloudAct.NeedAns_Time = CloudAct.SysTime; //��¼��������ʱ��
	CloudAct.NeedAns_Count = 0;
    return 0;
}
static void CloudReSend(uchar Time)
{
	xdata ulong Timex=Time;
	Timex*=100;
	if(CloudAct.NeedAns==false)//���ǵȴ�Ӧ��״̬
		return;
	if(CloudAct.SysTime-CloudAct.NeedAns_Time>=Timex)//�ȴ�Ӧ��ʱ
	{
		if(CloudAct.NeedAns_Count==2)//�Ѿ����·�������
		{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend abandon\r\n");
#endif
			CloudAct.NeedAns=false;//�������·���,ȡ���ȴ�Ӧ��״̬
			++CloudAct.NeedAns_FailCount;
			if(CloudAct.NeedAns_FailCount==3)//ʧ��3��,��ʼ��֤ģ���Ƿ�����
			{
				ushort TempT;
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline?\r\n");
#endif
				CloudAct.Cmd = 0xFF;//��ֹ����ok
				uart2_sendstr8("\rAT\r");//����AT��֤
				TempT = CloudAct.SysTime; //��¼�����ʱ��
				CloudAct.NeedAns=true;
				while (CloudAct.NeedAns == true)
				{
					CloudReceive();
					if (CloudAct.SysTime - TempT >= 3500) //3500msû�лظ�
					{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline - No,DisConect Now\r\n");
#endif
						CloudAct.DisConectWiFi=true;
						IE2 &= ~ES2;//�رմ���2�ж�
						return;//ģ��Ͽ�����,��������ģ��
					}
				}
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend check WiFi isOnline - Yes,Restart MCU\r\n");
#endif
				MCURST();//ͨ����֤,����MCU��ͼ����ƥ��ģ��״̬
			}
		}
		else
		{
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudReSend$%s,Cnt$%bu\r\n", CloudSendBuffer,CloudAct.NeedAns_Count+1);
#endif
			uart2_sendstr8(CloudSendBuffer);//���·���
			CloudAct.NeedAns_Time = CloudAct.SysTime;//���·���ʱ��
			++CloudAct.NeedAns_Count;//�������·��ʹ���
		}
	}
}
static bool CloudReport(uchar Code) //�豸�ϱ�
{
    if (CloudAct.NeedAns||CloudAct.DisConectWiFi==true) //������ϢδӦ��,��ģ��Ͽ�״̬
        return 1;
    if (Code == 0)
    {
        if (CloudAct.PubCode != 0) //Pubδ��ȷ�л�,�л��󷵻�
        {
            if (CloudAct.PubCode_t != 0) //Pubδ��ʼ��ʼ�л���Pub=0
            {
                CloudAct.Cmd = AT_MQTTPUB;
                CloudAct.PubCode_t = 0; //��־�Ѿ���ʼ�л�
                CloudSend(0);           //�л����豸����Pub
            }
            return 1;
        }
        CloudSendDataIdx = sprintf(CloudSendData, "\"WaterTemperature\":%.3f",
                                   (DS18B20ST.TemperatureData) * DS18B20ReTransfrom[DS18B20ST.ResolutionMode] + (float)DS18B20MinT);
    }
    else
    {
        if (CloudAct.PubCode != Code) //Pubδ��ȷ�л�
        {
			pdata uchar TempFailCount = CloudAct.NeedAns_FailCount;
            CloudAct.Cmd = AT_MQTTPUB;
            CloudAct.PubCode_t = Code;       //��־�Ѿ���ʼ�л�
            CloudSend(0);                    //�л����豸����Pub
            while (CloudAct.NeedAns == true)
			{
				CloudReceive();
				CloudReSend(5);//500ms�ȴ�
				if(CloudAct.NeedAns_FailCount==TempFailCount+1)//CloudReSend��������
					return 1;//Pubת��ʧ��
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
		case 200:
			CloudSendDataIdx = sprintf(CloudSendData, "");//û�з��ز���
            break;
        }
    }
    CloudAct.Cmd = AT_MQTTSEND; //��д���豸���ϱ�����,׼������
    CloudSend(0);
    return 0;
}
static void CloudHandleReceive(void)
{
#if LOGRANK_UART1 >= 3
    printf("LOG:HandleReceive$%s\r\n", CloudReceiveBuffer);
#endif
    if (strncmp(CloudReceiveBuffer, "ERROR", 5) == 0) //����ִ���쳣
    {
        //Ŀǰ������,������
    }
    else if (strncmp(CloudReceiveBuffer, "OK", 2) == 0) //����ִ������
    {
        if (CloudAct.Cmd != AT_MQTTSTART && CloudAct.Cmd != AT_MQTTSEND && CloudAct.Cmd != AT_MQTTSUB && CloudAct.Cmd != AT_WJAP)
        {
            CloudAct.NeedAns = false;
            if (CloudAct.Cmd == AT_MQTTPUB)
                CloudAct.PubCode = CloudAct.PubCode_t; //�л�Pub���
        }
    }
    else if (CloudReceiveBuffer[0] == '+') //��ȡ��ϸ��Ϣ
    {
        if (strncmp(CloudReceiveBuffer + 1, "MQTTRECV", 8) == 0) //�յ�������Ϣ
        {
			pdata uchar *P=CloudReceiveBuffer+16,*Pend;//�����￪ʼ����id��params
			pdata uchar i=0;
            switch (CloudReceiveBuffer[10]) //�ж϶��ĵ�ͨ��
            {
            case '0': //�ƶ���Ӧ�����ϱ�,�ƶ���Ӧ�¼��ϱ�-����ͨ��
                break;
            case '1': //�ƶ������豸����
                break;
            case '2': //�ƶ˵����豸����:1
				P=strstr(P,"\"id\":\""),P+=6;
				while(*P!='"')  //ѡ��"��Ϊ������־
					CloudAct.SubId[i++]=*P++;
				CloudAct.SubIdLen=i;
				P=strstr(P,"\"params\":{"),P+=10;
				Pend=strchr(P,'}');
			{
				bool isRow2;
				pdata uchar str[17]="                ",readcnt=0;//��֤16���ַ�,ˢ��ʣ�ಿ��Ϊ�հ�
				while(P!=Pend)
				{
					++P;//����"
					if(strncmp(P,"Text",4)==0)
					{
						pdata uchar i=0;
						P+=7;
						while(*P!='"')
							str[i++]=*P++;
						//str[i]=0;//����ַ�����ȡ
						++P;//����"
					}
					else
					{
						P+=8;
						isRow2=(*P++)-'0';
					}
					++readcnt;//����һ������
					if(*P==',')
						++P;
				}
				if(readcnt==2)//������������,��������
				{
					LCD1602WriteLine(str,isRow2);//ִ�з������
					CloudAct.NeedReport_ServiceReCode=200;//����ɹ�
				}
				else
					CloudAct.NeedReport_ServiceReCode=460;//�����������
				CloudAct.NeedReport_Service1=true;//Service1��Ҫ�ظ�
			}
                break;
            case '3': //�ƶ˵����豸����
                break;
            case '4': //�ƶ˵����豸����
                break;
            case '5': //�ƶ˵����豸����
                break;
            case '6': //�ƶ˵����豸����
                break;
            }
        }
        else if (strncmp(CloudReceiveBuffer + 1, "MQTTEVENT", 9) == 0)
        {
            if (CloudAct.Cmd == AT_MQTTSTART && (strncmp(CloudReceiveBuffer + 11, "CONNECT", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //�ɹ�����
            if (CloudAct.Cmd == AT_MQTTSUB && (strncmp(CloudReceiveBuffer + 13, "SUBSCRIBE", 9) == 0))
                if (strncmp(CloudReceiveBuffer + 23, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //�ɹ����ö���
            if (CloudAct.Cmd == AT_MQTTSEND && (strncmp(CloudReceiveBuffer + 11, "PUBLISH", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //�ɹ�����
        }
        else if (strncmp(CloudReceiveBuffer + 1, "WEVENT", 6) == 0)
        {
            if (CloudAct.Cmd == AT_WJAP && (strncmp(CloudReceiveBuffer + 8, "STATION_UP", 10) == 0))
                CloudAct.NeedAns = false; //�ɹ�����
        }
    }
    else //��ȡ������Ϣ
    {
        //Ŀǰ�ò���
    }
}
//------------------------------------------------------------------------------------------------//