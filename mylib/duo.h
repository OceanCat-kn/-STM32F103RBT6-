#ifndef __DUO_H
#define __DUO_H

#include "stm32f10x.h"

void Servo_PWM_Init(void);
void Servo_Set_Angle(u16 angle); 
void Servo2_Set_Angle(u16 angle);

#endif