/**
 * @file sys_serial.h
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief System support packet to handle serial interfaces
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_SERIAL_H
#define __SYS_SERIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Exported defines ---------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

/** List of serial devices*/
typedef enum
{
    SYS_SERIAL_0 = 0U,
    SYS_SERIAL_1 = 1U,
    SYS_SERIAL_NODEF = 0xFFU,
} sys_serial_port_t;

/** Operation status */
typedef enum
{
    SYS_SERIAL_STATUS_ERROR =   0U,
    SYS_SERIAL_STATUS_OK =      1U,
    SYS_SERIAL_STATUS_BUSY =    2U,
    SYS_SERIAL_STATUS_NODEF =   0xFFU,
} sys_serial_status_t;

/** Serial event */
typedef enum
{
    SYS_SERIAL_EVENT_RX_IDLE = 0U,
    SYS_SERIAL_EVENT_TX_DONE,
    SYS_SERIAL_EVENT_RX_BUF_FULL,
    SYS_SERIAL_EVENT_ERROR,
    SYS_SERIAL_EVENT_NOTDEF = 0xFF,
} sys_serial_event_t;

/** Data reception callback */
typedef void (* sys_serial_event_cb)(sys_serial_event_t event);

/* Exported macro -----------------------------------------------------------*/
/* Exported functions prototypes --------------------------------------------*/

/**
  * @brief  Initialization of serial interface
  * @param  dev serial interface number to init
  * @param  event_cb callback to indicate data has rrive to the serial interface
  * @retval Operation status
*/
sys_serial_status_t SYS_SERIAL_Init(sys_serial_port_t dev, sys_serial_event_cb event_cb);

/**
  * @brief  De-initialization of serial interface
  * @param  dev serial interface number to deinit
  * @retval Operation status
  */
sys_serial_status_t SERIAL_DeInit(sys_serial_port_t dev);

/**
  * @brief  Send serial data through defined interface
  * @param  dev serial interface number to use
  * @param  pdata pointer of data to send
  * @param  len number of bytes to send
  * @retval Operation status
  */
sys_serial_status_t SYS_SERIAL_Send(sys_serial_port_t dev, uint8_t *pdata, uint16_t len);

/**
  * @brief  Read data stored on serial buffer
  * @param  dev serial interface number to use
  * @param  pdata pointer where store read data
  * @param  max_len maximun number of bytes to read
  * @retval number of bytes read
  */
uint16_t SYS_SERIAL_Read(sys_serial_port_t dev, uint8_t *pdata, uint16_t max_len);

/**
  * @brief  Send serial data through defined interface
  * @param  dev serial interface number to use
  * @param  pdata pointer of data to send
  * @param  len number of bytes to send
  * @retval number of bytes in buffer
  */
uint16_t SYS_SERIAL_GetReadCount(sys_serial_port_t dev);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_SERIAL_H */

/*EOF*/