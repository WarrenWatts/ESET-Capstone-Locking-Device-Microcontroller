/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: espnowTask.c
** --------
** .......
*/

/* Standard Library Headers */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* RTOS Headers */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Driver Headers */
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_err.h"

/* Local Headers */
#include "main.h"
#include "espnowTask.h"
#include "gpioTask.h"



SemaphoreHandle_t xEspnowEventSem;
SemaphoreHandle_t xLockSemaphore;

static SemaphoreHandle_t xRcvInfoMutex;

void changeChannel(void);
void checkChannelStatus(void);

static void espnowPeerConfig(void);
static void startEspnowRtosConfig(void);
static void chnlCtrlTask(void *pvParameters);

void espNowSendCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
void espNowRcvCallback(const esp_now_recv_info_t* espNowInfo, const uint8_t *myData, int dateLen);


static const espnowFuncPtr stateArr[2] = // FIXME (MAGIC NUMBER)
{
    changeChannel,
    checkChannelStatus,
};


static const uint8_t msgData[2] = "1"; //FIXME (MAGIC NUMBER)
static const char TAG[9] = "ESP_GPIO"; // FIXME (MAGIC NUMBER)

extern const char rtrnNewLine[3]; // FIXME (MAGIC NUMBER)
extern const char heapFail[28]; // FIXME (MAGIC NUMBER)
extern const char mtxFail[25]; // FIXME (MAGIC NUMBER)

static esp_now_peer_info_t transmitterInfo;
static const uint8_t transmitterMAC[ESP_NOW_ETH_ALEN] = {0x7C, 0xDF, 0xA1, 0xE5, 0x44, 0x30};



void startEspnowConfig(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifiInit = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInit));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(DEF_CHNL, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_now_init());
    esp_now_register_recv_cb(espNowRcvCallback);
    esp_now_register_send_cb(espNowSendCallback);

    espnowPeerConfig();

    startEspnowRtosConfig();
}



static void espnowPeerConfig(void)
{
    if(xSemaphoreTake(xRcvInfoMutex, DEF_PEND))
    {
        static bool initialized = false;

        uint8_t primaryChnl = 0;
        uint8_t *primaryChnlPtr = &primaryChnl;
        wifi_second_chan_t secondaryChnl = 0;
        wifi_second_chan_t *secondaryChnlPtr = &secondaryChnl;

        esp_wifi_get_channel(primaryChnlPtr, secondaryChnlPtr);
        transmitterInfo.channel = primaryChnl;

        if(!initialized)
        {
            initialized = true;

            transmitterInfo.ifidx = ESP_IF_WIFI_STA;
            transmitterInfo.encrypt = false;
            memcpy(transmitterInfo.peer_addr, transmitterMAC, ESP_NOW_ETH_ALEN);

            esp_now_add_peer(&transmitterInfo);
        }
        else
        {
            esp_now_mod_peer(&transmitterInfo);
        }
        xSemaphoreGive(xRcvInfoMutex);
    }
    else
    {
        ESP_LOGW(TAG, "%s Mutex%s", heapFail, rtrnNewLine);
    }
}



void espNowRcvCallback(const esp_now_recv_info_t* espNowInfo, const uint8_t *myData, int dateLen)
{
    xSemaphoreGive(xLockSemaphore);
}



void espNowSendCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if(status)
    {
        xSemaphoreGive(xEspnowEventSem);
    }
    else
    {
        xSemaphoreTake(xEspnowEventSem, NO_WAIT);
    }
}



void startEspnowRtosConfig(void)
{
    if(!(xEspnowEventSem = xSemaphoreCreateCounting(1, 0))) // FIXME (MAGIC NUMBER)
    {
        ESP_LOGW(TAG, "%s Semaphore%s", heapFail, rtrnNewLine); // FIXME 
    }

    if(!(xRcvInfoMutex = xSemaphoreCreateMutex()))
    {
        ESP_LOGW(TAG, "%s Mutex%s", heapFail, rtrnNewLine); // FIXME
    }
    
    xTaskCreate(&chnlCtrlTask, "CTRL_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



void changeChannel(void)
{
    static uint8_t chnlCount = 0;

    if(chnlCount < 11)
    {
        chnlCount = 0;
    }

    ESP_ERROR_CHECK(esp_wifi_set_channel(chnlCount, WIFI_SECOND_CHAN_NONE));

    espnowPeerConfig();

    vTaskDelay(SEND_DELAY);
    esp_now_send(transmitterMAC, msgData, 2); // FIXME (MAGIC NUMBER)

    chnlCount++;
}



void checkChannelStatus(void)
{
    vTaskDelay(ALIVE_PEND);
    esp_now_send(transmitterMAC, msgData, 2); // FIXME (MAGIC NUMBER)
}



static void chnlCtrlTask(void *pvParameters)
{
    vTaskDelay(INIT_DELAY);

    while(true)
    {
        int semVal = 0;
        semVal = uxSemaphoreGetCount(xEspnowEventSem);

        stateArr[semVal]();
    }
}
