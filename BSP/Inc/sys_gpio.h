/**
 * @file sys_gpio.h
 * @author Sebastián Del Moral Gallardo (serbmorgal@gmail.com)
 * @brief Sustem Support Packet for GPIO control.
 * @version 0.1
 * @date 2020-11-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_GPIO_H
#define __SYS_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Exported defines ---------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

/** List of defined GPIOs*/
typedef enum sys_gpio_port
{
    SYS_GPIO_00 = 0U,
    SYS_GPIO_01,
    SYS_GPIO_MAX_NUM,
    SYS_SERIAL_NOTDEF = 0xFFU,
} sys_gpio_port_t;

/** List of defined GPIO modes*/
typedef enum sys_gpio_mode
{
    SYS_GPIO_MODE_OUT = 0U,
    SYS_GPIO_MODE_IN,
    SYS_GPIO_MODE_EXTI,
    SYS_GPIO_MODE_NOTDEF = 0xFFU,
} sys_gpio_mode_t;

/** List of defined GPIOs*/
typedef enum sys_gpio_state
{
    SYS_GPIO_STATE_RESET = 0x00,
    SYS_GPIO_STATE_SET = 0x01,
    SYS_GPIO_STATE_NOTDEF = 0xFFU,
} sys_gpio_state_t;

/** Operation status */
typedef enum sys_gpio_status
{
    SYS_GPIO_STATUS_ERROR =   0U,
    SYS_GPIO_STATUS_OK,
    SYS_GPIO_STATUS_BUSY,
    SYS_GPIO_STATUS_NOTDEF =   0xFFU,
} sys_gpio_status_t;

/** Gpio event */
typedef enum sys_gpio_event
{
    SYS_GPIO_EXTI_EVENT = 0U,
    SYS_GPIO_EVENT_ERROR,
    SYS_GPIO_EVENT_NOTDEF = 0xFF,
} sys_gpio_event_t;

/** Data reception callback */
typedef void (* sys_gpio_event_cb)(sys_gpio_event_t event);

/* Exported macro -----------------------------------------------------------*/
/* Exported functions prototypes --------------------------------------------*/

/**
  * @brief  Initialization of gpio hw
  * @param  dev gpio interface number to init
  * @param  mode gpio mode
  * @param  event_cb callback to indicate events associated to hw
  * @retval Operation status
*/
sys_gpio_status_t SYS_GPIO_Init(sys_gpio_port_t gpioId, sys_gpio_mode_t mode, sys_gpio_event_cb event_cb);

/**
  * @brief  De-initialization of gpio interface
  * @param  dev gpio interface number to deinit
  * @retval Operation status
  */
sys_gpio_status_t SYS_GPIO_DeInit(sys_gpio_port_t gpioId);

/**
  * @brief  Read gpio current states
  * @param  gpioId gpio id
  * @retval gpio state
  */
sys_gpio_state_t SYS_GPIO_Read(sys_gpio_port_t gpioId);

/**
  * @brief  Set a state on gpio
  * @param  gpioId gpio id
  * @param  state new state to assign
  * @retval Operation status
  */
sys_gpio_status_t SYS_GPIO_Set(sys_gpio_port_t gpioId, sys_gpio_state_t state);

/**
  * @brief  Toggle a state on gpio
  * @param  gpioId gpio id
  * @retval Operation status
  */
sys_gpio_status_t SYS_GPIO_Toggle(sys_gpio_port_t gpioId);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_GPIO_H */

/*EOF*/