#ifndef _CloudHandle_H_
#define _CloudHandle_H_
#include "CloudCodeArray.h"
#include "DS18B20.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
//------------------------------------------------------------------------------------------------//
//Cloud����#״ָ̬ʾ��
typedef struct
{
    ulong SysTime;    //ϵͳʱ��,��λ1ms
    uchar Cmd;        //��������ı��
    uchar MQTTSENDid; //MQTTSENDָ��ר��id��,0~255ѭ��
    uchar PubCode;    //��ǰPub
    uchar PubCode_t;  //Ŀ���л���Pub

    uchar NeedAns : 1;       //�ȴ�WiFiӦ����
    ulong NeedAns_Time;      //���������ʱ��
    uchar NeedAns_Count : 2; //���·��͵Ĵ���
	uchar NeedAns_FailCount:2;//���·�������ʧ�ܷ������·��͵Ĵ���
	uchar DisConectWiFi:1;//��WiFiģ��Ͽ�����

    uchar NeedReport : 1;                      //��Ҫ�ϱ��豸��Ϣ
    uchar NeedReport_WaterTemperatureLow : 1;  //��Ҫ����WaterTemperatureLow
    uchar NeedReport_WaterTemperatureHigh : 1; //��Ҫ����WaterTemperatureHigh

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