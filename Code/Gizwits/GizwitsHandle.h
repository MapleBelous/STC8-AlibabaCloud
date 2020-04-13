#ifndef _GizwitsHandle_H_
#define _GizwitsHandle_H_
#include "GizwitsMSG.h"
#include "MCUdef.h"
#include "uart.h"
#include <string.h>
//------------------------------------------------------------------------------------------------//
void GizwitsMainLoop(void); //Gizwits主循环函数
void GizwitsInit(void);     //初始化Gizwits内存

void GizwitsSend(uchar Mode); //发送GizwitsSendBuffer缓存区内的数据包&记录已发送数据包
//GizwitsSendBuffer缓存区内应存有Cmd-负载全部数据,GizwitsSendIdx为数据包大小(不计入包头+Len+Sum)
void GizwitsReAsk(uchar); //延时等待WiFi模组Ans&重新发送MCU的Ask数据包
//------------------------------------------------------------------------------------------------//
//++++++++定时任务时间间隔(Ms),0为不执行++++++++//
#define GizwitsNeedReportMs 0 //整体数据需要更新上报的最大间隔
#define DS18B20NeedReadMs 1000 //DS18B20采集温度数据间隔

//++++++++++++++++++++++++++++++++++++++++++++++//
#define GizwitsUseWiFiRealTime //激活从WiFi获取现实时间的指令
#define GizwitsUseWiFiProperty //激活从WiFi获取WiFi的设备信息指令
//++++++++++++++++++++++++++++++++++++++++++++++//
#define GizwitsReceiveBufferSize 500 //GizwitsReceive缓存区容量
#define GizwitsSendBufferSize 500    //GizwitsSend缓存区容量
#define GizwitsSendOldQSize 3        //MCU Ans历史记录条数上限
//------------------------------------------------------------------------------------------------//
typedef struct GizwitsActST //Gizwits任务指示器
{
    ulong GizwitsSysMs; //系统时间 (ms)
	uchar WiFiConect:1;//当前已经回应WiFi,Ans=0x02,可以开始Ask
    //++++++++++++++++++++++++++++++++++++++++++++++//
    //待执行任务标志&信息
    uchar NeedAns : 1;          //已发送Ask,需要WiFi应答
    uchar NeedAns_RstCount : 3; //重新发送Ask的次数
    ulong NeedAns_Ms;           //上次发送Ask的时间
    uchar NeedRstMCU : 1; //准备重启MCU
    ulong NeedRstMCU_Ms;  //准备重启MCU的时间
    uchar NeedReport : 1; //需要上报设备状态信息
    ulong NeedReport_Ms;  //上次上报设备状态的系统时间
    
	uchar NeedReadDS18B20:1;//0:可以开始转换温度;1:等待读取DS18B20温度数据
	ulong NeedReadDS18B20_Ms;//上次DS18B20(开始转换温度/读取数据)的系统时间

    //++++++++++++++++++++++++++++++++++++++++++++++//
    //WiFi模组工作状态
    uchar OpenSoftAP : 1;      //SoftAP模式
    uchar OpenStation : 1;     //Station模式
    uchar OpenOnBoarding : 1;  //配置(OnBoarding)模式
    uchar OpenBindMode : 1;    //开启绑定模式
    uchar ConnectRoute : 1;    //成功连接无线路由器
    uchar ConnectM2M : 1;      //成功连接M2M服务器
    uchar RouteRSSI : 3;       //ConnectRoute==1时有效,当前已连接无线路由器信号强度(RSSI),0->7
    uchar AppOnline : 1;       //有App在线
    uchar ProductTestMode : 1; //开启产品测试模式
    //++++++++++++++++++++++++++++++++++++++++++++++//
    //相关Ask or Ans指令模式
    uchar GizwitsConfigMode : 3; //请求WiFi模组配网的方式
    //1:SoftAP方式;2:AirLink方式;4:直接写入配置信息模式;配置方式不合法时,默认进入AirLink配置方式
    uchar GizwitsIllegalCode; //应答WiFi模组数据包非法,错误原因
                              //1校验和错误;2命令不可识别;3其它错误;4文件类型不匹配;0和5~255保留
} GizwitsActST;
extern xdata GizwitsActST GizwitsST;
//------------------------------------------------------------------------------------------------//

//GizwitsReceive 数据包
extern pdata ushort GizwitsReceiveLen;                             //当前接收的数据包Len值
extern pdata uchar GizwitsReceiveCmd;                              //当前接收的数据包Cmd值
extern pdata uchar GizwitsReceiveSn;                               //当前接收的数据包Sn值
extern pdata ushort GizwitsReceiveFlags;                           //当前接收的数据包Flags值
extern xdata uchar GizwitsReceiveBuffer[GizwitsReceiveBufferSize]; //当前接收的数据包负载缓存区
extern pdata ushort GizwitsReceiveIdx;                             //当前接收的数据包负载缓存区下标
extern pdata uchar GizwitsReceiveSum;                              //当前接收的数据包Sum值

//GizwitsSend - 数据包
extern xdata uchar GizwitsSendBuffer[GizwitsSendBufferSize]; //发送数据包缓存区
extern pdata ushort GizwitsSendIdx;                          //发送数据包缓存区下标

//GizwitsSendOld - GizwitsSend数据包历史记录,Ans数据包GizwitsSendOldQSize个,末尾固定为Ask数据包
extern xdata uchar GizwitsSendOldBuffer[GizwitsSendOldQSize + 1][GizwitsSendBufferSize]; //发送数据包缓存区历史记录
extern xdata ushort GizwitsSendOldIdx[GizwitsSendOldQSize + 1];                          //发送数据包缓存区下标
extern xdata uchar GizwitsSendOldHead, GizwitsSendOldTail;                               //队列头尾下标

#ifdef GizwitsUseWiFiRealTime
typedef struct GizwitsTimeFromWiFi //从WiFi模组获取的现实时间
{                                  //当前时区下
    ushort Year : 12;              //年,足够支持0~4095
    uchar Month : 4;               //月
    uchar Day : 5;                 //日
    uchar Hour : 5;                //小时
    uchar Minute : 6;              //分钟
    uchar Second : 6;              //秒
    ulong NTP;                     //19700101至今秒数-零时区时间
} GizwitsTimeFromWiFi;
extern xdata GizwitsTimeFromWiFi GizwitsRealTime;
#endif
#ifdef GizwitsUseWiFiProperty
typedef struct GizwitsPropertyFromWiFi //使用从WiFi获取WiFi的设备信息指令
{
    uchar ModuleType;           //模组类型
    uchar SerialProtocolVer[8]; //通用串口协议版本号
    uchar HardwareVer[8];       //硬件版本号
    uchar SoftwareVer[8];       //软件版本号
    uchar Mac[16];              //Mac地址
    uchar Ip[16];               //Ip地址
    uchar Property[8];          //设备属性
} GizwitsPropertyFromWiFi;
extern xdata GizwitsPropertyFromWiFi GizwitsWiFiProperty;
#endif

//------------------------------------------------------------------------------------------------//
#endif