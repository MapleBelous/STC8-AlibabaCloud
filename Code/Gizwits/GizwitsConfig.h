#ifndef _GizwitsConfig_H_
#define _GizwitsConfig_H_
#include "MCUdef.h"
//------------------------------------------------------------------------------------------------//

#define ConfigMode4UseCodeArray

//------------------------------------------------------------------------------------------------//
//Gizwits System Set for WiFiAsk, AnsCmd = 0x02
extern code uchar SerialProtocolVer[];
extern code uchar BusinessProtocolVer[];
extern code uchar HardwareVer[];
extern code uchar SoftwareVer[];
extern code uchar ProductKey[];
extern code ushort BindingStateSec;
extern code uchar DeviceProperties[];
extern code uchar ProductSecret[];
extern code ushort DataLen;
extern code uchar Data[];

//ConfigMode4 WiFi Config for MCUAsk, AskCmd = 0x09
#ifdef ConfigMode4UseCodeArray
#define ConfigMode4ArrayMode code
#else
#define ConfigMode4ArrayMode xdata
#endif
extern ConfigMode4ArrayMode uchar ConfigMode4Ssid[];
extern ConfigMode4ArrayMode uchar ConfigMode4Password[];
extern ConfigMode4ArrayMode uchar ConfigMode4Bssid[];
extern ConfigMode4ArrayMode uchar ConfigMode4Tz[];
extern ConfigMode4ArrayMode uchar ConfigMode4ServerName[];

//------------------------------------------------------------------------------------------------//
#endif