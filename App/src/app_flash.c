/*
 * app_flash.c
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */


#include <string.h>
#include "app_flash.h"

// --- UID: 96-битный идентификатор STM32F103 ---
void AppConfig_GetMCU_UID(uint8_t *out_uid)
{
    if (out_uid == NULL) return;
    uint8_t *uid_base = (uint8_t *)0x1FFFF7E8;
    memcpy(out_uid, uid_base, 12);
}
