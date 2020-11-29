/**
 * @file dac_mcp4728.c
 * @author Sebastián Del Moral Gallardo (sebmorgal@gmail.com)
 * @brief Library to handle DAC MCP4728 (https://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf)
 * @version 0.1
 * @date 2020-11-28
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Private includes --------------------------------------------------------*/
#include "dac_mcp4728.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Low level library for i2v handling */
#include "sys_i2c.h"

/* Private macro -----------------------------------------------------------*/
#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private defines ---------------------------------------------------------*/

/* I2c Retry count */
#define I2C_RETRY                   (10U)

/* Data len for commands */
#define CMD_BUFFER_LEN              (16U)

/* Command bit shift */
#define CMD_BIT_SHIFT               (5U)
#define FAST_WRITE CMD_BIT_SHIFT    (6U)

#define CMD_VREF_GAIN_A_BIT_SHIFT   (0U)
#define CMD_VREF_GAIN_B_BIT_SHIFT   (1U)
#define CMD_VREF_GAIN_C_BIT_SHIFT   (2U)
#define CMD_VREF_GAIN_D_BIT_SHIFT   (3U)

#define CMD_PD_A_BIT_SHIFT          (2U)
#define CMD_PD_B_BIT_SHIFT          (0U)
#define CMD_PD_C_BIT_SHIFT          (6U)
#define CMD_PD_D_BIT_SHIFT          (4U)

/* Commands for DAC (5b)*/
typedef enum dac_cmd
{
    DAC_CMD_FAST_WRITE = 0x00,      /**< Fast write input reg */
    DAC_CMD_WRITE_REG_MULT = 0x08,  /**< Multiple write input reg */
    DAC_CMD_WRITE_REG_SEQ = 0x0A,   /**< Sequential write input reg */
    DAC_CMD_WRITE_REG_SING = 0x0B,  /**< Single write input reg */
    DAC_CMD_WRITE_ADDR = 0x0C,      /**< Write i2c address */
    DAC_CMD_WRITE_VREF = 0x10,      /**< Write Vreg bit */
    DAC_CMD_WRITE_PD = 0x14,        /**< Write PD bits */
    DAC_CMD_WRITE_GAIN = 0x18,      /**< Write Gain bit */
} dac_cmd_t;

/* Private variable --------------------------------------------------------*/
/* Private macro -----------------------------------------------------------*/
/* Private functions prototypes --------------------------------------------*/
/* Private functions definition --------------------------------------------*/

/**
 * @brief Low level funtions to initiate i2c interface
 * 
 * @param u8InterfaceId user interface id
 * @return DacStatus_t operation status
 */
static DacStatus_t lowlevel_init(uint8_t u8InterfaceId);

/**
 * @brief Low level function to send data over i2c interface.
 * 
 * @param u8InterfaceId user interface id
 * @param u8Adress address use in transaction
 * @param pu8Data pointer with data to send
 * @param u16len len of data to send
 * @return DacStatus_t operation result
 */
static DacStatus_t lowlevel_send(uint8_t u8InterfaceId, uint8_t u8Adress, uint8_t *pu8Data, uint16_t u16len);

/**
 * @brief Get functional status of i2c bus
 * 
 * @param u8InterfaceId user interface id
 */
static DacStatus_t lowlevel_IfStatus(uint8_t u8InterfaceId);

/* Low level interface -----------------------------------------------------*/

static DacStatus_t lowlevel_init(uint8_t u8InterfaceId)
{
    DacStatus_t eRetval = DAC_ERROR;

    /* User defined init code */
    if (u8InterfaceId == (uint8_t)SYS_I2C_0)
    {
        sys_i2c_status_t eOpResult = SYS_I2C_Init(SYS_I2C_0, NULL);

        if ( eOpResult == SYS_I2C_STATUS_OK )
        {
            eRetval = DAC_OK;
        }
        else if (eOpResult == SYS_I2C_STATUS_BUSY)
        {
            eRetval = DAC_BUSY;
        }
    }

    return eRetval;
}

static DacStatus_t lowlevel_send(uint8_t u8InterfaceId, uint8_t u8Adress, uint8_t *pu8Data, uint16_t u16len)
{
    USER_ASSERT(pu8Data != NULL);
    DacStatus_t eRetval = DAC_ERROR;

    if ( u8InterfaceId == (uint8_t)SYS_I2C_0 )
    {
        sys_i2c_status_t eOpResult = SYS_I2C_MasterWrite(SYS_I2C_0, u8Adress, pu8Data, u16len);

        if ( eOpResult == SYS_I2C_STATUS_OK )
        {
            eRetval = DAC_OK;
        }
        else if (eOpResult == SYS_I2C_STATUS_BUSY)
        {
            eRetval = DAC_BUSY;
        }
    }

    return eRetval;
}


static DacStatus_t lowlevel_IfStatus(uint8_t u8InterfaceId)
{
    DacStatus_t eRetval = DAC_ERROR;

    if ( u8InterfaceId == (uint8_t)SYS_I2C_0 )
    {
        sys_i2c_status_t eOpResult = SYS_I2C_GetState(SYS_I2C_0);

        if ( eOpResult == SYS_I2C_STATUS_OK )
        {
            eRetval = DAC_OK;
        }
        else if (eOpResult == SYS_I2C_STATUS_BUSY)
        {
            eRetval = DAC_BUSY;
        }
        else
        {
            eRetval = DAC_ERROR;
        }
    }

    return eRetval;
}

/* Public function definition ----------------------------------------------*/

DacStatus_t DacInit(DacMcp4728_t *xDacHandler)
{
    USER_ASSERT(xDacHandler != NULL);
    DacStatus_t eRetval = lowlevel_init(xDacHandler->u8IfPort);

    if (eRetval == DAC_OK)
    {
        uint8_t u8Data[CMD_BUFFER_LEN] = {0};
        uint16_t u16CmdLen = 0;

        /* Config Vref */
        u16CmdLen = 0;
        u8Data[u16CmdLen] = (uint8_t)(DAC_CMD_WRITE_VREF << CMD_BIT_SHIFT);
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eVref) << CMD_VREF_GAIN_A_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eVref) << CMD_VREF_GAIN_B_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eVref) << CMD_VREF_GAIN_C_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eVref) << CMD_VREF_GAIN_D_BIT_SHIFT );
        u16CmdLen++;

        if (eRetval == DAC_OK)
        {
            eRetval = lowlevel_send(xDacHandler->u8IfPort, xDacHandler->u8Address, u8Data, u16CmdLen);
            
            uint32_t u32RetryCount = I2C_RETRY;
            while (lowlevel_IfStatus(xDacHandler->u8IfPort) == DAC_BUSY && (eRetval == DAC_OK))
            {
                /* TODO: add timeout */
                if (u32RetryCount == 0)
                {
                    break;
                }
                u32RetryCount--;
            }
        }

        /* Config Gain*/
        u16CmdLen = 0;
        u8Data[u16CmdLen] = (uint8_t)(DAC_CMD_WRITE_GAIN << CMD_BIT_SHIFT);
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eGain) << CMD_VREF_GAIN_A_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eGain) << CMD_VREF_GAIN_B_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eGain) << CMD_VREF_GAIN_C_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->eGain) << CMD_VREF_GAIN_D_BIT_SHIFT );
        u16CmdLen++;

        if (eRetval == DAC_OK)
        {
            eRetval = lowlevel_send(xDacHandler->u8IfPort, xDacHandler->u8Address, u8Data, u16CmdLen);

            uint32_t u32RetryCount = I2C_RETRY;
            while (lowlevel_IfStatus(xDacHandler->u8IfPort) == DAC_BUSY && (eRetval == DAC_OK))
            {
                /* TODO: add timeout */
                if (u32RetryCount == 0)
                {
                    break;
                }
                u32RetryCount--;
            }
        }

        /* Config PowerDown*/
        u16CmdLen = 0;
        u8Data[u16CmdLen] = (uint8_t)(DAC_CMD_WRITE_PD << CMD_BIT_SHIFT);
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->ePd) << CMD_PD_A_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->ePd) << CMD_PD_B_BIT_SHIFT );
        u16CmdLen++;
        u8Data[u16CmdLen] = (uint8_t)( (uint8_t)(xDacHandler->ePd) << CMD_PD_C_BIT_SHIFT );
        u8Data[u16CmdLen] |= (uint8_t)( (uint8_t)(xDacHandler->ePd) << CMD_PD_D_BIT_SHIFT );
        u16CmdLen++;

        if (eRetval == DAC_OK)
        {
            eRetval = lowlevel_send(xDacHandler->u8IfPort, xDacHandler->u8Address, u8Data, u16CmdLen);

            uint32_t u32RetryCount = I2C_RETRY;
            while (lowlevel_IfStatus(xDacHandler->u8IfPort) == DAC_BUSY && (eRetval == DAC_OK))
            {
                /* TODO: add timeout */
                if (u32RetryCount == 0)
                {
                    break;
                }
                u32RetryCount--;
            }
        }
    }

    return eRetval;
}

DacStatus_t DacUpdateAll(DacMcp4728_t *xDacHandler, uint16_t *pu16Count)
{
    USER_ASSERT(xDacHandler != NULL);
    USER_ASSERT(pu16Count != NULL);

    uint8_t u8Data[CMD_BUFFER_LEN] = {0};
    uint16_t u16CmdLen = 0;
    DacStatus_t eRetval = DAC_NOTDEF;

    /* Payload for fast write Channels A,B,C,D */
    for (uint32_t u32Channel = 0; u32Channel < DAC_CH_MAX_NUM; u32Channel++)
    {
        u8Data[u16CmdLen] = (uint8_t)(DAC_CMD_FAST_WRITE << CMD_BIT_SHIFT);
        u8Data[u16CmdLen] |= (uint8_t)(xDacHandler->ePd << 4U);
        u8Data[u16CmdLen] |= (uint8_t)(*pu16Count >> 8U);
        u16CmdLen++;
        u8Data[u16CmdLen] = (uint8_t)(*pu16Count);
        u16CmdLen++;
        pu16Count++;
    }

    eRetval = lowlevel_send(xDacHandler->u8IfPort, xDacHandler->u8Address, u8Data, u16CmdLen);

    return eRetval;
}

DacStatus_t DacUpdateChannel(DacMcp4728_t *xDacHandler, DacChannel_t eChannel, uint16_t u16Count)
{
    USER_ASSERT(xDacHandler != NULL);
    USER_ASSERT(u16Count < DAC_MAX_COUNT);
    USER_ASSERT(eChannel < DAC_CH_MAX_NUM);
    DacStatus_t eRetval = DAC_NOTDEF;
    uint8_t u8Data[CMD_BUFFER_LEN] = {0};
    uint16_t u16CmdLen = 0;

    u8Data[u16CmdLen] = (uint8_t)(DAC_CMD_WRITE_REG_SING << CMD_BIT_SHIFT);
    u8Data[u16CmdLen] |= (uint8_t)(eChannel << 1U);
    u16CmdLen++;
    u8Data[u16CmdLen] = (uint8_t)(xDacHandler->eVref    << 7U);
    u8Data[u16CmdLen] |= (uint8_t)(xDacHandler->ePd     << 5U);
    u8Data[u16CmdLen] |= (uint8_t)(xDacHandler->eGain   << 4U);
    u8Data[u16CmdLen] |= (uint8_t)(u16Count >> 8U);
    u16CmdLen++;
    u8Data[u16CmdLen] = (uint8_t)(u16Count);
    u16CmdLen++;

    eRetval = lowlevel_send(xDacHandler->u8IfPort, xDacHandler->u8Address, u8Data, u16CmdLen);

    return eRetval;
}

/* EOF */