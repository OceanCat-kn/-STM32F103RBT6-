#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*
 * FreeRTOSConfig.h
 * 针对 STM32F103（Cortex-M3，72MHz）的完整配置
 * 工程：开机动画 + 开机音乐双任务
 *
 * 放置位置：和 main.c 同一目录即可
 */

/* ============================================================
   1. 基础内核配置
   ============================================================ */

/* 使用抢占式调度（高优先级任务可打断低优先级任务） */
#define configUSE_PREEMPTION                    1

/* 同优先级任务之间使用时间片轮转 */
#define configUSE_TIME_SLICING                  1

/* CPU主频，必须与实际一致！F103=72MHz，F407=168MHz */
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 72000000 )

/* Tick频率：1000 = 每1ms产生一次系统节拍（vTaskDelay(1)=1ms） */
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )

/* 最大任务优先级数量（0最低，数字越大优先级越高） */
#define configMAX_PRIORITIES                    ( 5 )

/* 空闲任务栈大小（单位：word，1word=4字节） */
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )

/* FreeRTOS堆大小：10KB，够用（动画+音乐+主逻辑三个任务） */
/* 如果编译报 RAM 不够，改小到 6144 */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 6 * 1024 ) )

/* 任务名最大长度 */
#define configMAX_TASK_NAME_LEN                 ( 16 )

/* 使用32位Tick计数器（推荐，防止49天溢出问题） */
#define configUSE_16_BIT_TICKS                  0

/* 空闲任务让出CPU给同优先级的用户任务 */
#define configIDLE_SHOULD_YIELD                 1

/* ============================================================
   2. 钩子函数开关
   ============================================================ */

/* Tick钩子：每次系统节拍调用 vApplicationTickHook()
   已在 main.c 中定义，用来调 HAL_IncTick() */
#define configUSE_TICK_HOOK                     1

/* 空闲钩子：关闭（不需要低功耗处理） */
#define configUSE_IDLE_HOOK                     0

/* 栈溢出检测：2=最严格检测，调试阶段务必开启！
   上线后可改为0关闭以节省性能 */
#define configCHECK_FOR_STACK_OVERFLOW          2

/* 堆内存分配失败钩子 */
#define configUSE_MALLOC_FAILED_HOOK            1

/* ============================================================
   3. 互斥量 / 信号量 / 队列
   ============================================================ */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               8

/* ============================================================
   4. 软件定时器（暂时关闭，节省RAM）
   ============================================================ */
#define configUSE_TIMERS                        0
#define configTIMER_TASK_PRIORITY               ( 3 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE * 2 )

/* ============================================================
   5. 调试 / Trace（关闭，节省资源）
   ============================================================ */
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configGENERATE_RUN_TIME_STATS           0

/* ============================================================
   6. 中断优先级配置（Cortex-M3，STM32F103）
   ★ 这部分配错会直接导致 HardFault，请仔细核对！
   ============================================================ */

/* STM32F103 的 NVIC 优先级位数 = 4位（0~15共16个优先级） */
#define configPRIO_BITS                         4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* FreeRTOS内核使用的最低中断优先级 */
#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* 可在ISR中调用FreeRTOS API的最高优先级
   优先级0~4的中断不受FreeRTOS管理
   优先级5~15的中断内可调用 xxxFromISR() 系列函数 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* ============================================================
   7. 中断向量重定向
   ★ 必须配置！否则编译报重复定义错误
   ★ 同时要注释掉 stm32f10x_it.c 中对应的三个函数！
   ============================================================ */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* ============================================================
   8. INCLUDE 配置（开启需要用到的API）
   ============================================================ */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetHandle                  1

#endif /* FREERTOS_CONFIG_H */
