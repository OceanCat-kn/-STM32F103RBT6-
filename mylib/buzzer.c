#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"

/*
 * 硬件连接说明：
 *   蜂鸣器正极 → PA7 (TIM3_CH2)
 *   蜂鸣器负极 → GND
 *
 * TIM3配置：
 *   APB1时钟 = 36MHz（STM32F103标准配置）
 *   TIM3_CLK = 72MHz（APB1×2）
 *   预分频PSC = 71，计数时钟 = 1MHz
 *   ARR根据频率动态设置：ARR = 1000000 / freq - 1
 *   CCR = ARR / 2（50%占空比，音量最大）
 */

/* ============================================================
   初始化TIM3_CH2为PWM输出（PA7）
   ============================================================ */
void Buzzer_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef       TIM_OCInitStruct;

    /* 1. 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. PA7配置为复用推挽输出 */
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 3. TIM3时基配置
     *    PSC=71 → 计数时钟 = 72MHz / (71+1) = 1MHz
     *    ARR先设1000（1kHz默认），后面动态改 */
    TIM_TimeBaseStruct.TIM_Period        = 999;
    TIM_TimeBaseStruct.TIM_Prescaler     = 71;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);

    /* 4. TIM3 CH2 PWM模式1配置 */
    TIM_OCInitStruct.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse       = 499;   // 50%占空比
    TIM_OCInitStruct.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC2Init(TIM3, &TIM_OCInitStruct);

    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    /* 5. 先关闭，等任务启动后再播放 */
    TIM_Cmd(TIM3, DISABLE);
}

/* ============================================================
   播放单个音符
   freq        : 频率(Hz)，传NOTE_REST(0)则静音
   duration_ms : 持续时间(ms)
   ★ 必须在FreeRTOS任务中调用（用vTaskDelay实现非阻塞等待）
   ============================================================ */
void Buzzer_PlayTone(uint32_t freq, uint32_t duration_ms)
{
    if (freq == NOTE_REST || freq == 0)
    {
        /* 休止符：关闭PWM输出 */
        TIM_Cmd(TIM3, DISABLE);
    }
    else
    {
        /* 根据频率计算ARR：计数时钟1MHz，ARR = 1MHz / freq - 1 */
        uint32_t arr = (1000000UL / freq) - 1;
        uint32_t ccr = arr / 2;  // 50%占空比

        TIM_Cmd(TIM3, DISABLE);                        // 先停止，防止毛刺
        TIM_SetAutoreload(TIM3, arr);                  // 更新ARR（频率）
        TIM_SetCompare2(TIM3, ccr);                    // 更新CCR（占空比）
        TIM3->CNT = 0;                                 // 计数器清零
        TIM_Cmd(TIM3, ENABLE);                         // 启动输出
    }

    /* 用FreeRTOS延时，不会阻塞其他任务（动画照常跑） */
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

/* ============================================================
   停止蜂鸣器
   ============================================================ */
void Buzzer_Stop(void)
{
    TIM_Cmd(TIM3, DISABLE);
}
