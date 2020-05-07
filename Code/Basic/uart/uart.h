#ifndef _uart_H_
#define _uart_H_
#include "MCUdef.h"
#include "pindef.h"
//------------------------------------------------------------------------------------------------//
//���ڽ��ջ�������С
#define uart2_buffer_size 600
#define uart4_buffer_size 32
//------------------------------------------------------------------------------------------------//
extern bool uart1Busy,uart2Busy,uart4Busy;//���ڷ�����æ��־

extern xdata uchar uart2Buffer[];//����2���ջ�����
extern data ushort uart2Idx1,uart2Idx2;//���ڻ���������
extern xdata uchar uart4Buffer[];//����4���ջ�����
extern data uchar uart4Idx1,uart4Idx2;//���ڻ���������
//------------------------------------------------------------------------------------------------//
void uart1_send8(uchar dat);
void uart2_send8(uchar dat);
void uart2_sendstr8(uchar *p);
void uart4_send8(uchar dat);
//------------------------------------------------------------------------------------------------//
#endif