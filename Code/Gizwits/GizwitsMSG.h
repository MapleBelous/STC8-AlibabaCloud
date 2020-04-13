#ifndef _GizwitsMSG_H_
#define _GizwitsMSG_H_
#include "GizwitsConfig.h"
#include "GizwitsHandle.h"
#include "MCUdef.h"
#include <string.h>
#include "DS18B20.h"
//------------------------------------------------------------------------------------------------//
ushort GizwitsAnalyseCmd(uchar, ushort); //分析数据包Cmd&检查Flags

void GizwitsAct(void);                //处理当前任务
void GizwitsHandleWiFiAsk(uchar);     //处理接收到的Ask数据包(生成Ans数据包/相关信息处理和任务设置)
void GizwitsHandleWiFiAns(uchar);     //处理接收到的Ans数据包(相关信息处理和取消系统等待Ans状态)
uchar GizwitsHandleMCUAsk(uchar Cmd); //生成待发送Cmd命令的Ask数据包
//------------------------------------------------------------------------------------------------//

//GizwitsAnalyseCmd DEF
#define ReCmd_FlagsERR 0xF000
#define ReCmd_CmdERR 0xF100
#define ReCmd_MSGERR 0xF200
#define ReCmd_MCU2WiFi 0x0100

//------------------------------------------------------------------------------------------------//
#endif