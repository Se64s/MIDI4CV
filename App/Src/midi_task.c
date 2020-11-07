/**
 * @file midi_task.c
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Task to handle midi communication and mapping midi commands to system features
 * @version 0.1
 * @date 2020-10-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "midi_task.h"
#include "cli_task.h"
#include "sys_serial.h"
#include "midi_lib.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Midi serial debug */
// #define MIDI_SERIAL_DEBUG

/* Define midi serial interface */
#define MIDI_SERIAL                         SYS_SERIAL_1

/* Initial delay to wait for other task */
#define MIDI_INIT_DELAY                     (100U)

/* Seize of midi task command */
#define MIDI_EVENT_QUEUE_ELEMENT_SIZE       ( sizeof(MidiEvent_t) )

/* Number of commands that can queue the task */
#define MIDI_EVENT_QUEUE_SIZE               (10U)

/* Midi SysEx buffer size */
#define MIDI_SYSEX_BUFF_SIZE                (32U)

/* Private macro -------------------------------------------------------------*/
#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private variables ---------------------------------------------------------*/

/** Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

/** Queue handler */
QueueHandle_t xMidiQueueHandler = NULL;

/** Midi inteface handler */
MidiLibHandler_t xMidiHandler = {0};

/** Buffer for SysEx data */
static uint8_t u8SysExBuff[MIDI_SYSEX_BUFF_SIZE] = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Init resources for CLI tasks
  * @param pvParameters function paramters
  * @retval None
  */
void vMidiTaskMain(void *pvParameters);

/**
  * @brief Callback to handle serial events
  * @param event serial event generated
  * @retval None
  */
void vSerialEventCb(sys_serial_event_t event);

/**
 * @brief Callback for Midi commands with 1 data argument.
 * @param cmd 
 * @param data 
 */
void vMidiCallBackCmd1(uint8_t cmd, uint8_t data);

/**
 * @brief Callback for Midi commands with 2 data arguments.
 * 
 * @param cmd 
 * @param data0 
 * @param data1 
 */
void vMidiCallBackCmd2(uint8_t cmd, uint8_t data0, uint8_t data1);

/* Private fuctions ----------------------------------------------------------*/

void vSerialEventCb( sys_serial_event_t event )
{
    if ((xMidiTaskHandle != NULL) && (xMidiQueueHandler != NULL))
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        MidiEvent_t xEvent = { .eType = MIDI_EVENT_SERIAL_EVENT };

        switch (event)
        {
            case SYS_SERIAL_EVENT_RX_IDLE:
            {
                xEvent.uPayload.xSerialEvent.eEvent = MIDI_SERIAL_EVENT_RX;
            }
            break;

            case SYS_SERIAL_EVENT_RX_BUF_FULL:
            {
                xEvent.uPayload.xSerialEvent.eEvent = MIDI_SERIAL_EVENT_RX;
            }
            break;

            case SYS_SERIAL_EVENT_ERROR:
            {
                xEvent.uPayload.xSerialEvent.eEvent = MIDI_SERIAL_EVENT_ERROR;
            }
            break;

            default:
            {
                xEvent.uPayload.xSerialEvent.eEvent = MIDI_SERIAL_EVENT_NOT_DEF;
            }
            break;
        }

        if (xEvent.uPayload.xSerialEvent.eEvent != MIDI_SERIAL_EVENT_NOT_DEF)
        {
            xQueueSendFromISR(xMidiQueueHandler, &xEvent, &xHigherPriorityTaskWoken);
        }
    }
}

void vMidiCallBackCmd1(uint8_t cmd, uint8_t data)
{
#ifdef MIDI_SERIAL_DEBUG
    vCliRawPrintf("\r\nCMD: %02X-%02X", cmd, data);
#endif
    MidiEvent_t xEvent = { 
        .eType = MIDI_EVENT_MIDI_CMD_DATA1, 
        .uPayload.xCmdData1.u8Status = cmd,
        .uPayload.xCmdData1.u8Data1 = data
    };
    (void)xQueueSend(xMidiQueueHandler, &xEvent, (TickType_t)0);
}

void vMidiCallBackCmd2(uint8_t cmd, uint8_t data0, uint8_t data1)
{
#ifdef MIDI_SERIAL_DEBUG
    vCliRawPrintf("\r\nCMD: %02X-%02X-%02X", cmd, data0, data1);
#endif
    MidiEvent_t xEvent = { 
        .eType = MIDI_EVENT_MIDI_CMD_DATA2, 
        .uPayload.xCmdData2.u8Status = cmd,
        .uPayload.xCmdData2.u8Data1 = data0,
        .uPayload.xCmdData2.u8Data2 = data1
    };
    (void)xQueueSend(xMidiQueueHandler, &xEvent, (TickType_t)0);
}

void vMidiTaskMain( void *pvParameters )
{
    /* Init Midi lib control structure */
    (void)MidiLibInit(&xMidiHandler, u8SysExBuff, sizeof(u8SysExBuff), NULL, vMidiCallBackCmd1, vMidiCallBackCmd2, NULL);

    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(MIDI_INIT_DELAY));

    /* Show init msg */
    vCliPrintf(MIDI_TASK_NAME, "Init");

    /* Infinite loop */
    for(;;)
    {
        MidiEvent_t xEvent;

        if (xQueueReceive(xMidiQueueHandler, &xEvent, portMAX_DELAY) == pdPASS)
        {
            switch (xEvent.eType)
            {
                case MIDI_EVENT_SERIAL_EVENT:
                    {
                        uint8_t u8DataRx = 0;
                        while (SYS_SERIAL_Read(MIDI_SERIAL, &u8DataRx, 1U) != 0U)
                        {
#ifdef MIDI_SERIAL_DEBUG
                            vCliRawPrintf("\r\nRx x%02X", u8DataRx);
#endif
                            (void)MidiLibUpdate(&xMidiHandler, u8DataRx);
                        }
                    }
                    break;

                case MIDI_EVENT_MIDI_CMD_DATA1:
                {
                    vCliPrintf(MIDI_TASK_NAME, "CMD: %02X-%02X-%02X", 
                        xEvent.uPayload.xCmdData1.u8Status, 
                        xEvent.uPayload.xCmdData1.u8Data1);
                }
                break;

                case MIDI_EVENT_MIDI_CMD_DATA2:
                {
                    vCliPrintf(MIDI_TASK_NAME, "CMD: %02X-%02X-%02X", 
                        xEvent.uPayload.xCmdData2.u8Status, 
                        xEvent.uPayload.xCmdData2.u8Data1,
                        xEvent.uPayload.xCmdData2.u8Data2);
                }
                break;

                default:
                    vCliPrintf(MIDI_TASK_NAME, "Not defined event; %02X", xEvent.eType);
                    break;
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bMidiTaskInit(void)
{
    bool bRetval = false;

    /* Init HW resources */
    (void)SYS_SERIAL_Init(MIDI_SERIAL, vSerialEventCb);

    /* Create task */
    xTaskCreate(vMidiTaskMain, MIDI_TASK_NAME, MIDI_TASK_STACK, NULL, MIDI_TASK_PRIO, &xMidiTaskHandle);

    /* Create task queue */
    xMidiQueueHandler = xQueueCreate(MIDI_EVENT_QUEUE_SIZE, MIDI_EVENT_QUEUE_ELEMENT_SIZE);

    /* Check resources */
    ERR_ASSERT(xMidiTaskHandle);
    ERR_ASSERT(xMidiQueueHandler);

    if ((xMidiTaskHandle != NULL) && (xMidiQueueHandler != NULL))
    {
        bRetval = true;
    }

    return bRetval;
}

bool xMidiQueueEvent(MidiEvent_t *xMidiEvent)
{
    bool bRetval = false;

    if (xMidiQueueHandler != NULL)
    {
        if (xQueueSend(xMidiQueueHandler, (void*) &xMidiEvent, (TickType_t) 0) == pdTRUE)
        {
            bRetval = true;
        }
    }

    return bRetval;
}

/* EOF */