/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: main.c
** --------
** .......
*/

/* Standard Library Headers */
#include <stdio.h>

/* RTOS Headers */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Driver Headers */
#include "esp_log.h"
#include "esp_err.h"

/* Local Headers */
#include "main.h"
#include "espnowTask.h"
#include "gpioTask.h"



const char heapFail[28] = "Insufficient heap space for"; // FIXME (MAGIC NUMBER)
const char mtxFail[25] = "Mutex failed to take key"; // FIXME (MAGIC NUMBER)
const char rtrnNewLine[3] = "\r\n"; // FIXME (MAGIC NUMBER)



void app_main(void)
{
    startGpioConfig();
    startEspnowConfig();
}
