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
#include "cv_ctrl_task.h"
#include "sys_serial.h"
#include "midi_lib.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/** Midi control structure */
typedef struct MidiCfg
{
    MidiMode_t eMode;
    uint32_t u32ChannelMask;
} MidiCfg_t;

/* Private define ------------------------------------------------------------*/

/* Midi serial debug */
#define MIDI_SERIAL_DEBUG

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

#define CHECK_CHANNEL_MASK(ch, mask)        ( ( ( 1U << (ch) ) & (mask) ) != 0x00 )

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

/** Midi config handler */
MidiCfg_t xMidiConfig = {
    .eMode = MidiMode1,
    .u32ChannelMask = 0x01  /**< Mask for channel 01 */
};

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
    uint8_t u8Channel = MIDI_STATUS_GET_CH(cmd);

    if (CHECK_CHANNEL_MASK(u8Channel, xMidiConfig.u32ChannelMask))
    {
        vCliPrintf(MIDI_TASK_NAME, "CMD: %02X-%02X", cmd, data);
    }
}

void vMidiCallBackCmd2(uint8_t cmd, uint8_t data0, uint8_t data1)
{
#ifdef MIDI_SERIAL_DEBUG
    vCliRawPrintf("\r\nCMD: %02X-%02X-%02X", cmd, data0, data1);
#endif
    uint8_t u8Status = MIDI_STATUS_GET_CMD(cmd);
    uint8_t u8Channel = MIDI_STATUS_GET_CH(cmd);

    /* Check message channel */
    if (CHECK_CHANNEL_MASK(u8Channel, xMidiConfig.u32ChannelMask))
    {
        vCliPrintf(MIDI_TASK_NAME, "CMD: %02X-%02X-%02X", cmd, data0, data1);

        bool bProcess = false;

        switch (u8Status)
        {
            case MIDI_STATUS_NOTE_OFF:
            case MIDI_STATUS_NOTE_ON:
            case MIDI_STATUS_PITCH_BEND:
            {
                bProcess = true;
            }
            break;

            case MIDI_STATUS_CC:
            {
                if (data0 == MIDI_CC_MOD)
                {
                    bProcess = true;
                }
            }
            break;

            default:
            break;
        }

        if (bProcess)
        {
            vCliPrintf(MIDI_TASK_NAME, "Sending message to processing task");

            CvCtrlEvent_t xCvCtrlEvent = {
                .eType = CVCTRL_EVENT_MIDI_CMD,
                .uPayload.xMidiCmdEvent.u8Status = cmd,
                .uPayload.xMidiCmdEvent.u8Data0 = data0,
                .uPayload.xMidiCmdEvent.u8Data1 = data1
            };

            (void)xCvCtrlQueueEvent(&xCvCtrlEvent);
        }
    }
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

                case MIDI_EVENT_CFG_UPDATE:
                    {
                        switch (xEvent.uPayload.xCfgUpdate.eParameterId)
                        {
                            case MIDI_CFG_MODE:
                                {
                                    if (xEvent.uPayload.xCfgUpdate.u32Value < (uint32_t)MidiModeNum)
                                    {
                                        xMidiConfig.eMode = xEvent.uPayload.xCfgUpdate.u32Value;
                                        vCliPrintf(MIDI_TASK_NAME, "Update Midi Mode: %d", xMidiConfig.eMode);
                                    }
                                }
                                break;

                            case MIDI_CFG_CHANNEL_MASK:
                                {
                                    if (xEvent.uPayload.xCfgUpdate.u32Value < (uint32_t)MIDI_CH_MAX_NUM)
                                    {
                                        xMidiConfig.u32ChannelMask = xEvent.uPayload.xCfgUpdate.u32Value;
                                        vCliPrintf(MIDI_TASK_NAME, "Update Channel Mask: %d", xMidiConfig.u32ChannelMask);
                                    }
                                }
                                break;

                            default:
                                vCliPrintf(MIDI_TASK_NAME, "Not defined parameter; %02X", xEvent.uPayload.xCfgUpdate.eParameterId);
                                break;
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
        if (xQueueSend(xMidiQueueHandler, (void*)xMidiEvent, (TickType_t) 0) == pdTRUE)
        {
            bRetval = true;
        }
    }

    return bRetval;
}

/* EOF */