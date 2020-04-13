#include "GizwitsConfig.h"
//------------------------------------------------------------------------------------------------//

//Gizwits System Set for WiFiAsk, AnsCmd = 0x02
code uchar SerialProtocolVer[8] = {"00000004"};//通用串口协议版本号
code uchar BusinessProtocolVer[8] = {"00000002"};//义务协议版本号
code uchar HardwareVer[8] = {"Ha200402"};//硬件版本号
code uchar SoftwareVer[8] = {"Hs200407"};//软件版本号
code uchar ProductKey[32] = {"c6491f9bc91b497c80fef21ad01db936"};
code ushort BindingStateSec = 30;//可绑定状态失效时间(s),0表示随时可绑定
code uchar DeviceProperties[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//设备属性
code uchar ProductSecret[32] = {"21a08167d7d84d43aa525fc56170e7fd"};
code ushort DataLen = 40;//环境参数长度,必须精准无误,=strlen(Data)
code uchar Data[] = "apName=ESP8266_SoftAPMode&apPwd=asdf1234";//环境参数

//ConfigMode4 WiFi Config for MCUAsk, AskCmd = 0x09
ConfigMode4ArrayMode uchar ConfigMode4Ssid[] = "BelousWIFI";
ConfigMode4ArrayMode uchar ConfigMode4Password[] = "12340987";
ConfigMode4ArrayMode uchar ConfigMode4Bssid[] = "";
ConfigMode4ArrayMode uchar ConfigMode4Tz[] = "";
ConfigMode4ArrayMode uchar ConfigMode4ServerName[] = "";

//------------------------------------------------------------------------------------------------//