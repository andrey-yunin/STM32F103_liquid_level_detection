/*
 * lld_timer_driver.c
 *
 *  Created on: Jul 16, 2026
 *
 *  Author: andrey
 *
 *  TIM3 dual input capture: захват периода F₁ (CH1, PA6) и F₂ (CH2, PA7).
 *  Вызов HAL_TIM_IC_CaptureCallback из TIM3_IRQHandler.
 */

#include "lld_timer_driver.h"

#include "main.h"

extern TIM_HandleTypeDef htim3;

// --- Состояние захвата ---
static volatile uint16_t s_last_capture[2];
static volatile uint32_t s_period[2];
static volatile bool     s_updated[2];

// --- HAL callback: захват периода ---
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM3) return;

    uint32_t channel;

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        channel = 0;
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        channel = 1;
    } else {
        return;
    }

    uint16_t ccr = (uint16_t)HAL_TIM_ReadCapturedValue(htim, (channel == 0) ? TIM_CHANNEL_1 : TIM_CHANNEL_2);
    uint16_t last = s_last_capture[channel];
    uint16_t period = ccr - last;       // 16-bit unsigned wrap корректна

    s_last_capture[channel] = ccr;
    s_period[channel] = (period > 0) ? (uint32_t)period : 1;
    s_updated[channel] = true;
}

// --- Инициализация ---
// CubeMX уже настроил TIM3 (MX_TIM3_Init). Здесь только сброс состояния.
void LLD_Timer_Init(void)
{
    s_last_capture[0] = 0;
    s_last_capture[1] = 0;
    s_period[0] = 0;
    s_period[1] = 0;
    s_updated[0] = false;
    s_updated[1] = false;
}

// --- Запуск захвата на обоих каналах ---
void LLD_Timer_Start(void)
{
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
}

// --- Остановка захвата ---
void LLD_Timer_Stop(void)
{
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
}

// --- Чтение последнего измерения ---
bool LLD_Timer_GetSample(LldTimerSample_t *out)
{
    if (out == NULL) return false;

    if (!s_updated[0] || !s_updated[1]) return false;

    out->period_ticks_1 = s_period[0];
    out->period_ticks_2 = s_period[1];

    s_updated[0] = false;
    s_updated[1] = false;

    return true;
}


