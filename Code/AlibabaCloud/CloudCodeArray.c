#include "CloudCodeArray.h"
//------------------------------------------------------------------------------------------------//
//########Cloud����########//
code uchar DeviceName[] = "DS18B20";
code uchar ProductKey[] = "a1ci94s55wO";
code uchar WiFiSSID[] = "TP-LINK_830A";
code uchar WiFiPassword[] = "zdwzxy27";
code uchar MQTTUser[] = "DS18B20&a1ci94s55wO";
code uchar MQTTPassword[] = "bb1155f896fd6dcc27d5513ccdf3bda60b072aa8";
code uchar MQTTHost[] = "a1ci94s55wO.iot-as-mqtt.cn-shanghai.aliyuncs.com";
code uchar MQTTPort[] = "1883";
code uchar MQTTClientID[] = "123456|securemode=3\\,signmethod=hmacsha1\\,timestamp=789|";
code uchar MQTTHeartBeat[] = "60";
code uchar MQTTAutoReConect[] = "ON";
code uchar MQTTAutoStart[] = "OFF";
//------------------------------------------------------------------------------------------------//
//########Publish�¼�����########//
code uchar Event_1[] = "WaterTemperatureLow";  //�¼�1,PubCode=1
code uchar Event_2[] = "WaterTemperatureHigh"; //�¼�2,PubCode=2
code uchar Event_1_Len = 19;
code uchar Event_2_Len = 20;
//########Publish��������########//
code uchar Service_1[] = "LCD1602Display"; //�¼�1,PubCode=200
code uchar Service_1_Len = 14;
//------------------------------------------------------------------------------------------------//
//AT���
code uchar ATCmd0[] = "MQTTAUTH";
code uchar ATCmd1[] = "MQTTSOCK";
code uchar ATCmd2[] = "MQTTCID";
code uchar ATCmd3[] = "MQTTKEEPALIVE";
code uchar ATCmd4[] = "MQTTRECONN";
code uchar ATCmd5[] = "MQTTAUTOSTART";
code uchar ATCmd6[] = "MQTTSTART";
code uchar ATCmd7[] = "MQTTSUB";
code uchar ATCmd8[] = "MQTTPUB";
code uchar ATCmd9[] = "MQTTSEND";
code uchar ATCmd10[] = "MQTTUNSUB";
code uchar ATCmd11[] = "MQTTSTATUS";
code uchar ATCmd12[] = "MQTTCLOSE";
code uchar ATCmd13[] = "WJAP";
code uchar ATCmd14[] = "REBOOT";
code uchar ATCmd15[] = "WJAPQ";
code uchar *ATCmd[] = {ATCmd0, ATCmd1, ATCmd2, ATCmd3, ATCmd4, ATCmd5, ATCmd6, ATCmd7, ATCmd8, ATCmd9,
                       ATCmd10, ATCmd11, ATCmd12, ATCmd13, ATCmd14,ATCmd15};
//------------------------------------------------------------------------------------------------//