/**
 * @file midi_lib.c
 * @author Sebastian Del Moral (sebmorgal@gmail.com)
 * @brief Library to parse Midi messages.
 * @version 0.1
 * @date 2020-10-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "midi_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define MIDI_STATUS_MASK            ( 1 << 7 )
#define MIDI_STATUS_RT_MASK         (0xF8)

#define MIDI_IS_STATUS(data)        ( ( (data) & MIDI_STATUS_MASK ) == MIDI_STATUS_MASK )
// #define MIDI_IS_RT(data)            (((data) & MIDI_STATUS_RT_MASK) == MIDI_STATUS_RT_MASK)
#define MIDI_IS_RT(data)            ( ( ( (data) & MIDI_STATUS_RT_MASK) == MIDI_STATUS_RT_MASK ) && ( (data) != MIDI_STATUS_SYS_EX_START) && ( (data) != MIDI_STATUS_SYS_EX_END) )
#define MIDI_STATUS_GET_CH(status)  ( ( (status) & MIDI_STATUS_CH_MASK) >> 0 )
#define MIDI_STATUS_GET_CMD(status) ( ( (status) & MIDI_STATUS_CMD_MASK ) >> 4 )

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static midiStatus_t state_handler_wait_byte_init(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_first_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_second_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_sys_ex(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);
static midiStatus_t state_handler_dispatch_status(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte);

/* Private function ----------------------------------------------------------*/

static midiStatus_t state_handler_wait_byte_init(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (pxMidiHandler->xMidiRtCmdCB != NULL)
            {
                pxMidiHandler->xMidiRtCmdCB(rx_byte);
            }
        }
        else
        {
            pxMidiHandler->eFsmState = dispatch_status;
            retval = midiHandleNewState;
        }
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_first_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (pxMidiHandler->xMidiRtCmdCB != NULL)
            {
                pxMidiHandler->xMidiRtCmdCB(rx_byte);
            }
        }
        else
        {
            pxMidiHandler->eFsmState = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        pxMidiHandler->u8TmpData1 = rx_byte;
        pxMidiHandler->eFsmState = wait_byte_second_data;
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_second_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (pxMidiHandler->xMidiRtCmdCB != NULL)
            {
                pxMidiHandler->xMidiRtCmdCB(rx_byte);
            }
        }
        else
        {
            pxMidiHandler->eFsmState = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        pxMidiHandler->u8TmpData2 = rx_byte;
        if (pxMidiHandler->xMidiData2CmdCB != NULL)
        {
            pxMidiHandler->xMidiData2CmdCB(pxMidiHandler->u8TmpRunningStatus, pxMidiHandler->u8TmpData1, pxMidiHandler->u8TmpData2);
        }
        pxMidiHandler->eFsmState = wait_byte_first_data;
        pxMidiHandler->u8TmpData1 = 0;
        pxMidiHandler->u8TmpData2 = 0;
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_sys_ex(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (pxMidiHandler->xMidiRtCmdCB != NULL)
            {
                pxMidiHandler->xMidiRtCmdCB(rx_byte);
            }
        }
        else
        {
            if (rx_byte == MIDI_STATUS_SYS_EX_END)
            {
                /* Handle end of sys_ex */
                if (pxMidiHandler->xSysExCmdCB != NULL)
                {
                    pxMidiHandler->xSysExCmdCB(pxMidiHandler->pu8SysExData, pxMidiHandler->u32IndexSysEx);
                    //i_data_sys_ex = 0;
                }
            }
            pxMidiHandler->eFsmState = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        /* Handle data byte for sys_ex */
        if (pxMidiHandler->u32IndexSysEx < pxMidiHandler->u32SysExDataSize)
        {
            pxMidiHandler->pu8SysExData[pxMidiHandler->u32IndexSysEx] = rx_byte;
            pxMidiHandler->u32IndexSysEx++;
        }
        else
        {
            pxMidiHandler->u32IndexSysEx = 0;
            pxMidiHandler->eFsmState = wait_byte_init;
            retval = midiSysExcBuffFull;
        }
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_data(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (pxMidiHandler->xMidiRtCmdCB != NULL)
            {
                pxMidiHandler->xMidiRtCmdCB(rx_byte);
            }
        }
        else
        {
            pxMidiHandler->eFsmState = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        if (pxMidiHandler->xMidiData1CmdCB != NULL)
        {
            pxMidiHandler->xMidiData1CmdCB(pxMidiHandler->u8TmpRunningStatus, rx_byte);
        }
    }

    return retval;
}

static midiStatus_t state_handler_dispatch_status(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte)
{
    midiStatus_t retval = midiError;

    /* Check if 1 data cmd */
    if (((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_PROG_CHANGE) ||
        ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_CH_PRESS) ||
        (rx_byte == MIDI_STATUS_TIME_CODE) ||
        (rx_byte == MIDI_STATUS_SONG_SELECT))
    {
        pxMidiHandler->eFsmState = wait_byte_data;
        retval = midiOk;
    }
    /* Check if 2 data cmd */
    else if (((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_POLY_PRESS) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_CC) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_PITCH_BEND) ||
             (rx_byte == MIDI_STATUS_SONG_POS))
    {
        pxMidiHandler->eFsmState = wait_byte_first_data;
        retval = midiOk;
    }
    /* Check if sys ex cmd */
    else if (rx_byte == MIDI_STATUS_SYS_EX_START)
    {
        pxMidiHandler->u32IndexSysEx = 0;
        pxMidiHandler->eFsmState = wait_byte_sys_ex;
        retval = midiOk;
    }
    /* If not defined, jump to start */
    else
    {
        pxMidiHandler->eFsmState = wait_byte_init;
    }

    /* Save running status */
    if (retval == midiOk)
    {
        pxMidiHandler->u8TmpRunningStatus = rx_byte;
    }

    return retval;
}

/* ---------------------------------------------------------------------------*/

/** State Handler definitions */
midiStatus_t (*rx_state_table[])(MidiLibHandler_t *pxMidiHandler, uint8_t rx_byte) = {
    state_handler_wait_byte_init,
    state_handler_wait_byte_first_data,
    state_handler_wait_byte_second_data,
    state_handler_wait_byte_sys_ex,
    state_handler_wait_byte_data,
    state_handler_dispatch_status};

/* Public function -----------------------------------------------------------*/

/* Init library FSM and setup callback functions for rx parsing */
midiStatus_t MidiLibInit(MidiLibHandler_t *pxMidiHandler,
                    uint8_t *pu8SysExBuff,
                    uint32_t u32SysExBuffLen,
                    midi_cb_msg_sys_ex_t cb_sys_ex,
                    midi_cb_msg_data1_t cb_msg_data1,
                    midi_cb_msg_data2_t cb_msg_data2,
                    midi_cb_msg_rt_t cb_msg_rt)
{
    midiStatus_t retval = midiError;

    if ( (pu8SysExBuff != NULL) && (u32SysExBuffLen > 0)  && (pxMidiHandler != NULL) )
    {
        /* Init private cb handlers */
        if (cb_sys_ex != NULL)
        {
            pxMidiHandler->xSysExCmdCB = cb_sys_ex;
        }
        if (cb_msg_data1 != NULL)
        {
            pxMidiHandler->xMidiData1CmdCB = cb_msg_data1;
        }
        if (cb_msg_data2 != NULL)
        {
            pxMidiHandler->xMidiData2CmdCB = cb_msg_data2;
        }
        if (cb_msg_rt != NULL)
        {
            pxMidiHandler->xMidiRtCmdCB = cb_msg_rt;
        }

        pxMidiHandler->pu8SysExData = pu8SysExBuff;
        pxMidiHandler->u32SysExDataSize = u32SysExBuffLen;

        pxMidiHandler->eFsmState = wait_byte_init;
        pxMidiHandler->u8TmpRunningStatus = 0;

        retval = midiOk;
    }

    return retval;
}

/* Update midi rx_fsm */
midiStatus_t MidiLibUpdate(MidiLibHandler_t *pxMidiHandler, uint8_t data_rx)
{
    midiStatus_t retval = midiError;

    if (pxMidiHandler != NULL)
    {
        do {
            retval = rx_state_table[pxMidiHandler->eFsmState](pxMidiHandler, data_rx);
        } while (retval == midiHandleNewState);
    }

    return retval;
}

/* Set rx_fsm into reset state */
midiStatus_t MidiLibResetHandler(MidiLibHandler_t *pxMidiHandler)
{
    midiStatus_t retval = midiError;

    if (pxMidiHandler != NULL)
    {
        pxMidiHandler->eFsmState = wait_byte_init;
        pxMidiHandler->u8TmpRunningStatus = 0;
        pxMidiHandler->u8TmpData1 = 0;
        pxMidiHandler->u8TmpData2 = 0;

        retval = midiOk;
    }

    return retval;
}

/* EOF */
