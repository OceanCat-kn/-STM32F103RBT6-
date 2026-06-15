#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"

/* ============================================================
   音符频率定义 (Hz)
   使用TIM3_CH2 PWM输出，PA7引脚接蜂鸣器
   ============================================================ */

#define NOTE_G3   196 
#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_A4   440
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_D5   587
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_G5   784
#define NOTE_A5   880
#define NOTE_REST 0     // 休止符

/* 音符时长定义 (ms) */
#define BPM         120             // 每分钟120拍
#define BEAT        (60000 / BPM)   // 一拍 = 500ms
#define NOTE_WHOLE  (BEAT * 4)      // 全音符
#define NOTE_HALF   (BEAT * 2)      // 二分音符
#define NOTE_QUART  (BEAT)          // 四分音符
#define NOTE_8TH    (BEAT / 2)      // 八分音符
#define NOTE_16TH   (BEAT / 4)      // 十六分音符

/* 音符结构体 */
typedef struct {
    uint32_t freq;      // 频率(Hz)，0=休止符
    uint32_t duration;  // 时长(ms)
} Note_t;

/* 函数声明 */
void Buzzer_Init(void);
void Buzzer_PlayTone(uint32_t freq, uint32_t duration_ms);
void Buzzer_Stop(void);

#endif /* __BUZZER_H */
