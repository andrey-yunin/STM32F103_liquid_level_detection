/*
 * app_flash.c
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */


#include "app_flash.h"
#include "app_config.h"   // CAN_NODE_ID
#include "main.h"         // HAL_FLASH_*
#include "cmsis_os.h"     // osMutex
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// --- Инкапсулированные данные (скрыты внутри модуля) ---
static AppConfig_t g_app_config;
static osMutexId_t configMutex = NULL;

// --- Атрибуты мьютекса: recursive + prio inherit (канон экосистемы) ---
const osMutexAttr_t configMutex_attr = {
    "configMutex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};


// --- Внутренние вспомогательные функции ---

// CRC16-CCITT Modbus (Poly 0xA001). Считается только по полям ДО checksum.
static uint16_t CalculateChecksum(AppConfig_t *cfg)
{
    uint16_t crc = 0xFFFF;
    const uint8_t *p = (const uint8_t *)cfg;
    const uint32_t len = (uint32_t)offsetof(AppConfig_t, checksum);

    for (uint32_t i = 0; i < len; i++) {
        crc ^= p[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if ((crc & 0x0001U) != 0U) crc = (crc >> 1) ^ 0xA001U;
            else crc >>= 1;
        }
    }
    return crc;
}


// --- UID: 96-битный идентификатор STM32F103 ---
void AppConfig_GetMCU_UID(uint8_t *out_uid)
{
    if (out_uid == NULL) return;
    uint8_t *uid_base = (uint8_t *)0x1FFFF7E8;
    memcpy(out_uid, uid_base, 12);
}


// --- Публичный API с защитой Mutex ---

void AppConfig_Init(void)
{
    // 1. Создание мьютекса
    if (configMutex == NULL) {
        configMutex = osMutexNew(&configMutex_attr);
    }

    // 2. Маппинг конфигурационной страницы
    AppConfig_t *flash_cfg = (AppConfig_t *)APP_CONFIG_FLASH_ADDR;

    // 3. Валидация по Magic Key и CRC16
    if (flash_cfg->magic == APP_CONFIG_MAGIC &&
        flash_cfg->checksum == CalculateChecksum(flash_cfg)) {
        memcpy(&g_app_config, flash_cfg, sizeof(AppConfig_t));
    } else {
        // Заводские настройки в RAM (Flash не перезаписываем)
        memset(&g_app_config, 0, sizeof(AppConfig_t));
        g_app_config.magic = APP_CONFIG_MAGIC;
        g_app_config.performer_id = APP_CONFIG_DEFAULT_NODE_ID;
        g_app_config.checksum = CalculateChecksum(&g_app_config);
    }
}


// Физическое стирание страницы конфигурации
void AppConfig_FactoryReset(void)
{
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = APP_CONFIG_FLASH_ADDR;
    EraseInitStruct.NbPages     = 1;
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_FLASH_Lock();
}

uint32_t AppConfig_GetPerformerID(void)
{
    uint32_t id = APP_CONFIG_DEFAULT_NODE_ID;
    if (osMutexAcquire(configMutex, osWaitForever) == osOK) {
        id = g_app_config.performer_id;
        osMutexRelease(configMutex);
    }
    return id;
}


// Безопасная запись NodeID ТОЛЬКО в RAM (сохранение — через COMMIT)
void AppConfig_SetPerformerID(uint32_t id)
{
    if (osMutexAcquire(configMutex, osWaitForever) == osOK) {
        g_app_config.performer_id = (uint8_t)id;
        osMutexRelease(configMutex);
    }
}


// Сохранение RAM -> Flash (атомарная транзакция: стереть -> записать словами)
bool AppConfig_Commit(void)
{
    bool success = false;
    HAL_StatusTypeDef status = HAL_ERROR;
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    // 1. Мьютекс: во время записи никто не меняет g_app_config
    if (osMutexAcquire(configMutex, osWaitForever) == osOK) {

        // 2. Актуальная контрольная сумма перед записью
        g_app_config.checksum = CalculateChecksum(&g_app_config);

        // 3. Разблокировка Flash
        HAL_FLASH_Unlock();

        // 4. Стирание страницы (биты только 1->0, нужно обнулить до 0xFF)
        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.PageAddress = APP_CONFIG_FLASH_ADDR;
        EraseInitStruct.NbPages     = 1;
        status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

        // 5. Запись по 32-битным словам (12 байт = 3 слова)
        if (status == HAL_OK) {
            uint32_t *pData = (uint32_t *)&g_app_config;
            uint32_t addr = APP_CONFIG_FLASH_ADDR;
            for (uint32_t i = 0; i < sizeof(AppConfig_t); i += 4) {
                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *pData);
                if (status != HAL_OK) break;
                addr += 4;
                pData++;
            }
        }

        // 6. Блокировка Flash
        HAL_FLASH_Lock();

        success = (status == HAL_OK);
        osMutexRelease(configMutex);
    }
    return success;
}








