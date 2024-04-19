/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: espnowTask.h
** --------
** .......
*/

#ifndef ESPNOWTASK_H_
#define ESPNOWTASK_H_

/* Standard Library Headers */
#include <stdio.h>

/* RTOS Headers */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Driver Headers */
#include "esp_log.h"
#include "esp_err.h"



/* Defines */
#define DEF_DELAY pdMS_TO_TICKS(15000)
#define SCAN_DELAY pdMS_TO_TICKS(0)
#define SEND_WAIT pdMS_TO_TICKS(250)
#define DEF_PEN pdMS_TO_TICKS(10)
#define DEF_CHNL 11

/* Function Declarations */
extern void startEspnowConfig(void);
extern void setSpamGuard(bool spamGuardVal);

/* Typedefs for Pointer to Function */
typedef void (*espnowFuncPtr)(void);

#endif /* ESPNOWTASK_H_ */