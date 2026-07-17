/*
 * lld_timer_driver.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_LLD_TIMER_DRIVER_H_
#define INC_LLD_TIMER_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

#define LLD_TIMER_CLOCK_HZ         64000000  // 64 MHz (prescaler 0, HSI 64 MHz)

// --- Результат одного измерения (период в тиках таймера) ---
typedef struct {
    uint32_t period_ticks_1;   // период канала 1 (игла)
    uint32_t period_ticks_2;   // период канала 2 (опора)
} LldTimerSample_t;

void LLD_Timer_Init(void);
void LLD_Timer_Start(void);
void LLD_Timer_Stop(void);
bool LLD_Timer_GetSample(LldTimerSample_t *out);


#endif /* INC_LLD_TIMER_DRIVER_H_ */
