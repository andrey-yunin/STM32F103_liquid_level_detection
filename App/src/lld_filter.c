/*
 * ldd_filter.c
 *
 *  Created on: Jul 17, 2026
 *      Author: andrey
 *
 *
 *      *  Дифференциальный фильтр LLD.
 *  Вычисляет F_diff = F₂ − F₁, обновляет адаптивную базу,
 *  детектирует касание при delta > порога N раз подряд.
 */

#include <stddef.h>

#include "lld_filter.h"

#define DIV_ROUND(num, den)   (((num) + ((den) / 2)) / (den))

static int32_t s_baseline_hz;
static int32_t s_f_diff_hz;
static int32_t s_f1_hz;
static int32_t s_f2_hz;
static uint32_t s_confirm;

void LLD_Filter_Init(void)
{
    s_baseline_hz = 0;
    s_f_diff_hz = 0;
    s_f1_hz = 0;
    s_f2_hz = 0;
    s_confirm = 0;
}

void LLD_Filter_SetBaseline(void)
{
    s_baseline_hz = s_f_diff_hz;
    s_confirm = 0;
}

bool LLD_Filter_Process(const LldTimerSample_t *sample)
{
    if (sample == NULL) return false;

    if (sample->period_ticks_1 == 0 || sample->period_ticks_2 == 0) {
        return false;
    }

    // частота = 64,000,000 / период (с округлением)
    s_f1_hz = (int32_t)DIV_ROUND(LLD_TIMER_CLOCK_HZ, sample->period_ticks_1);
    s_f2_hz = (int32_t)DIV_ROUND(LLD_TIMER_CLOCK_HZ, sample->period_ticks_2);
    s_f_diff_hz = s_f2_hz - s_f1_hz;

    // первая итерация — захват базы
    if (s_baseline_hz == 0) {
        s_baseline_hz = s_f_diff_hz;
        return false;
    }

    int32_t delta = s_f_diff_hz - s_baseline_hz;

    if (delta < 0) delta = -delta;

    if (delta > LLD_FILTER_THRESHOLD_HZ) {
        s_confirm++;
        if (s_confirm >= LLD_FILTER_CONFIRM_COUNT) {
            return true;    // касание подтверждено
        }
    } else {
        s_confirm = 0;
        // адаптация базы
        s_baseline_hz = (s_baseline_hz * LLD_FILTER_BASE_ALPHA +
                         s_f_diff_hz * (100 - LLD_FILTER_BASE_ALPHA)) / 100;
    }

    return false;
}

int32_t LLD_Filter_GetDelta(void)
{
    return s_f_diff_hz - s_baseline_hz;
}

void LLD_Filter_GetStatus(LldFilterStatus_t *out)
{
    if (out == NULL) return;
    out->f1_hz       = s_f1_hz;
    out->f2_hz       = s_f2_hz;
    out->f_diff_hz   = s_f_diff_hz;
    out->baseline_hz = s_baseline_hz;
    out->delta_hz    = s_f_diff_hz - s_baseline_hz;
}



