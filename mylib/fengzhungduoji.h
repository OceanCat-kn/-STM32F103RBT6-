#ifndef __FENGZHUNGDUOJI_H
#define __FENGZHUNGDUOJI_H

#include "stm32f10x.h"


// 定义姿态角到舵机角度的映射参数
#define GYRO_ROLL_MIN    -90.0f   // 陀螺仪roll最小角度
#define GYRO_ROLL_MAX     90.0f   // 陀螺仪roll最大角度
#define GYRO_PITCH_MIN   -90.0f   // 陀螺仪pitch最小角度
#define GYRO_PITCH_MAX    90.0f   // 陀螺仪pitch最大角度
#define SERVO_MIN_ANGLE    0      // 舵机最小角度
#define SERVO_MAX_ANGLE  180      // 舵机最大角度



void fengzhungduoji(int);
void recv_handler(u8 recv);
u16 Map_Angle(float input_val, float min_in, float max_in, u16 min_out, u16 max_out);


#endif
