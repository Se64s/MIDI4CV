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
    MIDI_EVENT_MIDI_CMD_DATA1,
    MIDI_EVENT_MIDI_CMD_DATA2,
    MIDI_EVENT_MIDI_CMD_SYSEX,
    MIDI_EVENT_MIDI_CMD_RT,
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

/** Payload for MIDI command with one argument */
typedef struct MidiEventPayloadMidiCmdData1
{
    uint8_t u8Status;
    uint8_t u8Data1;
} MidiEventPayloadMidiCmdData1_t;

/** Payload for MIDI command with two arguments */
typedef struct MidiEventPayloadMidiCmdData2
{
    uint8_t u8Status;
    uint8_t u8Data1;
    uint8_t u8Data2;
} MidiEventPayloadMidiCmdData2_t;

/** Union definitions with all event payload */
typedef union MidiEventPayload
{
    MidiEventPayloadSerialEvent_t xSerialEvent;
    MidiEventPayloadMidiCmdData1_t xCmdData1;
    MidiEventPayloadMidiCmdData2_t xCmdData2;
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