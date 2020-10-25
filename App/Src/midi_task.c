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
void vSerialEventCb( sys_serial_event_t event );

/* Private fuctions ----------------------------------------------------------*/

void vSerialEventCb( sys_serial_event_t event )
{
    switch (event)
    {
        case SYS_SERIAL_EVENT_RX_IDLE:
        break;

        case SYS_SERIAL_EVENT_ERROR:
        break;

        default:
        break;
    }
}

void vMidiTaskMain( void *pvParameters )
{
    /* Init Midi lib control structure */
    (void)MidiLibInit(&xMidiHandler, u8SysExBuff, sizeof(u8SysExBuff), NULL, NULL, NULL, NULL);

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
                case MIDI_EVENT_SERIAL_DATA:
                    {
                        vCliPrintf(MIDI_TASK_NAME, "Rx serial data: %d bytes", xEvent.uPayload.xSerialData.u8DataLen);

                        uint32_t u32Index = 0;

                        while (u32Index < xEvent.uPayload.xSerialData.u8DataLen)
                        {
                            (void)MidiLibUpdate(&xMidiHandler, xEvent.uPayload.xSerialData.pu8Data[u32Index++]);
                        }
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
    if (xMidiTaskHandle != NULL)
    {
        bRetval = true;
    }

    return bRetval;
}

QueueHandle_t xMidiGetQueue(void)
{
    return xMidiQueueHandler;
}

bool bMidiTaskNotify(uint32_t u32Event)
{
    bool bRetval = false;

    /* Check if task has been init */
    if (xMidiTaskHandle != NULL)
    {
        xTaskNotify(xMidiTaskHandle, u32Event, eSetBits);
        bRetval = true;
    }

    return bRetval;
}

/* EOF */