#ifndef __DUOJI_CONTAL_H__
#define __DUOJI_CONTAL_H__
/*
开发板设备		所连接的管脚
CE						PC5
CSN						PC4
SCK						PA7
MOSI					PA6
MISO					PA5
IRQ						PA4
已知芯片的管脚是多功能复用型的管脚
芯片的管脚会有 输入功能 输出功能 复用功能 模拟功能
我们这次操作的是开发板上的无线通信模块Nrf24L01
已知该模块的接口是SPI接口,我们需要把模块接入到有SPI接口的管脚上
但是为了考虑到移植性的问题,我们还是采取使用GPIO的方式模拟SPI协议
*/
#include "stm32f10x_conf.h"//包含了该头文件所有的外设头文件均已包含
//让Nrf模块之间通信(一次发送数据<=32字节)
//用下面的这个结构体(32字节)进行通信
//里面的成员的含义自己说了算


struct doujizhungtai
{
	short ax, ay, az, gx, gy, gz;
	float pitch, roll, yaw, temp;
	u16 servo1_angle, servo2_angle;
	char buf[64];
};

void duoji_congtal(u8 ret,struct doujizhungtai T_data);


#endif