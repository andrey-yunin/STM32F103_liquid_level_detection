/*
 * lld_controller.c
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 *
 *
 *  Конечный автомат LLD.
 *  IDLE: ожидание ARM.
 *  ARMED: захват частоты, фильтрация, детекция касания.
 *  TRIGGERED: касание подтверждено, ожидание DISARM.
 *  ERROR: сбой.
 */

#include <stddef.h>

#include "lld_controller.h"

#include "can_protocol.h"
#include "lld_timer_driver.h"

static LldState_t s_state;

void LLD_Controller_Init(void)
{
    s_state = LLD_STATE_IDLE;
    LLD_Timer_Init();
    LLD_Filter_Init();
}

bool LLD_Controller_HandleCommand(uint16_t cmd_code)
{
    switch (cmd_code) {

    case CAN_CMD_LLD_ARM:
        if (s_state != LLD_STATE_IDLE) return false;
        LLD_Filter_Init();
        LLD_Timer_Start();
        s_state = LLD_STATE_ARMED;
        return true;

    case CAN_CMD_LLD_DISARM:
        if (s_state != LLD_STATE_ARMED &&
            s_state != LLD_STATE_TRIGGERED) return false;
        LLD_Timer_Stop();
        s_state = LLD_STATE_IDLE;
        return true;

    default:
        return false;
    }
}

bool LLD_Controller_ProcessSample(void)
{
    if (s_state != LLD_STATE_ARMED) return false;

    LldTimerSample_t sample;
    if (!LLD_Timer_GetSample(&sample)) return false;

    if (LLD_Filter_Process(&sample)) {
        LLD_Timer_Stop();
        s_state = LLD_STATE_TRIGGERED;
        return true;
    }

    return false;
}

void LLD_Controller_GetStatus(LldControllerStatus_t *out)
{
    if (out == NULL) return;
    out->state = s_state;
    LLD_Filter_GetStatus(&out->filter);
}



