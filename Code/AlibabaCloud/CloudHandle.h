#ifndef _CloudHandle_H_
#define _CloudHandle_H_
#include "CloudCodeArray.h"
#include "DS18B20.h"
#include "LCD1602.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
//------------------------------------------------------------------------------------------------//
//Cloud����#״ָ̬ʾ��
typedef struct
{
	//ϵͳ״̬
    ulong SysTime;    //ϵͳʱ��,��λ1ms
	uchar DisConectWiFi:1;//��WiFiģ��Ͽ�����
	//MQTT״̬
    uchar Cmd;        //��������ı��
    uchar MQTTSENDid; //MQTTSENDָ��ר��id��,0~255ѭ��
	uchar SubisBusy:6;//��ǰSubͨ��ʹ�����,0~5
	uchar SubCode:3;  //׼�����ĵ�Sub,0~5
	uchar SubId[11];  //����Id��
	uchar SubIdLen:4; //����Id�ų���
    uchar PubCode;    //��ǰPub
    uchar PubCode_t;  //Ŀ���л���Pub
	//�ȴ�WiFiӦ�����ط�����״̬
    uchar NeedAns : 1;       //�ȴ�WiFiӦ����
    ulong NeedAns_Time;      //���������ʱ��
    uchar NeedAns_Count : 2; //���·��͵Ĵ���
	uchar NeedAns_FailCount:2;//���·�������ʧ�ܷ������·��͵Ĵ���
	//�ϱ�״̬
    uchar NeedReport : 1;                      //��Ҫ�ϱ��豸��Ϣ
    uchar NeedReport_Event_1 : 1;  //��Ҫ����WaterTemperatureLow
    uchar NeedReport_Event_2 : 1; //��Ҫ����WaterTemperatureHigh
	uchar NeedReport_Service1:1;//��Ҫ��Ӧservice1
	ulong NeedReport_ServiceReCode;//��Ӧ��Code
	//���豸״̬
    uchar NeedReadDS18B20 : 1; //DS18B20״̬-ת���¶���or����
    ulong NeedReadDS18B20_Ms;
} CloudActST;
extern xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
extern void CloudLoop(void); //Cloud��ѭ��
extern void CloudInit(void); //��ʼ��Cloud
//------------------------------------------------------------------------------------------------//
//����������С
#define CloudReceiveBufferSize 500
#define CloudSendBufferSize 500
#define CloudSendDataSize 100
//------------------------------------------------------------------------------------------------//
//��ʱ����ʱ�����
#define DS18B20NeedReadMs 1000
//------------------------------------------------------------------------------------------------//
#endif