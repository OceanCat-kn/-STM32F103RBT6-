	#include "duoji_congtal.h"
	#include "nrf24l01.h"
	#include "led.h"
	#include "oled.h"
	#include "stdio.h"
	
	#include "duoji_congtal.h"
#include "fengzhungduoji.h"
#include "math.h"
#include "duo.h"
	
void duoji_congtal(u8 ret,struct doujizhungtai T_data)
{	
		struct nrf_msg_st msg;
		ret = NRF_Rx_Dat((u8 *)&T_data, sizeof(msg));//接收数据
		led_on(2);//0号LED灯亮
		if(ret == RX_DR)//判断是否接到数据
		{
			
			// 7. OLED显示（新增舵机角度显示）
		OLED_Set_Pos(0, 0);
		sprintf((char *)T_data.buf, "PITCH:%+06.2f", T_data.pitch);
		OLED_ShowString(0, 1, (u8 *)T_data.buf, 16);
		
		OLED_Set_Pos(0, 2);
		sprintf((char *)T_data.buf, "ROLL: %+06.2f", T_data.roll);
		OLED_ShowString(0, 3, (u8 *)T_data.buf, 16);
		
		OLED_Set_Pos(0, 4);
		sprintf((char *)T_data.buf, "S1:%3d S2:%3d", T_data.servo1_angle, T_data.servo2_angle);  // 显示舵机角度
		OLED_ShowString(0, 5, (u8 *)T_data.buf, 14);
		
		OLED_Set_Pos(0, 6);
		sprintf((char *)T_data.buf, "TEMP: %+06.2f",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                T_data.temp);
		OLED_ShowString(0, 7, (u8 *)T_data.buf, 12);
		OLED_ShowString(0, 0, (u8 *)"rx bingo", 12);//显示rx bingo提示信息
		} 
		else
		{
			OLED_ShowString(0, 0, (u8 *)"rx error", 12);//显示rx error提示信息
		}
			// 4. 计算姿态角（基于加速度计，简单稳定）
		// roll: 横滚角（左右倾斜）→ 控制舵机1
		T_data.roll = atan2(T_data.ax, sqrt(T_data.ay*T_data.ay + T_data.az*T_data.az)) * 180 / 3.14159;
		// pitch: 俯仰角（前后倾斜）→ 控制舵机2
		T_data.pitch = atan2(T_data.ay, T_data.az) * 180 / 3.14159;
		// yaw: 偏航角（旋转），暂不使用
		T_data.yaw = atan2(T_data.gz, sqrt(T_data.gx*T_data.gx + T_data.gy*T_data.gy)) * 180 / 3.14159;
		
		// 5. 姿态角映射为舵机角度（核心控制逻辑）
		// roll(-90~90°) → 舵机1(0~180°)
		T_data.servo1_angle = Map_Angle(T_data.roll, GYRO_ROLL_MIN, GYRO_ROLL_MAX, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
		// pitch(-90~90°) → 舵机2(0~180°)
		T_data.servo2_angle = Map_Angle(T_data.pitch, GYRO_PITCH_MIN, GYRO_PITCH_MAX, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
		
		// 6. 驱动舵机转动到目标角度
		Servo_Set_Angle(T_data.servo1_angle);   // 控制舵机1（PB8）
		Servo2_Set_Angle(T_data.servo2_angle);  // 控制舵机2（PB9）
//		if(K3_MODE==0)
//		{
//			led_off(2);
//			OLED_Clear();
//			show_main_menu();
//			break;
		}

	