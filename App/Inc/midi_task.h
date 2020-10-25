/**
 * @file midi_task.h
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Task to handle midi communication and mapping midi commands to system features
 * @version 0.1
 * @date 2020-10-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIDI_TASK_H
#define __MIDI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include "sys_rtos.h"

/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define MIDI_TASK_NAME      "MIDI"
#define MIDI_TASK_STACK     (256U)
#define MIDI_TASK_PRIO      (1U)

/* Exported types ------------------------------------------------------------*/

/** Midi task defined events */
typedef enum
{
    MIDI_EVENT_SERIAL_DATA = 0U,
    MIDI_EVENT_NOT_DEF = 0xFF
} MidiEventType_t;

/** Payload for MIDI msg event */
typedef struct
{
    uint8_t u8DataLen;
    uint8_t *pu8Data;
} MidiEventPayloadSerialData_t;

/** Union definitions with all event payload */
typedef union
{
    MidiEventPayloadSerialData_t xSerialData;
} MidiPayload_t;

/** Midi Event definition */
typedef struct
{
    MidiEventType_t eType;      /** Event type id */
    MidiPayload_t uPayload;     /** Payload of event id */
} MidiEvent_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for MIDI task.
  * @retval operation result, true for correct creation, false for error.
  */
bool bMidiTaskInit(void);

/**
 * @brief Get command queue for task.
 * @return QueueHandle_t 
 */
QueueHandle_t xMidiGetQueue(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error.
  */
bool bMidiTaskNotify(uint32_t u32Event);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_TASK_H */

/* EOF */