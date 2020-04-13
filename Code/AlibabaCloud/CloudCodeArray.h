#ifndef _CloudCodeArray_H_
#define _CloudCodeArray_H_
#include "MCUdef.h"
//------------------------------------------------------------------------------------------------//
//AT命令集
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
//Cloud参数
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
//Subscribe参数
#define SubscribeSet1 "/sys/%s/%s/thing/event/property/post_reply,0" //设置为云端响应属性上报,SubCode=0
#define SubscribeSet2 "/sys/%s/%s/thing/service/property/set,0"      //设置为云端设置设备属性,SubCode=1
#define SubscribeSet3 "/sys/%s/%s/thing/event/%s/post_reply,0"       //设置为云端响应事件上报,SubCode=0
#define SubscribeSet4 "/sys/%s/%s/thing/service/%s,0"                //设置为云端调用设备服务,SubCode=2~5
//------------------------------------------------------------------------------------------------//
//Publish参数
#define PublishSet1 "/sys/%s/%s/thing/event/property/post,0" //设置为设备属性上报,PubCode=0
#define PublishSet2 "/sys/%s/%s/thing/event/%s/post,0"       //设置为设备事件上报,PubCode=1~199
#define PublishSet3 "/sys/%s/%s/thing/service/%s_reply,0"    //设置为响应服务调用,PubCode=200~255
//--------------------------------------------------//
extern code uchar Event_1[]; //事件1,PubCode=1
extern code uchar Event_2[]; //事件2,PubCode=2
extern code uchar Event_1_Len;
extern code uchar Event_2_Len;
//--------------------------------------------------//
extern code uchar Service_1[]; //服务1,PubCode=200,SubCode=2
extern code uchar Service_1_Len;
//------------------------------------------------------------------------------------------------//
//Send参数
#define SendSet1 "{\"id\":\"%03bu\",\"version\":\"1.0\",\"params\":{%s},\"method\":\"thing.event.property.post\"}" //设备上报属性
#define SendSet1Len 77                                                                                             //SendSet1字符串固定长度+id三位数=74+3=77
#define SendSet2 "{\"id\":\"%03bu\",\"version\":\"1.0\",\"params\":{%s},\"method\":\"thing.event.%s.post\"}"       //设备上报事件
#define SendSet2Len 69
//------------------------------------------------------------------------------------------------//
#endif