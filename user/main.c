#include "led.h"
#include "buzzer.h"
#include "button.h"
#include "delay.h"
#include "ldt.h"
#include "dht.h"
#include "eint.h"
#include "iwdg.h"
#include "usart1.h"
#include "stdio.h"
#include "oled.h"
#include "servo_contal.h"
#include "nrf24l01.h"
#include "duoji_congtal.h"
#include "fengzhungduoji.h"
#include "math.h"

/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"




/* ============================================================
   UI状态机定义（保持原有不变）
   ============================================================ */
typedef enum
{
    UI_MENU = 0,
    UI_TEMP,
    UI_CALC,
    UI_DOU,
    UI_MB,
    UI_NL,
    UI_MAX
} UI;

typedef enum
{
    SURE_TEMP = 1,
    SURE_CALC,
    SURE_DOU,
    SURE_MB,
    SURE_NL,
    SURE_MAX
} SURE;

static int UI_index  = 0;
static int func_active = 0;

static void show_main_menu(void);
static void SHOW_FUNC_UI(int);
static void show_temp_page(u8 hum, u8 temp);
static void show_calc_page(void);
static void show_gyro_page(void);

int k3_h3 = 0;

void h0(void)
{
    if (func_active == 0) {
        UI_index++;
        if (UI_index >= SURE_MAX)
            UI_index = 1;
        OLED_Clear();
        show_main_menu();
    }
}

int K3_MODE = 0;

void h1(void)
{
    k3_h3 = (k3_h3 + 1) % 2;
    func_active = k3_h3;

    if (k3_h3) {
        led_on(2);
        if (UI_index == 3) {
            OLED_Clear();
            K3_MODE = 1;
        }
        SHOW_FUNC_UI(UI_index);
    } else {
        if (UI_index == 3) {
            K3_MODE = 0;
            return;
        }
        OLED_Clear();
        show_main_menu();
        led_off(2);
    }
}

void h2(void)
{
    K3_MODE = 0;
    show_main_menu();
}

struct doujizhungtai T_data;
u8 ret;

/* ============================================================
   开机音乐谱 —— 《两只老虎》（欢快，适合开机）
   格式：{频率Hz, 时长ms}
   ============================================================ */
static const Note_t bootMusic[] = {
    /* 两只老虎，两只老虎 */
    {NOTE_C4, 400}, {NOTE_D4, 400}, {NOTE_E4, 400}, {NOTE_C4, 400},
    {NOTE_C4, 400}, {NOTE_D4, 400}, {NOTE_E4, 400}, {NOTE_C4, 400},
    /* 跑得快，跑得快 */
    {NOTE_E4, 400}, {NOTE_F4, 400}, {NOTE_G4, 800},
    {NOTE_E4, 400}, {NOTE_F4, 400}, {NOTE_G4, 800},
    /* 一只没有眼睛 */
    {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_G4, 200}, {NOTE_F4, 200},
    {NOTE_E4, 400}, {NOTE_C4, 400},
    /* 一只没有尾巴 */
    {NOTE_G4, 200}, {NOTE_A4, 200}, {NOTE_G4, 200}, {NOTE_F4, 200},
    {NOTE_E4, 400}, {NOTE_C4, 400},
    /* 真奇怪，真奇怪 */
    {NOTE_D4, 400}, {NOTE_G3, 400}, {NOTE_C4, 800},  /* G3=196Hz */
    {NOTE_D4, 400}, {NOTE_G3, 400}, {NOTE_C4, 800},
    /* 结尾上扬音效 */
    {NOTE_C4, 150}, {NOTE_E4, 150}, {NOTE_G4, 150}, {NOTE_C5, 400},
    {NOTE_REST, 200},
};
#define MUSIC_LEN  (sizeof(bootMusic) / sizeof(Note_t))

/* 注：NOTE_G3 未在buzzer.h中定义，此处直接写196 */
#define NOTE_G3  196

/* ============================================================
   FreeRTOS 任务句柄
   ============================================================ */
static TaskHandle_t xAnimTask  = NULL;
static TaskHandle_t xMusicTask = NULL;

/* ============================================================
   任务1：开机动画
   原有逻辑：picture_1~7 循环3次，最后显示picture_0
   全部改用 vTaskDelay，不再用 delay_ms
   ============================================================ */
static void AnimationTask(void *pvParameters)
{
    /* 播放3轮帧动画 */
    for (int round = 0; round < 3; round++)
    {
        picture_1(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_2(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_3(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_4(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_5(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_6(); vTaskDelay(pdMS_TO_TICKS(80));
        picture_7(); vTaskDelay(pdMS_TO_TICKS(80));
    }

    /* 显示最终开机画面，停留1秒 */
    picture_0();
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* 进入主菜单 */
    OLED_Clear();
    show_main_menu();

    /* 任务完成，自删除 */
    vTaskDelete(NULL);
}

/* ============================================================
   任务2：开机音乐
   遍历音符表，用 Buzzer_PlayTone 播放
   PlayTone 内部已用 vTaskDelay，不会阻塞动画任务
   ============================================================ */
static void MusicTask(void *pvParameters)
{
    for (uint32_t i = 0; i < MUSIC_LEN; i++)
    {
        Buzzer_PlayTone(bootMusic[i].freq, bootMusic[i].duration);
    }
    Buzzer_Stop();

    /* 任务完成，自删除 */
    vTaskDelete(NULL);
}

/* ============================================================
   任务3：主逻辑任务（原 while(1) 主循环）
   等待开机动画/音乐任务自然结束后，接管主控制权
   ============================================================ */
static void MainLogicTask(void *pvParameters)
{
    /* 等待动画任务结束（通过检测句柄是否已删除） */
    /* 简单做法：等待足够长的时间（3轮×7帧×80ms + 1000ms ≈ 2700ms） */
    vTaskDelay(pdMS_TO_TICKS(3000));

    /* 主循环：与原 while(1) 完全一致 */
    while (1)
    {
        while (K3_MODE)
        {
            duoji_congtal(ret, T_data);
            if (K3_MODE == 0)
            {
                led_off(2);
                OLED_Clear();
                show_main_menu();
                break;
            }
        }
        /* 让出CPU，避免空转 */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* ============================================================
   FreeRTOS 钩子函数
   ============================================================ */

/* Tick钩子：每1ms调用一次，保证HAL_GetTick()正常工作 */
void vApplicationTickHook(void)
{
    /* 如果你用的是标准库（StdPeriph），这里可以留空 */
    /* 如果用HAL库，取消下面注释：*/
    // HAL_IncTick();
}

/* 栈溢出钩子：调试用，上线可删 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    /* 栈溢出时亮红灯或停机 */
    led_on(1);
    while (1);
}

/* 堆内存不足钩子 */
void vApplicationMallocFailedHook(void)
{
    led_on(1);
    while (1);
}

/* ============================================================
   main 函数
   ============================================================ */
int main(void)
{
    u8 rxAddr[] = {0x10, 0x20, 0x30, 0x40, 0x53};
		led_init();
    led_on(1);          // 亮灯确认跑到这里
    delay_init();
    delay_ms(300);
    led_off(1);
		
    /* ---- 硬件初始化（与原工程完全一致） ---- */
    SPI_NRF_Init();
    led_init();     
    button_init();
    delay_init();
    ldt_init();
    dht_init();
    eint_init();
    usart_1_init();
    OLED_Init();
    OLED_Clear();

    /* ---- 初始化PWM蜂鸣器（TIM3_CH2） ---- */
		Buzzer_Init();      /* ← 新增：初始化TIM3 PWM */

    /* ---- NRF初始化 ---- */
    ret = NRF_Check();
    NRF_RX_Mode(rxAddr, 49, sizeof(struct nrf_msg_st));

    /* ---- 其他外设 ---- */
    GyroServo_Init();
    set_eint_handler(h0, h1, h2);

    /* ---- 创建FreeRTOS任务 ---- */
    /*
     * 任务优先级说明：
     *   AnimationTask : 优先级2，负责刷OLED（I2C较慢，优先保证帧率）
     *   MusicTask     : 优先级2，与动画同优先级，轮流调度
     *   MainLogicTask : 优先级1，最低，等开机完成后接管
     */
    xTaskCreate(AnimationTask,  "Anim",  256, NULL, 2, &xAnimTask);
    xTaskCreate(MusicTask,      "Music", 128, NULL, 2, &xMusicTask);
    xTaskCreate(MainLogicTask,  "Main",  256, NULL, 1, NULL);

//    /* ---- 启动FreeRTOS调度器（不会返回） ---- */
    vTaskStartScheduler();

    /* 永远不会到这里 */
    while (1);
}

/* ============================================================
   以下函数与原工程完全一致，未做任何修改
   ============================================================ */
static void show_main_menu(void)
{
    if (UI_index < 4)
    {
        OLED_ShowCHinese5(36, 0, 4);

        OLED_ShowChar(0, 2, (UI_index == SURE_TEMP) ? '>' : ' ', 16);
        OLED_ShowString(15, 3, (u8 *)"TEMP ", 8);
        OLED_ShowCHinese(15, 2, 5);

        OLED_ShowChar(0, 4, (UI_index == SURE_CALC) ? '>' : ' ', 16);
        OLED_ShowCHinese1(15, 4, 5);

        OLED_ShowChar(0, 6, (UI_index == SURE_DOU) ? '>' : ' ', 16);
        OLED_ShowCHinese2(15, 6, 5);
    }
    else
    {
        OLED_Clear();
        OLED_ShowChar(0, 0, (UI_index == UI_MB) ? '>' : ' ', 16);
        OLED_ShowCHinese3(15, 0, 6);

        OLED_ShowChar(0, 2, (UI_index == UI_NL) ? '>' : ' ', 16);
        OLED_ShowCHinese4(15, 2, 3);
    }
}

static void SHOW_FUNC_UI(int mode)
{
    switch (mode)
    {
        case SURE_TEMP:
            OLED_Clear();
            OLED_ShowString(15, 4, (u8 *)"TEMP PAGE", 8);
            break;
        case SURE_CALC:
            OLED_Clear();
            OLED_ShowString(15, 4, (u8 *)"CALC PAGE", 8);
            break;
        case 4:
            OLED_Clear();
            OLED_ShowString(15, 4, (u8 *)"STOPWATCH", 8);
            break;
        case 5:
            OLED_Clear();
            OLED_ShowString(15, 4, (u8 *)"CALENDAR", 8);
            break;
    }
}

static void show_gyro_page(void)
{
    OLED_Clear();
    OLED_ShowString(16, 2, (u8 *)"GYROSCOPE", 16);
    OLED_ShowString(0, 4, (u8 *)"TODO: FUNC", 16);
}

static void show_calc_page(void)
{
    OLED_Clear();
    OLED_ShowString(16, 2, (u8 *)"CALCULATOR", 16);
    OLED_ShowString(0, 4, (u8 *)"TODO: FUNC", 16);
}

static void show_temp_page(u8 hum, u8 temp)
{
    OLED_Clear();
    OLED_ShowString(16, 2, (u8 *)"TEMPERATURE", 16);
    OLED_ShowString(0, 4, (u8 *)"HUM: %d TEMP: %d", 16);
}
