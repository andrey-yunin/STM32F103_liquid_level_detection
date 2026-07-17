/*
 * app_queues.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_APP_QUEUES_H_
#define INC_APP_QUEUES_H_

#include "cmsis_os.h"

// --- Очереди межзадачного обмена ---
// can_rx_queue — сырые фреймы из CAN ISR в task_can_handler
// can_tx_queue — исходящие фреймы от задач в task_can_handler
// dispatcher_queue — разобранные команды в task_dispatcher
// lld_queue — LLD-доменные команды в task_lld_controller
extern osMessageQueueId_t can_rx_queueHandle;
extern osMessageQueueId_t can_tx_queueHandle;
extern osMessageQueueId_t dispatcher_queueHandle;
extern osMessageQueueId_t lld_queueHandle;

// --- Хендлы задач для уведомлений ---
extern osThreadId_t task_can_handleHandle;
extern osThreadId_t task_lld_controHandle;


#endif /* INC_APP_QUEUES_H_ */
