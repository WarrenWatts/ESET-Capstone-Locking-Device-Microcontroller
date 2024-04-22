/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: main.c
** --------
** Main function and configuring of all ESP and
** FreeRTOS functionality.
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



/* Variable Naming Abbreviations Legend:
**
** Mtx - Mutex
** Rtrn - Return
** Len - Length
** 
*/



/* Defining Declarations of Global Constant Strings */
const char heapFail[HEAP_LEN] = "Insufficient heap space for";
const char mtxFail[MTX_LEN] = "Mutex failed to take key";
const char rtrnNewLine[NEWLINE_LEN] = "\r\n";



/* The app_main() function acts as the main() function for
** ESP-IDF based projects. This function is utilized to 
** configure all necessary ESP and FreeRTOS functionality
** for this program.
**
** Parameters:
**  none
**
** Return:
**  none
*/
void app_main(void)
{
    startGpioConfig();
    startEspnowConfig();
}
