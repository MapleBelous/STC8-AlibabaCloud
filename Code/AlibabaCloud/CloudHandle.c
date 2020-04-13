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
static bool CloudSend(uchar op);      //�����������
static bool CloudReport(uchar Code);  //�豸�ϱ�
static void CloudHandleReceive(void); //����CloudReceive�յ���һ��WiFi��Ϣ
//------------------------------------------------------------------------------------------------//
void CloudLoop(void) //Cloud��ѭ��
{
    CloudReceive(); //##���ղ������ڻ���������
    //ע������
    if (CloudAct.NeedReadDS18B20 == false && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20NeedReadMs && DS18B20ConvertTemperature() == EXIT_SUCCESS) //��Ҫ��ȡDS18B20�¶�ֵ,���¶�ת��ָ��ɹ�
        CloudAct.NeedReadDS18B20 = true, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;                                                                         //�ȴ���ȡ�¶�ֵ
                                                                                                                                                                 //��ִ������
    if (CloudAct.NeedReport_WaterTemperatureLow == true && CloudReport(1) == 0)
        CloudAct.NeedReport_WaterTemperatureLow = false;
    if (CloudAct.NeedReport_WaterTemperatureHigh == true && CloudReport(2) == 0)
        CloudAct.NeedReport_WaterTemperatureHigh = false;
    if (CloudAct.NeedReport == true && CloudReport(0) == 0)
        CloudAct.NeedReport = false;

    if (CloudAct.NeedReadDS18B20 == true && CloudAct.SysTime - CloudAct.NeedReadDS18B20_Ms >= DS18B20ConvertTMaxTime[DS18B20ST.ResolutionMode] && DS18B20GetTemperature() == EXIT_SUCCESS) //�ɹ�ִ�ж�ȡ�¶�ֵ
        CloudAct.NeedReadDS18B20 = false, CloudAct.NeedReadDS18B20_Ms = CloudAct.SysTime;
}
void CloudInit(void) //��ʼ��Cloud
{
    xdata ulong TempT;
    memset(&CloudAct, 0, sizeof(CloudAct));                //��������ָʾ��
    CloudSendDataIdx = CloudSendIdx = CloudReceiveIdx = 0; //���������±�����
//------------------------------------------------//
T0:
    CloudAct.Cmd = AT_REBOOT; //����ģ��
    CloudSend(2);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T0;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-Reboot ok\r\n"); //��־��¼ģ���������
#endif
//------------------------------------------------//
T0x:
    CloudAct.Cmd = AT_WJAPQ; //�Ͽ���ǰWiFi����
    CloudSend(2);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 3000) //3000msû���������
        {
			CloudAct.NeedAns=false;
			goto T0x;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiDisConect ok\r\n"); //��־��¼ģ��Ͽ���ǰWiFi����
#endif
//------------------------------------------------//
T1:
    CloudAct.Cmd = AT_WJAP; //����Ŀ��WiFi
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >=10000) //10000msû���������,WiFI�ź���ʱ�������Ӻ���
        {
			CloudAct.NeedAns=false;
			goto T1;//�ط�ָ��
		}	
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-WiFiConect ok\r\n"); //��־��¼Ŀ��WiFi������
#endif
//------------------------------------------------//
T2:
    CloudAct.Cmd = AT_MQTTAUTH; //����MQTT�û�������
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T2;//�ط�ָ��
		}	
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_UserSet ok\r\n"); //��־��¼MQTT�û����������������
#endif
//------------------------------------------------//
T3:
    CloudAct.Cmd = AT_MQTTSOCK; //����MQTT�����Ͷ˿�
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T3;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HostSet ok\r\n"); //��־��¼MQTT�����Ͷ˿��������
#endif
//------------------------------------------------//
T4:
    CloudAct.Cmd = AT_MQTTCID; //����MQTT�ͻ���ID
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T4;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_ClintIDSet ok\r\n"); //��־��¼MQTT�ͻ���ID�������
#endif
//------------------------------------------------//
T5:
    CloudAct.Cmd = AT_MQTTKEEPALIVE; //����MQTT��������
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T5;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_HeartSet ok\r\n"); //��־��¼MQTT����ʱ���������
#endif
//------------------------------------------------//
T6:
    CloudAct.Cmd = AT_MQTTRECONN; //����MQTT�Ƿ��Զ�����
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T6;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoReConect Set ok\r\n"); //��־��¼MQTT�Զ������������
#endif
//------------------------------------------------//
T7:
    CloudAct.Cmd = AT_MQTTAUTOSTART; //����MQTT�Ƿ��ϵ��Զ�����
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T7;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_AutoStartSet ok\r\n"); //��־��¼MQTT�ϵ��Զ������������
#endif
//------------------------------------------------//
T8:
    CloudAct.Cmd = AT_MQTTSTART; //����MQTT
    CloudSend(2);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T8;//�ط�ָ��
		}
    }
#if LOGRANK_UART1 >= 2
    printf("LOG#:CloudInit-MQTT_Start ok\r\n"); //��־��¼MQTT�����ɹ�
#endif
//------------------------------------------------//
T9:
    CloudAct.Cmd = AT_MQTTPUB; //����MQTT����-Ĭ��Ϊ��������PubCode=0
    CloudAct.PubCode_t = 0;
    CloudSend(0);
    TempT = CloudAct.SysTime; //��¼�����ʱ��
    while (CloudAct.NeedAns == true)
    {
        CloudReceive();
        if (CloudAct.SysTime - TempT >= 1500) //1500msû���������
        {
			CloudAct.NeedAns=false;
			goto T9;//�ط�ָ��
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
    data ushort idx1 = uart4_idx1, idx2 = uart4_idx2;
    while (idx1 != idx2)
    {
        switch (CloudReceiveState)
        {
        case 0: //�ȴ���
            if (uart4_buffer[idx1] == 0x0A)
                CloudReceiveState = 1; //�ȴ�/r
            break;
        case 1:                             //��ȡ��
            if (uart4_buffer[idx1] == 0x0D) //����\n,������ȡ,��CloudHandleReceive��������
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
            case 200:
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
    uart4_sendstr8(CloudSendBuffer, 0);       //����׼���õ������ַ���
    CloudAct.NeedAns = true;                  //��ʼ�ȴ�Ӧ��
    CloudAct.NeedAns_Time = CloudAct.SysTime; //��¼��������ʱ��
    return 0;
}
static bool CloudReport(uchar Code) //�豸�ϱ�
{
    if (CloudAct.NeedAns) //������ϢδӦ��
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
            xdata ulong TempT;
        TX:
            CloudAct.Cmd = AT_MQTTPUB;
            CloudAct.PubCode_t = Code;       //��־�Ѿ���ʼ�л�
            CloudSend(0);                    //�л����豸����Pub
            TempT = CloudAct.SysTime;        //��¼�����ʱ��
            while (CloudAct.PubCode != Code) //�ȴ�,ֱ��Pub��ȷ
            {
                CloudReceive();
                if (CloudAct.SysTime - TempT >= 300) //300msû���������
				{
					CloudAct.NeedAns=false;
					goto TX;//�ط�ָ��
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
            switch (CloudReceiveBuffer[10]) //�ж϶��ĵ�ͨ��
            {
            case '0': //��������ͨ��
                break;
            case '1': //�豸�������
                break;
            case '3': //�ƶ���Ӧ�����ϱ�
                break;
            case '4': //�ƶ���Ӧ�¼��ϱ�
                break;
            }
        }
        else if (strncmp(CloudReceiveBuffer + 1, "MQTTEVENT", 9) == 0)
        {
            if (CloudAct.Cmd == AT_MQTTSTART && (strncmp(CloudReceiveBuffer + 11, "CONNECT", 7) == 0))
                if (strncmp(CloudReceiveBuffer + 19, "SUCCESS", 7) == 0)
                    CloudAct.NeedAns = false; //�ɹ�����
            if (CloudAct.Cmd == AT_MQTTSUB && (strncmp(CloudReceiveBuffer + 11, "SUBSCRIBE", 9) == 0))
                if (strncmp(CloudReceiveBuffer + 21, "SUCCESS", 7) == 0)
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