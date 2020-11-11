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

/** Midi event types */
typedef enum MidiEventType
{
    MIDI_EVENT_SERIAL_EVENT = 0U,
    MIDI_EVENT_CFG_UPDATE,
    SYNTH_EVENT_NOT_DEF = 0xFFU
} MidiEventType_t;

/** Midi serial event */
typedef enum MidiSerialEvent
{
    MIDI_SERIAL_EVENT_RX,           /**< Rx data from serial port */
    MIDI_SERIAL_EVENT_ERROR,        /**< Error detection */
    MIDI_SERIAL_EVENT_NOT_DEF = 0xFFU
} MidiSerialEvent_t;

/** Payload for MIDI serial event */
typedef struct MidiEventPayloadSerialEvent
{
    MidiSerialEvent_t eEvent;       /**< Definition with event type */
} MidiEventPayloadSerialEvent_t;

/** Midi Cfg parameter definition */
typedef enum MidiCfgParameter
{
    MIDI_CFG_MODE = 0U,     /**< Midi mode */
    MIDI_CFG_CHANNEL,       /**< Midi channel */
    MIDI_CFG_NUM_PARAM,     /**< Number of defined parameters */
} MidiCfgParameter_t;

/** Payload for MIDI cfg update event */
typedef struct MidiEventPayloadCfgUpdateEvent
{
    MidiCfgParameter_t eParameterId;
    uint32_t u32Value;
} MidiEventPayloadCfgUpdateEvent_t;

/** Union definitions with all event payload */
typedef union MidiEventPayload
{
    MidiEventPayloadSerialEvent_t xSerialEvent;
    MidiEventPayloadCfgUpdateEvent_t xCfgUpdate;
} MidiEventPayload_t;

/** MIDI task event */
typedef struct MidiEvent
{
    MidiEventType_t eType;          /**< Event type id */
    MidiEventPayload_t uPayload;    /**< Payload of event id */
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
 * @brief Queue command for midi task.
 * @param xMidiEvent 
 * @return true 
 * @return false 
 */
bool xMidiQueueEvent(MidiEvent_t *xMidiEvent);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_TASK_H */

/* EOF */