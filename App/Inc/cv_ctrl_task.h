/**
 * @file cv_ctrl_task.h
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Task to generate CV outputs from midi task.
 * @version 0.1
 * @date 2020-10-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CVCTRL_TASK_H
#define __CVCTRL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "sys_rtos.h"

/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define CVCTRL_TASK_NAME      "CVCTRL"
#define CVCTRL_TASK_STACK     (256U)
#define CVCTRL_TASK_PRIO      (1U)

/* Exported types ------------------------------------------------------------*/

/** CvCtrl event types */
typedef enum CvCtrlEventType
{
    CVCTRL_EVENT_MIDI_CMD = 0U,     /**< Midi command from midi task */
    CVCTRL_EVENT_BUTTON_PRESS,      /**< UI button update */
    CVCTRL_EVENT_NOT_DEF = 0xFFU
} CvCtrlEventType_t;

/** Button id */
typedef enum CvCtrlButtonId
{
    CVCTRL_BUTT0N_ID_0 = 0U,
    CVCTRL_BUTT0N_ID_NUM,
} CvCtrlButtonId_t;

/** Button event types */
typedef enum CvCtrlButtonEvent
{
    CVCTRL_BUTTON_EVENT_SHORT_PRESS= 0U,
    CVCTRL_BUTTON_EVENT_LONG_PRESS,
    CVCTRL_BUTTON_EVENT_NOT_DEF,
} CvCtrlButtonEvent_t;

/** Payload for Midi Cmd */
typedef struct CvCtrlEventPayloadMidiCmdEvent
{
    uint8_t u8Status;
    uint8_t u8Data0;
    uint8_t u8Data1;
} CvCtrlEventPayloadMidiCmdEvent_t;

/** Payload for Button update */
typedef struct CvCtrlEventPayloadButtonUpdateEvent
{
    CvCtrlButtonId_t eId;
    CvCtrlButtonEvent_t eType;
} CvCtrlEventPayloadButtonUpdateEvent_t;

/** Union definitions with all event payload */
typedef union CvCtrlEventPayload
{
    CvCtrlEventPayloadMidiCmdEvent_t xMidiCmdEvent;
    CvCtrlEventPayloadButtonUpdateEvent_t xButtonEvent;
} CvCtrlEventPayload_t;

/** CVCTRL task event */
typedef struct CvCtrlEvent
{
    CvCtrlEventType_t eType;        /**< Event type id */
    CvCtrlEventPayload_t uPayload;  /**< Payload of event id */
} CvCtrlEvent_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for CVCTRL task.
  * @retval operation result, true for correct creation, false for error.
  */
bool bCvCtrlTaskInit(void);

/**
 * @brief Queue command for cvctrl task.
 * @param xCvCtrlEvent 
 * @return true 
 * @return false 
 */
bool xCvCtrlQueueEvent(CvCtrlEvent_t *xCvCtrlEvent);

#ifdef __cplusplus
}
#endif

#endif /* __CVCTRL_TASK_H */

/* EOF */