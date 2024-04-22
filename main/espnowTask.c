/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: espnowTask.c
** --------
** Configures ESP-NOW functionality and creates a task to send
** ESP-NOW frames and a callback function that help to determine which kind
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
static void xChnlCtrlTask(void *pvParameters);
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

/* Variable to prevent spam of ESP-NOW Frames via Release Button */
static bool spamGuard = true;



/* The startEspnowConfig() function is used to initialize ESP-NOW
** functionality for the ESP32.
**
** Parameters:
**  none
**
** Return:
**  none
**
** Notes: ESP-NOW does not need a Wi-Fi connection, but it does
** utilize Wi-Fi RF bands for communication, so Wi-Fi must be started.
*/
void startEspnowConfig(void)
{
    /* Non-volatile Storage Initalization*/
    ESP_ERROR_CHECK(nvs_flash_init());

    /* TCP/IP Stack Initialization */
    ESP_ERROR_CHECK(esp_netif_init());
    
    /* Creates Default Event Loop Task (Task Priority: 20)*/
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifiInit = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInit));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); /* Wi-Fi Station */
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(DEF_CHNL, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_now_init());
    esp_now_register_recv_cb(espnowRcvCallback);

    startEspnowRtosConfig();

    espnowPeerConfig();
}



/* The startEspnowRtosConfig() function is used to initialize the
** Semaphores and Mutexes that are used by the xChnlCtrlTask and its
** associated functions. The xChnlCtrlTask is also created here.
**
** Parameters:
**  none
**
** Return:
**  none
*/
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
    
    xTaskCreate(&xChnlCtrlTask, "CTRL_TASK", STACK_DEPTH, 0, BASE_PRIO, 0);
}



/* The espnowPeerConfig() function is used to both
** initialize and modify the peer configuration. More
** specifically, the modification of the Wi-Fi channel that
** the device is transmitting ESP-NOW frames to.
**
** Parameters:
**  none
**
** Return:
**  none
**
** Notes: For transmission to be successful between two ESPs,
** they must be transmitted/receiving on the same Wi-Fi channel.
** It is also the case that the Wi-Fi channel being used currently
** to transmit must match the Wi-Fi channel you have configured
** for your peer to receive on, otherwise an error will occur.
** Since Wi-Fi networks can be dynamic in nature, changing the
** channels that a device is communicating on forcefully, a singular
** function that handled both the initial configuration of the peer
** and its modification was created.
*/
static void espnowPeerConfig(void)
{
    /* Mutex to guard transmitterInfo Variable */
    if(xSemaphoreTake(xMtxRcvInfo, DEF_PEND))
    {
        static bool initialized = false;

        uint8_t primaryChnl = 0;
        wifi_second_chan_t secondaryChnl = 0;

        esp_wifi_get_channel(&primaryChnl, &secondaryChnl);
        transmitterInfo.channel = primaryChnl;

        /* To prevent double initialization */
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



/* The espnowRcvCallback() function handles the frame data
** sent from the Input Device Microcontroller. If a char of
** '1' is received, then the solenoid is being to told to activate.
** If a char of '2' is received, then a "alive status" frame was
** successfully received by the Input Device Microcontroller, and the
** xSemEspnowEvent Semaphore is given for synchronization. 
**
** Parameters:
**  espnowInfo - ESP-NOW packet information struct
**  myData - string data received from ESP-NOW frame
**  dataLen - length of the data
**
** Return:
**  none
** 
** Notes: The usage of the global spamGuard variable makes its appearance here.
** In order to prevent the spamming of the release button to cause more than one
** Semaphore to be given after an ESP-NOW packet has been successfully received,
** its wrapper functions are utilized here. A value of false translates to
** disallowing the "giving" of the xSemLock Semaphore, since false in an if statement
** will fail.
*/
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



/* The changeChnl() function handles the changing/setting of
** Wi-Fi channels upon failure to successfully send an ESP-NOW packet
** (and therefore receiving an "alive status" frame) after a number
** of send attempts. The function to change the peer's channel is also
** called here.
**
** Parameters:
**  none
**
** Return:
**  none
** 
** Notes: The possible Wi-Fi communication channels are 1 through 11.
** A static variable is created that increments through these channels
** until the correct channel is located. Once the value of this variable
** exceed 11, it is reset to 0.
*/
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



/* The setSpamGuard() function is the setter function for the
** spamGuard variable, whose purpose is explained in the notes
** of the espnowRcvCallback() function.
**
** Parameters:
**  spamGuardVal - the Boolean value to set the spamGuard variable with
**
** Return:
**  none
*/
void setSpamGuard(bool spamGuardVal)
{
    /* Mutex to Guard spamGuard Variable */
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



/* The getSpamGuard() function is the getter function for the
** spamGuard variable, whose purpose is explained in the notes
** of the espnowRcvCallback() function.
**
** Parameters:
**  none
**
** Return:
**  A Boolean representing if the spamGuard is in use (which equates to false)
*/
static bool getSpamGuard(void)
{
    bool spamGuardVal = false;

    /* Mutex to Guard spamGuard Variable */
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



/* The xChnlCtrlTask() function controls the transmission of 
** ESP-NOW "alive status" frames, their rate of transmission,
** and changing of channels.
**
** Parameters:
**  none used
**
** Return:
**  none
**
** Notes: If the Semaphore fails to be taken within the SEND_WAIT
** period, then we failed to make a connection and we increment our
** attempt counter. If our attempt counter reaches the max amount of
** attempts, we change the channel. The connected Boolean variable
** is used to determine the delay between "alive status" frames.
*/
static void xChnlCtrlTask(void *pvParameters)
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
