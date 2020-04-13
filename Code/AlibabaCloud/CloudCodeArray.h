#ifndef _CloudCodeArray_H_
#define _CloudCodeArray_H_
#include "MCUdef.h"
//------------------------------------------------------------------------------------------------//
//AT���
extern code uchar ATCmd0[], ATCmd1[], ATCmd2[], ATCmd3[], ATCmd4[];
extern code uchar ATCmd5[], ATCmd6[], ATCmd7[], ATCmd8[], ATCmd9[];
extern code uchar ATCmd10[], ATCmd11[], ATCmd12[], ATCmd13[];
extern code uchar *ATCmd[];
//--------------------------------------------//
#define AT_MQTTAUTH 0
#define AT_MQTTSOCK 1
#define AT_MQTTCID 2
#define AT_MQTTKEEPALIVE 3
#define AT_MQTTRECONN 4
#define AT_MQTTAUTOSTART 5
#define AT_MQTTSTART 6
#define AT_MQTTSUB 7
#define AT_MQTTPUB 8
#define AT_MQTTSEND 9
#define AT_MQTTUNSUB 10
#define AT_MQTTSTATUS 11
#define AT_MQTTCLOSE 12
#define AT_WJAP 13
#define AT_REBOOT 14
//------------------------------------------------------------------------------------------------//
//Cloud����
extern code uchar DeviceName[];
extern code uchar ProductKey[];
extern code uchar WiFiSSID[];
extern code uchar WiFiPassword[];
extern code uchar MQTTUser[];
extern code uchar MQTTPassword[];
extern code uchar MQTTHost[];
extern code uchar MQTTPort[];
extern code uchar MQTTClientID[];
extern code uchar MQTTHeartBeat[];
extern code uchar MQTTAutoReConect[];
extern code uchar MQTTAutoStart[];
//------------------------------------------------------------------------------------------------//
//Subscribe����
#define SubscribeSet1 "/sys/%s/%s/thing/event/property/post_reply,0" //����Ϊ�ƶ���Ӧ�����ϱ�,SubCode=0
#define SubscribeSet2 "/sys/%s/%s/thing/service/property/set,0"      //����Ϊ�ƶ������豸����,SubCode=1
#define SubscribeSet3 "/sys/%s/%s/thing/event/%s/post_reply,0"       //����Ϊ�ƶ���Ӧ�¼��ϱ�,SubCode=0
#define SubscribeSet4 "/sys/%s/%s/thing/service/%s,0"                //����Ϊ�ƶ˵����豸����,SubCode=2~5
//------------------------------------------------------------------------------------------------//
//Publish����
#define PublishSet1 "/sys/%s/%s/thing/event/property/post,0" //����Ϊ�豸�����ϱ�,PubCode=0
#define PublishSet2 "/sys/%s/%s/thing/event/%s/post,0"       //����Ϊ�豸�¼��ϱ�,PubCode=1~199
#define PublishSet3 "/sys/%s/%s/thing/service/%s_reply,0"    //����Ϊ��Ӧ�������,PubCode=200~255
//--------------------------------------------------//
extern code uchar Event_1[]; //�¼�1,PubCode=1
extern code uchar Event_2[]; //�¼�2,PubCode=2
extern code uchar Event_1_Len;
extern code uchar Event_2_Len;
//--------------------------------------------------//
extern code uchar Service_1[]; //����1,PubCode=200,SubCode=2
extern code uchar Service_1_Len;
//------------------------------------------------------------------------------------------------//
//Send����
#define SendSet1 "{\"id\":\"%03bu\",\"version\":\"1.0\",\"params\":{%s},\"method\":\"thing.event.property.post\"}" //�豸�ϱ�����
#define SendSet1Len 77                                                                                             //SendSet1�ַ����̶�����+id��λ��=74+3=77
#define SendSet2 "{\"id\":\"%03bu\",\"version\":\"1.0\",\"params\":{%s},\"method\":\"thing.event.%s.post\"}"       //�豸�ϱ��¼�
#define SendSet2Len 69
//------------------------------------------------------------------------------------------------//
#endif