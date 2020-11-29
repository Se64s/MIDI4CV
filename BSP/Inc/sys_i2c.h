/**
 * @file    sys_i2c.h
 * @author  Sebastián Del Moral Gallardo (serbmorgal@gmail.com)
 * @brief   System Support Packet for I2C interface.
 * @version 0.1
 * @date    2020-11-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_I2C_H
#define __SYS_I2C_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Exported defines ---------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

/** List of i2c devices*/
typedef enum sys_i2c_port
{
    SYS_I2C_0 = 0U,
    SYS_I2C_MAX_NUM_PORT,
    SYS_I2C_NOTDEF = 0xFFU,
} sys_i2c_port_t;

/** Operation status */
typedef enum sys_i2c_status
{
    SYS_I2C_STATUS_ERROR =  0U,
    SYS_I2C_STATUS_OK =     1U,
    SYS_I2C_STATUS_BUSY =   2U,
    SYS_I2C_STATUS_NOTDEF = 0xFFU,
} sys_i2c_status_t;

/** I2C event */
typedef enum sys_i2c_event
{
    SYS_I2C_EVENT_TRANSFER_DONE = 0U,
    SYS_I2C_EVENT_ABORT,
    SYS_I2C_EVENT_ERROR,
    SYS_I2C_EVENT_NOTDEF = 0xFF,
} sys_i2c_event_t;

/** I2C event callback */
typedef void (* sys_i2c_event_cb)(sys_i2c_event_t event);

/* Exported macro -----------------------------------------------------------*/
/* Exported functions prototypes --------------------------------------------*/

/**
  * @brief  Initialization of i2c interface
  * @param  dev i2c port
  * @param  event_cb callback
  * @retval Operation status
  */
sys_i2c_status_t SYS_I2C_Init(sys_i2c_port_t dev, sys_i2c_event_cb event_cb);

/**
  * @brief  Deinit i2c interface
  * @param  dev i2c port
  * @retval Operation status
  */
sys_i2c_status_t SYS_I2C_DeInit(sys_i2c_port_t dev);

/**
  * @brief  Get interface state.
  * @param  dev i2c port
  * @retval Operation status
  */
sys_i2c_status_t SYS_I2C_GetState(sys_i2c_port_t dev);

/**
 * @brief Write data to a slave device
 * @param dev i2c port to use
 * @param u8Addr i2c address
 * @param pu8Data pointer of data to send
 * @param u16Size number of bytes to send
 * @return sys_i2c_status_t
 */
sys_i2c_status_t SYS_I2C_MasterWrite(sys_i2c_port_t dev, uint8_t u8Addr, uint8_t *pu8Data, uint16_t u16Size);

/**
 * @brief Read data from slave
 * @param dev i2c port to use
 * @param u8Addr i2c address
 * @param pu8Data read array
 * @param u16Size mazimun read size
 * @return sys_i2c_status_t operation status
 */
sys_i2c_status_t SYS_I2C_MasterRead(sys_i2c_port_t dev, uint8_t u8Addr,  uint8_t *pu8Data, uint16_t u16Size);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_I2C_H */

/* EOF */