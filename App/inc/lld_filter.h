/*
 * ldd_filter.h
 *
 *  Created on: Jul 17, 2026
 *      Author: andrey
 *
 *  Дифференциальный фильтр LLD.
 *  F_diff = F₂ − F₁, адаптивная база, детекция касания.
 */


#ifndef INC_LLD_FILTER_H_
#define INC_LLD_FILTER_H_

#include <stdint.h>
#include <stdbool.h>

#include "lld_timer_driver.h"

#define LLD_FILTER_THRESHOLD_HZ    25000   // порог delta для детекции касания
#define LLD_FILTER_CONFIRM_COUNT   3       // количество подтверждений подряд
#define LLD_FILTER_BASE_ALPHA      90      // адаптация базы: new_base = (old * 90 + f_diff * 10) / 100

typedef struct {
    int32_t f1_hz;
    int32_t f2_hz;
    int32_t f_diff_hz;
    int32_t delta_hz;
    int32_t baseline_hz;
} LldFilterStatus_t;

void     LLD_Filter_Init(void);
void     LLD_Filter_SetBaseline(void);
bool     LLD_Filter_Process(const LldTimerSample_t *sample);
int32_t  LLD_Filter_GetDelta(void);
void     LLD_Filter_GetStatus(LldFilterStatus_t *out);


#endif /* INC_LLD_FILTER_H_ */
