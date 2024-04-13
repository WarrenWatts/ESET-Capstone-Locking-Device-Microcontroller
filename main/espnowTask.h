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

#define INIT_DELAY pdMS_TO_TICKS(5000)
#define SEND_DELAY pdMS_TO_TICKS(100)
#define ALIVE_PEND pdMS_TO_TICKS(15000)
#define DEF_CHNL 11

typedef void (*espnowFuncPtr)(void);

extern void startEspnowConfig(void);

#endif /* ESPNOWTASK_H_ */