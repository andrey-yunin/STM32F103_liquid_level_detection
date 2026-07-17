/*
 * app_config.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_APP_CONFIG_H_
#define INC_APP_CONFIG_H_

#include "main.h"

// --- Версия прошивки ---
#define FW_REV_MAJOR                0x01
#define FW_REV_MINOR                0x00
#define FW_REV_PATCH                0x00

// --- Идентификация на шине CAN ---
// NodeID из диапазона 0x7x (dds240_global_config.h: CAN_ADDR_LLD_BOARD=0x70)
#define CAN_NODE_ID                 0x70
#define CAN_DATA_MAX_LEN            8

// --- Очереди ---
#define CAN_RX_QUEUE_LEN            16
#define CAN_TX_QUEUE_LEN            16
#define DISPATCHER_QUEUE_LEN        8
#define LLD_QUEUE_LEN               4

// --- Флаги уведомлений задач ---
#define FLAG_CAN_RX                 0x01
#define FLAG_CAN_TX                 0x02

// --- Типы CAN-фреймов ---
typedef struct {
    CAN_RxHeaderTypeDef header;
    uint8_t data[CAN_DATA_MAX_LEN];
} CanRxFrame_t;

typedef struct {
    CAN_TxHeaderTypeDef header;
    uint8_t data[CAN_DATA_MAX_LEN];
} CanTxFrame_t;

// --- Разобранная команда (dispatcher) ---
typedef struct {
    uint16_t cmd_code;
    uint8_t  device_id;
    uint8_t  data[5];
    uint8_t  data_len;
} ParsedCanCommand_t;

// --- Команда для очереди LLD Controller ---
typedef struct {
    uint16_t cmd_code;
    uint8_t  data[5];
    uint8_t  data_len;
} LldCommand_t;

#endif /* INC_APP_CONFIG_H_ */
