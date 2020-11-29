/**
 * @file dac_mcp4728.h
 * @author Sebastián Del Moral Gallardo (sebmorgal@gmail.com)
 * @brief Library to handle DAC MCP4728 (https://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf)
 * @version 0.1
 * @date 2020-11-28
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DAC_MCP4728_H
#define __DAC_MCP4728_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Exported types ------------------------------------------------------------*/

/** Defined dac lib status codes */
typedef enum DacStatus
{
    DAC_OK = 0x00,
    DAC_ERROR,
    DAC_BUSY,
    DAC_NOTDEF = 0xFF,
} DacStatus_t;

/** Defined DAC channels */
typedef enum DacChannel
{
    DAC_CH_A = 0,
    DAC_CH_B,
    DAC_CH_C,
    DAC_CH_D,
    DAC_CH_MAX_NUM,
} DacChannel_t;

/** Defined Vref option */
typedef enum DacVref
{
    DAC_VREF_INT,           /**< Internal Vref (2.048V) */
    DAC_VREF_EXT            /**< Externa Vref (VDD) */
} DacVref_t;

/** Gain options */
typedef enum DacGain
{
    DAC_GAIN_1,             /**< Gain of 1 */
    DAC_GAIN_2              /**< Gain of 2, only for Vref Int config */
} DacGain_t;

/** Power-Down options */
typedef enum DacPowerDown
{
    DAC_PD_NORMAL = 0x00,   /**< Normal mode */
    DAC_PD_LD_1K = 0x01,    /**< Output load 1 Kohm */
    DAC_PD_LD_100K = 0x02,  /**< Output load 100 Kohm */
    DAC_PD_LD_500K = 0x03   /**< Output load 500 Kohm */
} DacPowerDown_t;

/** Control struct with all functional parameters for MCP4728 */
typedef struct DacMcp4728_t
{
    uint8_t u8IfPort;
    uint8_t u8Address;
    DacVref_t eVref;
    DacGain_t eGain;
    DacPowerDown_t ePd;
} DacMcp4728_t;

/* Control structure definition ----------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/

/* Base Address */
#define DAC_BASE_ADDR   (0xC0)

/* Maximun count value for DAC (12b) */
#define DAC_MAX_COUNT   (4096U)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Config Dac with setup defined in handler.
 * 
 * @param xDacHandler handler with parameters to set.
 * @return DacStatus_t operation status.
 */
DacStatus_t DacInit(DacMcp4728_t *xDacHandler);

/**
 * @brief Set defined values in all available channels
 * 
 * @param xDacHandler hanlder of device to use
 * @param pu16Count array of 4 values (0-4096) to set on each channel (0-A,1-B,2-C,3-D)
 * @return DacStatus_t operation status.
 */
DacStatus_t DacUpdateAll(DacMcp4728_t *xDacHandler, uint16_t *pu16Count);

/**
 * @brief Set value for only one channel
 * 
 * @param xDacHandler dac handler structure
 * @param eChannel channel selected to update
 * @param u16Count count to setup on cahnnel (0-4096)
 * @return DacStatus_t operation status.
 */
DacStatus_t DacUpdateChannel(DacMcp4728_t *xDacHandler, DacChannel_t eChannel, uint16_t u16Count);

#ifdef __cplusplus
}
#endif

#endif /* __DAC_MCP4728_H */

/* EOF */