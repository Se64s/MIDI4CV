/**
 * @file cv_ctrl_task.c
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Task to generate CV outputs from midi task.
 * @version 0.1
 * @date 2020-10-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "cv_ctrl_task.h"
#include "cli_task.h"
#include "midi_lib.h"
#include "sys_gpio.h"
#include "sys_i2c.h"
#include "dac_mcp4728.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Initial delay to wait for other task */
#define CVCTRL_INIT_DELAY                   (100U)

/* Seize of midi task command */
#define CVCTRL_EVENT_QUEUE_ELEMENT_SIZE     (sizeof(CvCtrlEvent_t))

/* Number of commands that can queue the task */
#define CVCTRL_QUEUE_SIZE                   (5U)

/* GPIO assignations */
#define CVCTRL_GPIO_BUTTON                  SYS_GPIO_00
#define CVCTRL_GPIO_LED                     SYS_GPIO_01

/* I2C Port setup */
#define I2C_DAC                             SYS_I2C_0

/* Define Button times */
#define BUTTON_TIME_LONG                    (1000U)
#define BUTTON_TIME_VERY_LONG               (5000U)

/* Private macro -------------------------------------------------------------*/

#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private variables ---------------------------------------------------------*/

/** Task handler */
TaskHandle_t xCvCtrlTaskHandle = NULL;

/** Queue handler */
QueueHandle_t xCvCtrlQueueHandler = NULL;

/** Button timer handle */
TimerHandle_t  xButtonTimerHandler = NULL;

/** Dac handler */
DacMcp4728_t xDacHandler = { 
    .u8Address = DAC_BASE_ADDR,
    .u8IfPort = I2C_DAC,
    .eVref = DAC_VREF_INT,
    .eGain = DAC_GAIN_1,
    .ePd = DAC_PD_NORMAL
 };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Init resources for CLI tasks
  * @param pvParameters function paramters
  * @retval None
  */
void vCvCtrlTaskMain(void *pvParameters);

/**
 * @brief CAllback to handle button irq
 * 
 * @param event 
 */
void vButtonCallBack(sys_gpio_event_t event);

/**
 * @brief Handle of timer callback.
 * 
 * @param xTimer 
 */
void vButtonTimerCallBack(TimerHandle_t xTimer);

/* Private fuctions ----------------------------------------------------------*/

void vButtonCallBack(sys_gpio_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((event == SYS_GPIO_EXTI_RISE_EVENT) || (event == SYS_GPIO_EXTI_FALL_EVENT))
    {
        if (xButtonTimerHandler != NULL)
        {
            if (event == SYS_GPIO_EXTI_FALL_EVENT)
            {
                SYS_GPIO_Set(CVCTRL_GPIO_LED, SYS_GPIO_STATE_SET);

                (void)xTimerStartFromISR(xButtonTimerHandler, &xHigherPriorityTaskWoken);
            }
            else
            {
                SYS_GPIO_Set(CVCTRL_GPIO_LED, SYS_GPIO_STATE_RESET);

                if( xTimerIsTimerActive( xButtonTimerHandler ) != pdFALSE )
                {
                    CvCtrlEvent_t xEvent = { 
                        .eType = CVCTRL_EVENT_BUTTON_PRESS, 
                        .uPayload.xButtonEvent.eId = CVCTRL_BUTT0N_ID_0,
                        .uPayload.xButtonEvent.eType = CVCTRL_BUTTON_EVENT_LONG_PRESS
                    };

                    TickType_t xRemainingTime;
                    xRemainingTime = xTimerGetExpiryTime(xButtonTimerHandler) - xTaskGetTickCount();

                    if ( xRemainingTime > (BUTTON_TIME_VERY_LONG - BUTTON_TIME_LONG) )
                    {
                        xEvent.uPayload.xButtonEvent.eType = CVCTRL_BUTTON_EVENT_SHORT_PRESS;
                    }

                    xQueueSendFromISR(xCvCtrlQueueHandler, &xEvent, &xHigherPriorityTaskWoken);
                }

                xTimerStop(xButtonTimerHandler, 0U);
            }
        }
    }
}

void vButtonTimerCallBack(TimerHandle_t xTimer)
{
    (void)xTimer;

    CvCtrlEvent_t xEvent = { 
        .eType = CVCTRL_EVENT_BUTTON_PRESS, 
        .uPayload.xButtonEvent.eId = CVCTRL_BUTT0N_ID_0,
        .uPayload.xButtonEvent.eType = CVCTRL_BUTTON_EVENT_VERY_LONG_PRESS
    };

    xQueueSend(xCvCtrlQueueHandler, &xEvent, (TickType_t)0);
}

void vCvCtrlTaskMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(CVCTRL_INIT_DELAY));

    /* Show init msg */
    vCliPrintf(CVCTRL_TASK_NAME, "Init");

    /* Infinite loop */
    for(;;)
    {
        CvCtrlEvent_t xEvent;

        if (xQueueReceive(xCvCtrlQueueHandler, &xEvent, portMAX_DELAY) == pdPASS)
        {
            switch (xEvent.eType)
            {
                case CVCTRL_EVENT_MIDI_CMD:
                    {
                        vCliPrintf(CVCTRL_TASK_NAME, "Midi Cmd Event: %02X-%02X-%02X", 
                            xEvent.uPayload.xMidiCmdEvent.u8Status,
                            xEvent.uPayload.xMidiCmdEvent.u8Data0,
                            xEvent.uPayload.xMidiCmdEvent.u8Data1);
                    }
                    break;

                case CVCTRL_EVENT_BUTTON_PRESS:
                    {
                        vCliPrintf(CVCTRL_TASK_NAME, "Button Event: %02X", xEvent.uPayload.xButtonEvent.eType);
                    }
                    break;

                default:
                    {
                        vCliPrintf(CVCTRL_TASK_NAME, "Unknown event: %02X", xEvent.eType);
                    }
                    break;
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bCvCtrlTaskInit(void)
{
    bool bRetval = false;

    /* Init gpio handlers */
    (void)SYS_GPIO_Init(CVCTRL_GPIO_BUTTON, SYS_GPIO_MODE_EXTI, vButtonCallBack);
    (void)SYS_GPIO_Init(CVCTRL_GPIO_LED, SYS_GPIO_MODE_OUT, NULL);

    /* Dac init */
    (void)DacInit(&xDacHandler);

    /* Create task */
    xTaskCreate(vCvCtrlTaskMain, CVCTRL_TASK_NAME, CVCTRL_TASK_STACK, NULL, CVCTRL_TASK_PRIO, &xCvCtrlTaskHandle);

    /* Create task queue */
    xCvCtrlQueueHandler = xQueueCreate(CVCTRL_QUEUE_SIZE, CVCTRL_EVENT_QUEUE_ELEMENT_SIZE);

    /* Timer to control button actions */
    xButtonTimerHandler = xTimerCreate("ButtonTimer", pdMS_TO_TICKS(BUTTON_TIME_VERY_LONG), pdFALSE, (void *) 0, vButtonTimerCallBack);

    /* Check resources */
    ERR_ASSERT(xCvCtrlTaskHandle);
    ERR_ASSERT(xCvCtrlQueueHandler);
    ERR_ASSERT(xButtonTimerHandler);

    if ((xCvCtrlTaskHandle != NULL) && (xCvCtrlQueueHandler != NULL) && (xButtonTimerHandler != NULL))
    {
        bRetval = true;
    }

    return bRetval;
}

bool xCvCtrlQueueEvent(CvCtrlEvent_t *xCvCtrlEvent)
{
    bool bRetval = false;

    if (xCvCtrlQueueHandler != NULL)
    {
        if (xQueueSend(xCvCtrlQueueHandler, (void*)xCvCtrlEvent, (TickType_t) 0) == pdTRUE)
        {
            bRetval = true;
        }
    }

    return bRetval;
}

/* EOF */