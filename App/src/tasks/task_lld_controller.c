/*
 * task_lld_controller.c
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 *
 *
 *  Задача LLD Controller (доменный уровень).
 *  Принимает команды ARM/DISARM/GET_STATUS из lld_queue,
 *  опрашивает TIM3, фильтрует, отправляет LLD_TOUCH_EVENT.
 */

#include "tasks/task_lld_controller.h"

#include "cmsis_os.h"
#include "app_queues.h"
#include "can_protocol.h"
#include "task_watchdog.h"
#include "lld_controller.h"
#include "app_config.h"

#define LLD_POLL_INTERVAL_MS        5   // 5 ms между опросами таймера

void app_start_task_lld_controller(void *argument)
{
    (void)argument;

    LLD_Controller_Init();

    LldCommand_t lld_cmd;

    for (;;) {
        AppWatchdog_Heartbeat(APP_WDG_CLIENT_LLD);

        // 1. Проверка очереди команд (ожидание с таймаутом)
        if (osMessageQueueGet(lld_queueHandle, &lld_cmd, NULL, LLD_POLL_INTERVAL_MS) == osOK) {

            switch (lld_cmd.cmd_code) {

            case CAN_CMD_LLD_ARM:
                if (LLD_Controller_HandleCommand(CAN_CMD_LLD_ARM)) {
                    CAN_SendDone(CAN_CMD_LLD_ARM, CAN_DEVICE_TYPE_LLD);
                } else {
                    CAN_SendNackPublic(CAN_CMD_LLD_ARM, CAN_ERR_LLD_BUSY);
                }
                break;

            case CAN_CMD_LLD_DISARM:
                LLD_Controller_HandleCommand(CAN_CMD_LLD_DISARM);
                CAN_SendDone(CAN_CMD_LLD_DISARM, CAN_DEVICE_TYPE_LLD);
                break;

            case CAN_CMD_LLD_GET_STATUS: {
                LldControllerStatus_t status;
                LLD_Controller_GetStatus(&status);

                uint8_t buf[17];
                buf[0]  = (uint8_t)(status.filter.f1_hz & 0xFF);
                buf[1]  = (uint8_t)((status.filter.f1_hz >> 8) & 0xFF);
                buf[2]  = (uint8_t)((status.filter.f1_hz >> 16) & 0xFF);
                buf[3]  = (uint8_t)((status.filter.f1_hz >> 24) & 0xFF);
                buf[4]  = (uint8_t)(status.filter.f2_hz & 0xFF);
                buf[5]  = (uint8_t)((status.filter.f2_hz >> 8) & 0xFF);
                buf[6]  = (uint8_t)((status.filter.f2_hz >> 16) & 0xFF);
                buf[7]  = (uint8_t)((status.filter.f2_hz >> 24) & 0xFF);
                buf[8]  = (uint8_t)(status.filter.f_diff_hz & 0xFF);
                buf[9]  = (uint8_t)((status.filter.f_diff_hz >> 8) & 0xFF);
                buf[10] = (uint8_t)((status.filter.f_diff_hz >> 16) & 0xFF);
                buf[11] = (uint8_t)((status.filter.f_diff_hz >> 24) & 0xFF);
                buf[12] = (uint8_t)(status.filter.baseline_hz & 0xFF);
                buf[13] = (uint8_t)((status.filter.baseline_hz >> 8) & 0xFF);
                buf[14] = (uint8_t)((status.filter.baseline_hz >> 16) & 0xFF);
                buf[15] = (uint8_t)((status.filter.baseline_hz >> 24) & 0xFF);
                buf[16] = (uint8_t)status.state;

                CAN_SendDataFragmented(CAN_CMD_LLD_GET_STATUS, buf, 17);
                CAN_SendDone(CAN_CMD_LLD_GET_STATUS, CAN_DEVICE_TYPE_LLD);
                break;
            }

            default:
                break;
            }
        }

        // 2. Если вооружены — опрос семпла
        LldControllerStatus_t trigger;
        if (LLD_Controller_ProcessSample(&trigger)) {
            // DATA-контракт TOUCH_EVENT (0x0704), одиночный DATA-фрейм:
            //   byte 0..3: f_diff_hz (int32 LE) на момент триггера
            //   byte 4:    state (uint8) = LLD_STATE_TRIGGERED
            //   byte 5:    reserved = 0
            uint8_t data[6] = {0};
            data[0] = (uint8_t)(trigger.filter.f_diff_hz & 0xFF);
            data[1] = (uint8_t)((trigger.filter.f_diff_hz >> 8) & 0xFF);
            data[2] = (uint8_t)((trigger.filter.f_diff_hz >> 16) & 0xFF);
            data[3] = (uint8_t)((trigger.filter.f_diff_hz >> 24) & 0xFF);
            data[4] = (uint8_t)trigger.state;

            CAN_SendData(CAN_CMD_LLD_TOUCH_EVENT, data, 6);
            CAN_SendDone(CAN_CMD_LLD_TOUCH_EVENT, CAN_DEVICE_TYPE_LLD);
        }
    }
}


