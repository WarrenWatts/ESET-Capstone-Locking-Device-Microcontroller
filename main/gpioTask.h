/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: gpioTask.h
** --------
** .......
*/

#ifndef GPIOTASK_H_
#define GPIOTASK_H_

/* Standard Library Headers */
#include <stdio.h>

/* RTOS Headers */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Driver Headers */
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

#define SET_HIGH 1
#define SET_LOW 1

#define LOCK_MASK (1ULL << GPIO_NUM_41)
#define LOCK_PIN GPIO_NUM_41 // Could optimize by placing defines within one another if you get what I'm saying...

extern SemaphoreHandle_t xLockSemaphore;

extern void startGpioConfig(void);


#endif /* GPIOTASK_H_ */