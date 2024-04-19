/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: gpioTask.c
** --------
** .......
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



/* Local Defines */
#define TAG_LEN 9

/* Local Function Declarations */
static void lockTask(void *pvParameters);

/* FreeRTOS Defining API Handles */
SemaphoreHandle_t xSemLock;

/* Reference Declarations of Global Constant Strings */
extern const char rtrnNewLine[NEWLINE_LEN];
extern const char heapFail[HEAP_LEN];
extern const char mtxFail[MTX_LEN];

/* Local String Constants */
static const char TAG[TAG_LEN] = "ESP_GPIO";



void startGpioConfig(void)
{
    gpio_reset_pin(GPIO_NUM_41);
    gpio_set_direction(GPIO_NUM_41, GPIO_MODE_OUTPUT);

    if(!(xSemLock = xSemaphoreCreateBinary()))
    {
        ESP_LOGE(TAG, "%s xSemLock%s", heapFail, rtrnNewLine);
    }

    xTaskCreate(&lockTask, "LOCK_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



static void lockTask(void *pvParameters)
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