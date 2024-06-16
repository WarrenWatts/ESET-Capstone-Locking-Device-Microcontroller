/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: gpioTask.c
** --------
** Configures necessary GPIO for Solenoid lock 
** and creates a task that controls the locks
** activation.
*/

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

/* Local Headers */
#include "main.h"
#include "gpioTask.h"
#include "espnowTask.h"



/* Variable Naming Abbreviations Legend:
**
** Sem - Semaphore
** Mtx - Mutex
** Rtrn - Return
** Len - Length
** 
*/



/* Local Defines */
#define TAG_LEN 9

#define SET_HIGH 1
#define SET_LOW 0

#define LOCK_MASK (1ULL << GPIO_NUM_41)
#define LOCK_PIN GPIO_NUM_41

/* Local Function Declarations */
static void xLockTask(void *pvParameters);

/* FreeRTOS Local API Handles */
static SemaphoreHandle_t xSemLock;

/* Local String Constants */
static const char TAG[TAG_LEN] = "ESP_GPIO";



/* The startGpioConfig() function is used to initialize the
** singular GPIO pin that controls the retracting and engaging
** of the Solenoid lock. Due to its small size, a separate RTOS
** config function was not created, and the synchronization
** Semaphore for the locking mechanism and the Lock Task were created
** here.
**
** Parameters:
**  none
**
** Return:
**  none
*/
void startGpioConfig(void)
{
    gpio_reset_pin(GPIO_NUM_41); /* Wipes any previous configuration held by pin */
    gpio_set_direction(GPIO_NUM_41, GPIO_MODE_OUTPUT);

    if(!(xSemLock = xSemaphoreCreateBinary()))
    {
        ESP_LOGE(TAG, "%s xSemLock%s", heapFail, rtrnNewLine);
    }

    xTaskCreate(&xLockTask, "LOCK_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



/* The giveSemLock() function simply gives the
** synchronizing Semaphore xSemLock. This function 
** was created for the sake of abstraction and to prevent
** the need to share global variables. 
**
** Parameters:
**  none
**
** Return:
**  none
*/
void giveSemLock(void)
{
    xSemaphoreGive(xSemLock);
}




/* The xLockTask() function simply pends on a Semaphore
** indefinitely. Once a Semaphore has been given, it retracts
** the Solenoid, waits for a period, then re-engages the solenoid. 
**
** Parameters:
**  none
**
** Return:
**  none
*/
static void xLockTask(void *pvParameters)
{
    while(true)
    {
        xSemaphoreTake(xSemLock, portMAX_DELAY);

        gpio_set_level(LOCK_PIN, SET_HIGH);
        vTaskDelay(LOCK_DELAY);
        gpio_set_level(LOCK_PIN, SET_LOW);

        setSpamGuard(true);
    }
}