#ifndef _pindef_H_
#define _pindef_H_
//Pin Define 引脚名定义
#include <STC8.h>
//------------------------------------------------------------------------------------------------//

//LED
sbit LED_BLUE = P0 ^ 7;
sbit LED_RED = P1 ^ 0;
sbit LED_GREEN = P1 ^ 1;

//Gizwits-ESP8266
sbit ESP8266_RST = P3 ^ 6;

//DS18B20
sbit DS18B20_DATA = P4 ^ 1;

//LCD1602
#define LCD1602_DATA P2
sbit LCD1602_RS = P3 ^ 5;
sbit LCD1602_RW = P3 ^ 4;
sbit LCD1602_EN = P3 ^ 2;
sbit LCD1602_Busy = LCD1602_DATA ^ 7;

//------------------------------------------------------------------------------------------------//
#endif