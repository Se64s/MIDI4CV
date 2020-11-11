/**
 * @file midi_lib.h
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Library to parse Midi messages.
 * @version 0.1
 * @date 2020-10-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIDI_LIB_H
#define __MIDI_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Exported types ------------------------------------------------------------*/

/** Midi modes */
typedef enum
{
    MidiMode1 = 0x00,   /**< Omni-on Poly */
    MidiMode2,          /**< Omni-on Mono */
    MidiMode3,          /**< Omni-off Poly */
    MidiMode4,          /**< Omni-off Mono */
    MidiModeNum,        /**< Number of defined modes */
} MidiMode_t;

/** Defined midi lib status */
typedef enum
{
    midiOk = 0x00,
    midiSysExcBuffFull,
    midiHandleNewState,
    midiError,
} MidiStatus_t;

/** List of defined states used in rx_fsm */
typedef enum
{
    wait_byte_init = 0,
    wait_byte_first_data,
    wait_byte_second_data,
    wait_byte_sys_ex,
    wait_byte_data,
    dispatch_status,
    handle_1_data_msg,
    handle_2_data_msg,
    handle_sys_ex_msg,
    handle_rt_msg,
} Midi_rx_state_t;

/* CallBack definition -------------------------------------------------------*/

/** CB for handle sys_ex */
typedef void (*midi_cb_msg_sys_ex_t)(uint8_t *pdata, uint32_t len_data);

/** CB for handle 1 data message */
typedef void (*midi_cb_msg_data1_t)(uint8_t cmd, uint8_t data);

/** CB for handle 2 data message */
typedef void (*midi_cb_msg_data2_t)(uint8_t cmd, uint8_t data0, uint8_t data1);

/** CB for handle rt message */
typedef void (*midi_cb_msg_rt_t)(uint8_t rt_data);

/* Control structure definition ----------------------------------------------*/

/** General handler for manage a MIDI interface */
typedef struct MidiHandler
{
    midi_cb_msg_sys_ex_t xSysExCmdCB;       /**< Cb pointer for handle sys_ex functions */
    midi_cb_msg_data1_t xMidiData1CmdCB;    /**< Cb pointer for handle msg data 1 functions */
    midi_cb_msg_data2_t xMidiData2CmdCB;    /**< Cb pointer for handle msg data 2 functions */
    midi_cb_msg_rt_t xMidiRtCmdCB;          /**< Cb pointer for handle msg rt functions */
    Midi_rx_state_t eFsmState;              /**< Current system state control var */
    uint8_t u8TmpRunningStatus;             /**< Tmp var for running status */
    uint8_t u8TmpData1;                     /**< Tmp command data 1 */
    uint8_t u8TmpData2;                     /**< Tmp command data 2 */
    uint32_t u32IndexSysEx;                 /**< Index for sysex buffer */
    uint32_t u32SysExDataSize;              /**< Maximum size for sysex data */
    uint8_t *pu8SysExData;                  /**< address for sysex data */
} MidiLibHandler_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Get command from status byte */
#define MIDI_STATUS_GET_CMD(status)    ( (status) & MIDI_STATUS_CMD_MASK )

/* Get channel from status byte */
#define MIDI_STATUS_GET_CH(status)     ( (status) & MIDI_STATUS_CH_MASK )

/* Exported defines ----------------------------------------------------------*/

/* Midi data value not valid */
#define MIDI_DATA_NOT_VALID             0xFF

/* MIDI maximun program value */
#define MIDI_PROGRAM_MAX_VALUE          0x0F

/* MIDI maximun BANK value */
#define MIDI_BANK_MAX_VALUE             0x0F

/* Mask to get channel from status messages */
#define MIDI_STATUS_CH_MASK             0x0F
#define MIDI_STATUS_CMD_MASK            0xF0

/** MIDI Channel list */
typedef enum MidiChannel
{
    MIDI_CH_01 = 0x00,
    MIDI_CH_02 = 0x01,
    MIDI_CH_03 = 0x02,
    MIDI_CH_04 = 0x03,
    MIDI_CH_05 = 0x04,
    MIDI_CH_06 = 0x05,
    MIDI_CH_07 = 0x06,
    MIDI_CH_08 = 0x07,
    MIDI_CH_09 = 0x08,
    MIDI_CH_10 = 0x09,
    MIDI_CH_11 = 0x0A,
    MIDI_CH_12 = 0x0B,
    MIDI_CH_13 = 0x0C,
    MIDI_CH_14 = 0x0D,
    MIDI_CH_15 = 0x0E,
    MIDI_CH_16 = 0x0F,
    MIDI_CH_MAX_NUM = 0x10
} MidiChannel_t;

/** MIDI status cmd with 1 bytes */
typedef enum MidiStatus1Byte
{
    MIDI_STATUS_PROG_CHANGE = 0xC0,
    MIDI_STATUS_CH_PRESS    = 0xD0,
    MIDI_STATUS_TIME_CODE   = 0xF0,
    MIDI_STATUS_SONG_SELECT = 0xF3,
} MidiStatus1Byte_t;

/** MIDI status cmd with 2 bytes */
typedef enum MidiStatus2Bytes
{
    MIDI_STATUS_NOTE_OFF    = 0x80,
    MIDI_STATUS_NOTE_ON     = 0x90,
    MIDI_STATUS_POLY_PRESS  = 0xA0,
    MIDI_STATUS_CC          = 0xB0,
    MIDI_STATUS_PITCH_BEND  = 0xE0,
    MIDI_STATUS_SONG_POS    = 0xF2,
} MidiStatus2Bytes_t;

/** MIDI status cmd with 2 bytes */
typedef enum MidiCCType
{
    MIDI_CC_MOD         = 0x01,
    MIDI_CC_BRI         = 0x4A,
    MIDI_CC_HAR         = 0x47,
    MIDI_CC_ATT         = 0x49,
    MIDI_CC_C15         = 0x15,
    MIDI_CC_C16         = 0x16,
    MIDI_CC_C17         = 0x17,
    MIDI_CC_C18         = 0x18,
    MIDI_CC_C4B         = 0x4B,
    MIDI_CC_NOTE_OFF    = 0x7B,
} MidiCCType_t;

/** MIDI RT status commands */
typedef enum MidiRTStatus
{
    MIDI_RT_CLK     = 0xF8,
    MIDI_RT_START   = 0xFA,
    MIDI_RT_CONT    = 0xFB,
    MIDI_RT_STOP    = 0xFC,
    MIDI_RT_RESET   = 0xFF,
} MidiRTStatus_t;

/** MIDI status SYS_EX */
typedef enum MidiSysExStatus_t
{
    MIDI_STATUS_SYS_EX_START    = 0xF0,
    MIDI_STATUS_SYS_EX_END      = 0xF7,
} MidiSysExStatus_t;

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Init library FSM and setup callback functions for rx parsing.
 * @param  pxMidiHandler Midi control structure.
 * @param  pu8SysExBuff Buffer for sysEx messages.
 * @param  u32SysExBuffLen Buffer len for sysEx messages.
 * @param  cb_sys_ex: callback function for sysEx messages.
 * @param  cb_msg_data1: callback function for 1 data messages.
 * @param  cb_msg_data2: callback function for 2 data messages.
 * @param  cb_msg_rt: callback function for real time messages
 * @retval Operation status.
 */
MidiStatus_t MidiLibInit(MidiLibHandler_t *pxMidiHandler,
                    uint8_t *pu8SysExBuff,
                    uint32_t u32SysExBuffLen,
                    midi_cb_msg_sys_ex_t cb_sys_ex,
                    midi_cb_msg_data1_t cb_msg_data1,
                    midi_cb_msg_data2_t cb_msg_data2,
                    midi_cb_msg_rt_t cb_msg_rt);

/**
 * @brief  Update midi rx_fsm.
 * @param  pxMidiHandler Midi control structure.
 * @param  data_rx: input byte to update midi_rx fsm.
 * @retval Operation result.
 */
MidiStatus_t MidiLibUpdate(MidiLibHandler_t *pxMidiHandler, uint8_t data_rx);

/**
 * @brief  Set rx_fsm into reset state.
 * @param  pxMidiHandler Midi control structure.
 * @retval Operation result.
 */
MidiStatus_t MidiLibResetHandler(MidiLibHandler_t *pxMidiHandler);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_LIB_H */

/* EOF */
