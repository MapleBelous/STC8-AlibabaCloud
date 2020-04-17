#ifndef _CloudHandle_H_
#define _CloudHandle_H_
#include "CloudCodeArray.h"
#include "DS18B20.h"
#include "LCD1602.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
//------------------------------------------------------------------------------------------------//
//Cloud任务#状态指示器
typedef struct
{
	//系统状态
    ulong SysTime;    //系统时间,单位1ms
	uchar DisConectWiFi:1;//与WiFi模组断开连接
	//MQTT状态
    uchar Cmd;        //发送命令的编号
    uchar MQTTSENDid; //MQTTSEND指令专用id号,0~255循环
	uchar SubisBusy:6;//当前Sub通道使用情况,0~5
	uchar SubCode:3;  //准备订阅的Sub,0~5
	uchar SubId[11];  //订阅Id号
	uchar SubIdLen:4; //订阅Id号长度
    uchar PubCode;    //当前Pub
    uchar PubCode_t;  //目标切换的Pub
	//等待WiFi应答与重发命令状态
    uchar NeedAns : 1;       //等待WiFi应答中
    ulong NeedAns_Time;      //发送命令的时间
    uchar NeedAns_Count : 2; //重新发送的次数
	uchar NeedAns_FailCount:2;//重新发送连续失败放弃重新发送的次数
	//上报状态
    uchar NeedReport : 1;                      //需要上报设备信息
    uchar NeedReport_Event_1 : 1;  //需要警报WaterTemperatureLow
    uchar NeedReport_Event_2 : 1; //需要警报WaterTemperatureHigh
	uchar NeedReport_Service1:1;//需要回应service1
	ulong NeedReport_ServiceReCode;//回应的Code
	//子设备状态
    uchar NeedReadDS18B20 : 1; //DS18B20状态-转换温度中or待命
    ulong NeedReadDS18B20_Ms;
} CloudActST;
extern xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
extern void CloudLoop(void); //Cloud主循环
extern void CloudInit(void); //初始化Cloud
//------------------------------------------------------------------------------------------------//
//各缓冲区大小
#define CloudReceiveBufferSize 500
#define CloudSendBufferSize 500
#define CloudSendDataSize 100
//------------------------------------------------------------------------------------------------//
//定时任务时间参数
#define DS18B20NeedReadMs 1000
//------------------------------------------------------------------------------------------------//
#endif