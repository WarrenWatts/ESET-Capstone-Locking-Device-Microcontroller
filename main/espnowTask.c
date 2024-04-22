/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: espnowTask.c
** --------
** Configures ESP-NOW functionality and creates a task to send
** ESP-NOW messages and a callback function that help to determine which kind
** of message was received (status or activation message), handling them
** accordingly.
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



/* Variable Naming Abbreviations Legend:
**
** Sem - Semaphore
** Mtx - Mutex
** Rtrn - Return
** Len - Length
** Msg - message
** Cnt - Count
** Chnl - Channel
** Rcv - Receive
** 
*/



/* Local Defines */
#define MSG_LEN 2
#define TAG_LEN 9

#define MAX_CNT 1
#define START_CNT 0
#define MAX_ATTMPT 10

#define MAX_CHNL 11
#define BASE_CHNL (MAX_CNT)

/* Local Function Declarations */
void espnowRcvCallback(const esp_now_recv_info_t* espnowInfo, const uint8_t *myData, int dataLen);
static void xchnlCtrlTask(void *pvParameters);
static void startEspnowRtosConfig(void);
static void espnowPeerConfig(void);
static bool getSpamGuard(void);
static void changeChnl(void);

/* FreeRTOS Local API Handles */
static SemaphoreHandle_t xSemEspnowEvent;
static SemaphoreHandle_t xMtxRcvInfo;
static SemaphoreHandle_t xMtxSpamGuard;

/* Reference Declarations of Global Constant Strings */
extern const char rtrnNewLine[NEWLINE_LEN];
extern const char heapFail[HEAP_LEN];
extern const char mtxFail[MTX_LEN];

/* Local String Constants */
static const uint8_t msgData[MSG_LEN] = "1";
static const char TAG[TAG_LEN] = "ESP_GPIO";

/* Local Constant Logging String */
static esp_now_peer_info_t transmitterInfo;

/* Peer Receiver MAC Address String */
static const uint8_t transmitterMAC[ESP_NOW_ETH_ALEN] = {0x7C, 0xDF, 0xA1, 0xE6, 0xE1, 0x71};

/* Variable to prevent spam of ESP-NOW Messages via Release Button */
static bool spamGuard = true;



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
    esp_now_register_recv_cb(espnowRcvCallback);

    startEspnowRtosConfig();

    espnowPeerConfig();
}



void startEspnowRtosConfig(void)
{
    if(!(xSemEspnowEvent = xSemaphoreCreateCounting(MAX_CNT, START_CNT)))
    {
        ESP_LOGE(TAG, "%s xSemEspnowEvent%s", heapFail, rtrnNewLine); 
    }

    if(!(xMtxRcvInfo = xSemaphoreCreateMutex()))
    {
        ESP_LOGE(TAG, "%s xMtxRcvInfo%s", heapFail, rtrnNewLine);
    }

    if(!(xMtxSpamGuard = xSemaphoreCreateMutex()))
    {
        ESP_LOGE(TAG, "%s xMtxSpamGuard%s", heapFail, rtrnNewLine);
    }
    
    xTaskCreate(&xchnlCtrlTask, "CTRL_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



static void espnowPeerConfig(void)
{
    if(xSemaphoreTake(xMtxRcvInfo, DEF_PEND))
    {
        static bool initialized = false;

        uint8_t primaryChnl = 0;
        wifi_second_chan_t secondaryChnl = 0;

        esp_wifi_get_channel(&primaryChnl, &secondaryChnl);
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
        xSemaphoreGive(xMtxRcvInfo);
    }
    else
    {
        ESP_LOGE(TAG, "%s xMtxRcvInfo%s", heapFail, rtrnNewLine);
    }
}



void espnowRcvCallback(const esp_now_recv_info_t* espnowInfo, const uint8_t *myData, int dataLen)
{
    switch(myData[0])
    {
        case '1':
            if(getSpamGuard())
            {
                setSpamGuard(false);
                giveSemLock();
            }
            break;

        case '2':
            xSemaphoreGive(xSemEspnowEvent);
            break;
        
        default:
            break;
    }
}



static void changeChnl(void)
{
    static uint8_t chnlCnt = BASE_CHNL;

    if(chnlCnt > MAX_CHNL)
    {
        chnlCnt = BASE_CHNL;
    }
    ESP_ERROR_CHECK(esp_wifi_set_channel(chnlCnt, WIFI_SECOND_CHAN_NONE));

    espnowPeerConfig();

    chnlCnt++;
}



void setSpamGuard(bool spamGuardVal)
{
    if(xSemaphoreTake(xMtxSpamGuard, DEF_PEND))
    {
        spamGuard = spamGuardVal;
        xSemaphoreGive(xMtxSpamGuard);
    }
    else
    {
        ESP_LOGE(TAG, "xMtxSpamGuard %s%s", mtxFail, rtrnNewLine);
    }
}



static bool getSpamGuard(void)
{
    bool spamGuardVal = false;

    if(xSemaphoreTake(xMtxSpamGuard, DEF_PEND))
    {
        spamGuardVal = spamGuard;
        xSemaphoreGive(xMtxSpamGuard);
    }
    else
    {
        ESP_LOGE(TAG, "xMtxSpamGuard %s%s", mtxFail, rtrnNewLine);
    }

    return spamGuardVal;
}



static void xchnlCtrlTask(void *pvParameters)
{
    bool connected = true;
    uint16_t aliveStatusDelay;
    uint8_t attemptCnt = 0;

    while(true)
    {
        aliveStatusDelay = (connected) ? DEF_DELAY : SCAN_DELAY;
        vTaskDelay(aliveStatusDelay);

        esp_now_send(transmitterMAC, msgData, MSG_LEN);

        if(!xSemaphoreTake(xSemEspnowEvent, SEND_WAIT))
        {
            connected = false;
            if(++attemptCnt == MAX_ATTMPT)
            {
                changeChnl();
            }
            else
            {
                continue;
            }
        }
        else
        {
            connected = true;
        }
        attemptCnt = 0;
    }
}
