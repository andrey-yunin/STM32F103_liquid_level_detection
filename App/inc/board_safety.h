/*
 * board_safety.h
 *
 *  Created on: Jul 16, 2026
 *      Author: andrey
 */

#ifndef INC_BOARD_SAFETY_H_
#define INC_BOARD_SAFETY_H_

// Переводит GPIO в безопасное состояние.
// Должен быть вызываем из startup, fault-обработчиков и Error_Handler —
// без FreeRTOS, CAN, Flash или других зависимостей.
void Board_EnterSafeState(void);

#endif /* INC_BOARD_SAFETY_H_ */
