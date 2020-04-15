#ifndef _CloudHandle_H_
#define _CloudHandle_H_
#include "CloudCodeArray.h"
#include "DS18B20.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
//------------------------------------------------------------------------------------------------//
//Cloud任务#状态指示器
typedef struct
{
    ulong SysTime;    //系统时间,单位1ms
    uchar Cmd;        //发送命令的编号
    uchar MQTTSENDid; //MQTTSEND指令专用id号,0~255循环
    uchar PubCode;    //当前Pub
    uchar PubCode_t;  //目标切换的Pub

    uchar NeedAns : 1;       //等待WiFi应答中
    ulong NeedAns_Time;      //发送命令的时间
    uchar NeedAns_Count : 2; //重新发送的次数
	uchar NeedAns_FailCount:2;//重新发送连续失败放弃重新发送的次数
	uchar DisConectWiFi:1;//与WiFi模组断开连接

    uchar NeedReport : 1;                      //需要上报设备信息
    uchar NeedReport_WaterTemperatureLow : 1;  //需要警报WaterTemperatureLow
    uchar NeedReport_WaterTemperatureHigh : 1; //需要警报WaterTemperatureHigh

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