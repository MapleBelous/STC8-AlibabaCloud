#ifndef _CloudHandle_H_
#define _CloudHandle_H_
#include "CloudCodeArray.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
#include "DS18B20.h"
#include "LCD1602.h"
#include "GY_25.h"
#include "LED.h"
//------------------------------------------------------------------------------------------------//
//Cloud����#״ָ̬ʾ��
typedef struct
{
	//-------------------------ϵͳ״̬------------------------------//
    ulong SysTime;    //ϵͳʱ��,��λ1ms
	uchar DisConectWiFi:1;//��WiFiģ��Ͽ�����(���߹���)
	//-------------------------MQTT״̬------------------------------//
    uchar Cmd;        //��������ı��
    uchar MQTTSENDid; //MQTTSENDָ��ר��id��,0~255ѭ��
	//---------------------------Sub---------------------------------//
	uchar SubisBusy:6;//��ǰSubͨ��ʹ�����,0~5
	uchar SubCode:3;  //׼�����ĵ�Sub,0~5
	uchar SubId[11];  //����Id��
	uchar SubIdLen:4; //����Id�ų���
	//---------------------------Pub---------------------------------//
    uchar PubCode;    //��ǰPub
    uchar PubCode_t;  //Ŀ���л���Pub
	//-----------------�ȴ�WiFiӦ�����ط�����״̬--------------------//
    uchar NeedAns : 1;       //�ȴ�WiFiӦ����
    ulong NeedAns_Time;      //���������ʱ��
    uchar NeedAns_Count : 2; //���·��͵Ĵ���
	uchar NeedAns_FailCount:2;//���·�������ʧ�ܷ������·��͵Ĵ���
	//-------------------------�ϱ�״̬------------------------------//
    uchar NeedReport : 1;          //��Ҫ�ϱ��豸����
	struct{
		uchar DS18B20:1;           
		uchar GY_25:1;
		uchar LED1:1,LED2:1,LED3:1;
		uchar IsStorageLED:1;
	}NeedReportT;					//��Ҫ�ϱ��Ĳ�������
    uchar NeedReport_Event_1 : 1;  //��Ҫ�ϱ��¼�:����WaterTemperatureLow
    uchar NeedReport_Event_2 : 1;  //��Ҫ�ϱ��¼�:����WaterTemperatureHigh
	uchar NeedReport_Service1:1;//��Ҫ��Ӧservice1
	uchar NeedReport_Service2:1;//��Ҫ��Ӧservice2
	uchar NeedReport_Service3:1;//��Ҫ��Ӧservice3
	ulong NeedReport_ServiceReCode;//��Ӧ��Code
	//-------------------------���豸״̬-----------------------------//
    uchar NeedReadGY_25 : 1; //GY_25״̬-�ѷ��Ͳ�ѯָ��or����
    ulong NeedReadGY_25_Ms;
    uchar NeedReadDS18B20 : 1; //DS18B20״̬-ת���¶���or����
    ulong NeedReadDS18B20_Ms;
	//-------------------------��ʱ�����-----------------------------//
	uchar ServiceMCUOffline:1;//���߹�������
	uchar ServiceMCURst:1;//����MCU����(����ǰӦ������)
} CloudActST;
extern xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
extern void CloudLoop(void); //Cloud��ѭ��
extern void CloudInit(void); //��ʼ��Cloud
//------------------------------------------------------------------------------------------------//
//����������С
#define CloudReceiveBufferSize 600
#define CloudSendBufferSize 600
#define CloudSendDataSize 300
//------------------------------------------------------------------------------------------------//
//��ʱ����ʱ�����
#define DS18B20NeedReadMs 500
#define GY_25NeedReadMs 1000
//------------------------------------------------------------------------------------------------//
#endif