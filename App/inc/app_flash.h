/*
 * app_flash.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_APP_FLASH_H_
#define INC_APP_FLASH_H_

#include "app_config.h"
#include <stdint.h>
#include <stdbool.h>

// --- Адрес конфигурационной страницы (последняя страница, 1KB) ---
#define APP_CONFIG_FLASH_ADDR     0x0800FC00
#define APP_CONFIG_MAGIC          0x55AAEEFF  // канон экосистемы

// --- NodeID по умолчанию (заводские настройки): адрес LLD 0x70 ---
#define APP_CONFIG_DEFAULT_NODE_ID  CAN_NODE_ID  // из app_config.h = 0x70

// --- Структура конфигурации (канон: magic + performer_id + доменные поля + CRC16) ---
// Размер 12 байт, кратен 4 — запись во Flash словами uint32_t.
typedef struct {
    uint32_t magic;              // Маркер валидности (APP_CONFIG_MAGIC)
    uint8_t  performer_id;       // CAN NodeID платы (8-битный)
    uint8_t  reserved_bytes[5];  // Резерв под доменные поля LLD (пороги, калибровка)
    uint16_t checksum;           // CRC16-CCITT Modbus (Poly 0xA001) — ВСЕГДА последним
} AppConfig_t;

// --- Публичный API ---
void     AppConfig_GetMCU_UID(uint8_t *out_uid);   // 96-битный UID (0x1FFFF7E8)
void     AppConfig_Init(void);                      // загрузка из Flash / заводские + mutex
uint32_t AppConfig_GetPerformerID(void);            // чтение NodeID (RAM)
void     AppConfig_SetPerformerID(uint32_t id);     // запись NodeID (RAM-only)
bool     AppConfig_Commit(void);                    // RAM -> Flash (атомарно)
void     AppConfig_FactoryReset(void);              // стирание страницы конфигурации

#endif /* INC_APP_FLASH_H_ */
