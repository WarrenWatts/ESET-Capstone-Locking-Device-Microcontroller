/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: gpioTask.h
** --------
** Header file for gpioTask.c. Provides function
** declarations for the configuration function
** and a Semaphore giver function.
*/

#ifndef GPIOTASK_H_
#define GPIOTASK_H_

/* Standard Library Headers */
#include <stdio.h>

/* RTOS Headers */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Driver Headers */
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"


/* Function Declarations */
extern void startGpioConfig(void);
extern void giveSemLock(void);


#endif /* GPIOTASK_H_ */