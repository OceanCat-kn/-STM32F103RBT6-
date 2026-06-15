#include "delay.h"
#include "dht.h"
#include "ldt.h"
#include "eint.h"
//#include "iwdg.h"
#include "usart1.h"
#include "stdio.h"
#include "math.h"
#include "oled.h"
#include "mpu6050.h"
#include "buzzer.h"
//#include "led.h"
#include "duo.h"  // 包含舵机驱动函数声明

// 定义姿态角到舵机角度的映射参数
#define GYRO_ROLL_MIN    -90.0f   // 陀螺仪roll最小角度
#define GYRO_ROLL_MAX     90.0f   // 陀螺仪roll最大角度
#define GYRO_PITCH_MIN   -90.0f   // 陀螺仪pitch最小角度
#define GYRO_PITCH_MAX    90.0f   // 陀螺仪pitch最大角度
#define SERVO_MIN_ANGLE    0      // 舵机最小角度
#define SERVO_MAX_ANGLE  180      // 舵机最大角度

// 结构体：存储传感器数据
struct data
{
	int dht_hum;
	int dht_temp;
	float sht_hum;
	float sht_temp;
	u8 buf[16];
};

// 串口指令解析标志位
#define O_FLAG 0
#define N_FLAG 1
#define F_FLAG 2
int flag = O_FLAG;

// 串口接收回调函数（保留原有逻辑）
void recv_handler(u8 recv)
{
	switch(flag)
	{
		case O_FLAG : 
			if(recv=='O') flag=N_FLAG;
			break;
		case N_FLAG :
			if(recv=='N')
			{
				buzzer_on();
				flag=O_FLAG;
			}
			else if(recv=='F')
				flag=F_FLAG;
			else
				flag=O_FLAG;
			break;
		case F_FLAG : 
			if(recv=='F') buzzer_off();
			flag=O_FLAG;
			break;
	}
}

// 姿态角映射为舵机角度（核心函数）
// input_val: 陀螺仪原始角度  min_in: 输入最小值  max_in: 输入最大值
// min_out: 输出最小值  max_out: 输出最大值
u16 Map_Angle(float input_val, float min_in, float max_in, u16 min_out, u16 max_out)
{
	float ratio;
	u16 output_val;
	
	// 1. 限制输入值范围（防止超出陀螺仪量程）
	if(input_val < min_in) input_val = min_in;
	if(input_val > max_in) input_val = max_in;
	
	// 2. 计算映射比例
	ratio = (input_val - min_in) / (max_in - min_in);
	
	// 3. 映射为舵机角度
	output_val = min_out + (u16)(ratio * (max_out - min_out));
	
	// 4. 舵机角度保护
	if(output_val < min_out) output_val = min_out;
	if(output_val > max_out) output_val = max_out;
	
	return output_val;
}

void fengzhungduoji(int flag_duoji)
{
	short ax, ay, az, gx, gy, gz;
	float pitch, roll, yaw, temp;
	char buf[64];
	u16 servo1_angle, servo2_angle;  // 舵机1（PB8）、舵机2（PB9）目标角度

	// 1. 初始化所有外设
//	led_init();
	buzzer_init();
//	button_init();
	delay_init();
	dht_init();
	ldt_init(); 
	eint_init();
	usart_1_init();
	OLED_Init();
	MPU_Init();                // 初始化陀螺仪
	Servo_PWM_Init();          // 初始化舵机PWM（新增）
	
	// 2. 舵机初始归中（90度）
	Servo_Set_Angle(90);
	delay_ms(100);
	Servo2_Set_Angle(90);
	delay_ms(100);
	
	OLED_Clear();
	set_usart1_handler(recv_handler);  // 设置串口回调函数
	
	while(1)  
	{
		if(flag_duoji==1)
			return ;
		// 3. 读取陀螺仪原始数据
		MPU_Get_Accelerometer(&ax, &ay, &az);  // 读取加速度计数据
		MPU_Get_Gyroscope(&gx, &gy, &gz);      // 读取陀螺仪数据
		temp = MPU_Get_Temperature() / 100.0;  // 读取温度
		
		// 4. 计算姿态角（基于加速度计，简单稳定）
		// roll: 横滚角（左右倾斜）→ 控制舵机1
		roll = atan2(ax, sqrt(ay*ay + az*az)) * 180 / 3.14159;
		// pitch: 俯仰角（前后倾斜）→ 控制舵机2
		pitch = atan2(ay, az) * 180 / 3.14159;
		// yaw: 偏航角（旋转），暂不使用
		yaw = atan2(gz, sqrt(gx*gx + gy*gy)) * 180 / 3.14159;
		
		// 5. 姿态角映射为舵机角度（核心控制逻辑）
		// roll(-90~90°) → 舵机1(0~180°)
		servo1_angle = Map_Angle(roll, GYRO_ROLL_MIN, GYRO_ROLL_MAX, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
		// pitch(-90~90°) → 舵机2(0~180°)
		servo2_angle = Map_Angle(pitch, GYRO_PITCH_MIN, GYRO_PITCH_MAX, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
		
		// 6. 驱动舵机转动到目标角度
		Servo_Set_Angle(servo1_angle);   // 控制舵机1（PB8）
		Servo2_Set_Angle(servo2_angle);  // 控制舵机2（PB9）
		
		// 7. OLED显示（新增舵机角度显示）
		OLED_Set_Pos(0, 0);
		sprintf((char *)buf, "PITCH:%+06.2f", pitch);
		OLED_ShowString(0, 0, (u8 *)buf, 16);
		
		OLED_Set_Pos(0, 2);
		sprintf((char *)buf, "ROLL: %+06.2f", roll);
		OLED_ShowString(0, 2, (u8 *)buf, 16);
		
		OLED_Set_Pos(0, 4);
		sprintf((char *)buf, "S1:%3d S2:%3d", servo1_angle, servo2_angle);  // 显示舵机角度
		OLED_ShowString(0, 4, (u8 *)buf, 16);
		
		OLED_Set_Pos(0, 6);
		sprintf((char *)buf, "TEMP: %+06.2f", temp);
		OLED_ShowString(0, 6, (u8 *)buf, 16);
		
		// 8. 串口打印（新增舵机角度）
		printf("PITCH:%+06.2f ROLL:%+06.2f S1:%d S2:%d TEMP:%+06.2f\n", 
		       pitch, roll, servo1_angle, servo2_angle, temp);
		
		delay_ms(10);  // 降低循环频率，避免舵机抖动（可根据需求调整）
	}
}

