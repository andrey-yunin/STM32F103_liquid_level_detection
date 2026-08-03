/*
 * lld_controller.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 *
 *
 *  Конечный автомат LLD: IDLE → ARMED → TRIGGERED → IDLE.
 */

#ifndef INC_LLD_CONTROLLER_H_
#define INC_LLD_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

#include "lld_filter.h"

typedef enum {
    LLD_STATE_IDLE = 0,
    LLD_STATE_ARMED,
    LLD_STATE_TRIGGERED,
    LLD_STATE_ERROR
} LldState_t;

typedef struct {
    LldState_t       state;
    LldFilterStatus_t filter;
} LldControllerStatus_t;

void LLD_Controller_Init(void);
bool LLD_Controller_HandleCommand(uint16_t cmd_code);
bool LLD_Controller_ProcessSample(LldControllerStatus_t *out_trigger);
void LLD_Controller_GetStatus(LldControllerStatus_t *out);


#endif /* INC_LLD_CONTROLLER_H_ */
