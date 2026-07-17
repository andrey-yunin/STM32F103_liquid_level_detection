/*
 * app_flash.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_APP_FLASH_H_
#define INC_APP_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

#define APP_FLASH_CONFIG_PAGE      0x0800FC00  // последняя страница (1KB)
#define APP_FLASH_MAGIC            0x240A240A  // маркер валидности

// Структура конфигурационной страницы.
// Выровнена под 4 байта для Flash-записи.
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  node_id;
    uint8_t  reserved[251];
} AppFlashConfig_t;

bool    AppFlash_IsValid(void);
void    AppFlash_ReadConfig(AppFlashConfig_t *out);
bool    AppFlash_WriteNodeId(uint8_t node_id);
void    AppFlash_RestoreDefaults(void);

void AppConfig_GetMCU_UID(uint8_t *out_uid);

#endif /* INC_APP_FLASH_H_ */
