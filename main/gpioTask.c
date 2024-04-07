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



static void lockTask(void *pvParameters);

SemaphoreHandle_t xLockSemaphore;

static const char TAG[9] = "ESP_GPIO"; // FIXME (MAGIC NUMBER)

extern const char rtrnNewLine[3]; // FIXME (MAGIC NUMBER)
extern const char heapFail[28]; // FIXME (MAGIC NUMBER)
extern const char mtxFail[25]; // FIXME (MAGIC NUMBER)



void startGpioConfig(void)
{
    gpio_config_t lockConfig;

    lockConfig.pin_bit_mask = LOCK_MASK;
    lockConfig.mode = GPIO_MODE_OUTPUT;
    lockConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    lockConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    lockConfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&lockConfig);

    if(!(xLockSemaphore = xSemaphoreCreateBinary()))
    {
        ESP_LOGW(TAG, "%s Semaphore%s", heapFail, rtrnNewLine);
    }

    xTaskCreate(&lockTask, "LOCK_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



static void lockTask(void *pvParameters)
{
    while(true)
    {
        xSemaphoreTake(xLockSemaphore, portMAX_DELAY);

        gpio_set_level(LOCK_PIN, SET_HIGH);
        vTaskDelay(LOCK_DELAY);
        gpio_set_level(LOCK_PIN, SET_LOW);
    }
}