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
//Cloud任务#状态指示器
typedef struct
{
	//-------------------------系统状态------------------------------//
    ulong SysTime;    //系统时间,单位1ms
	uchar DisConectWiFi:1;//与WiFi模组断开连接(离线工作)
	//-------------------------MQTT状态------------------------------//
    uchar Cmd;        //发送命令的编号
    uchar MQTTSENDid; //MQTTSEND指令专用id号,0~255循环
	//---------------------------Sub---------------------------------//
	uchar SubisBusy:6;//当前Sub通道使用情况,0~5
	uchar SubCode:3;  //准备订阅的Sub,0~5
	uchar SubId[11];  //订阅Id号
	uchar SubIdLen:4; //订阅Id号长度
	//---------------------------Pub---------------------------------//
    uchar PubCode;    //当前Pub
    uchar PubCode_t;  //目标切换的Pub
	//-----------------等待WiFi应答与重发命令状态--------------------//
    uchar NeedAns : 1;       //等待WiFi应答中
    ulong NeedAns_Time;      //发送命令的时间
    uchar NeedAns_Count : 2; //重新发送的次数
	uchar NeedAns_FailCount:2;//重新发送连续失败放弃重新发送的次数
	//-------------------------上报状态------------------------------//
    uchar NeedReport : 1;          //需要上报设备参数
	struct{
		uchar DS18B20:1;           
		uchar GY_25:1;
		uchar LED1:1,LED2:1,LED3:1;
		uchar IsStorageLED:1;
	}NeedReportT;					//需要上报的参数类型
    uchar NeedReport_Event_1 : 1;  //需要上报事件:警报WaterTemperatureLow
    uchar NeedReport_Event_2 : 1;  //需要上报事件:警报WaterTemperatureHigh
	uchar NeedReport_Service1:1;//需要回应service1
	uchar NeedReport_Service2:1;//需要回应service2
	uchar NeedReport_Service3:1;//需要回应service3
	ulong NeedReport_ServiceReCode;//回应的Code
	//-------------------------子设备状态-----------------------------//
    uchar NeedReadGY_25 : 1; //GY_25状态-已发送查询指令or待命
    ulong NeedReadGY_25_Ms;
    uchar NeedReadDS18B20 : 1; //DS18B20状态-转换温度中or待命
    ulong NeedReadDS18B20_Ms;
	//-------------------------延时类服务-----------------------------//
	uchar ServiceMCUOffline:1;//离线工作任务
	uchar ServiceMCURst:1;//重启MCU任务(重启前应当离线)
} CloudActST;
extern xdata CloudActST CloudAct;
//------------------------------------------------------------------------------------------------//
extern void CloudLoop(void); //Cloud主循环
extern void CloudInit(void); //初始化Cloud
//------------------------------------------------------------------------------------------------//
//各缓冲区大小
#define CloudReceiveBufferSize 600
#define CloudSendBufferSize 600
#define CloudSendDataSize 300
//------------------------------------------------------------------------------------------------//
//定时任务时间参数
#define DS18B20NeedReadMs 500
#define GY_25NeedReadMs 1000
//------------------------------------------------------------------------------------------------//
#endif