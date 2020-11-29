/**
 * @file    sys_i2c.c
 * @author  Sebastián Del Moral Gallardo (serbmorgal@gmail.com)
 * @brief   System Support Packet for I2C interface.
 * @version 0.1
 * @date    2020-11-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Private includes --------------------------------------------------------*/
#include "sys_i2c.h"
#include "stm32g0xx_hal.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Private macro -----------------------------------------------------------*/
#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private defines ---------------------------------------------------------*/

/* Own device address */
#define I2C_0_ADDRESS       0x3E

/* Private variable --------------------------------------------------------*/

/* I2C HAL handlers*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;
DMA_HandleTypeDef hdma_i2c1_rx;

/* Peripheral event callbacks */
static sys_i2c_event_cb i2c1_event_cb = NULL;

/* Private macro -----------------------------------------------------------*/
/* Private functions prototypes --------------------------------------------*/
/* Private functions definition --------------------------------------------*/
/* HAL Callback ------------------------------------------------------------*/

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(SYS_I2C_EVENT_TRANSFER_DONE);
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(SYS_I2C_EVENT_TRANSFER_DONE);
        }
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(SYS_I2C_EVENT_ERROR);
        }
    }
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(SYS_I2C_EVENT_ABORT);
        }
    }
}

/* Public function definition ----------------------------------------------*/

sys_i2c_status_t SYS_I2C_Init(sys_i2c_port_t dev, sys_i2c_event_cb event_cb)
{
    USER_ASSERT(dev < SYS_I2C_MAX_NUM_PORT);

    sys_i2c_status_t eRetval = SYS_I2C_STATUS_ERROR;

    if (dev == SYS_I2C_0)
    {
        if (event_cb != NULL)
        {
            i2c1_event_cb = event_cb;
        }

        hi2c1.Instance = I2C1;
        hi2c1.Init.Timing = 0x00303D5B;
        hi2c1.Init.OwnAddress1 = I2C_0_ADDRESS;
        hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hi2c1.Init.OwnAddress2 = 0;
        hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
        hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
        if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        {
            USER_ASSERT(0);
        }

        /** Configure Analogue filter 
         */
        if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        {
            USER_ASSERT(0);
        }

        /** Configure Digital filter 
         */
        if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0x0) != HAL_OK)
        {
            USER_ASSERT(0);
        }

        /** I2C Fast mode Plus enable 
         */
        HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);

        eRetval = SYS_I2C_STATUS_OK;
    }

    return eRetval;
}

sys_i2c_status_t SYS_I2C_DeInit(sys_i2c_port_t dev)
{
    USER_ASSERT(dev < SYS_I2C_MAX_NUM_PORT);

    sys_i2c_status_t eRetval = SYS_I2C_STATUS_ERROR;

    if (dev == SYS_I2C_0)
    {
        if (HAL_I2C_DeInit(&hi2c1) != HAL_OK)
        {
            USER_ASSERT(0);
        }

        eRetval = SYS_I2C_STATUS_OK;
    }

    return eRetval;
}

sys_i2c_status_t SYS_I2C_GetState(sys_i2c_port_t dev)
{
    USER_ASSERT(dev < SYS_I2C_MAX_NUM_PORT);

    sys_i2c_status_t eRetval = SYS_I2C_STATUS_ERROR;

    if (dev == SYS_I2C_0)
    {
        if (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
        {
            eRetval = SYS_I2C_STATUS_BUSY;
        }
        else
        {
            eRetval = SYS_I2C_STATUS_OK;
        }
    }

    return eRetval;
}

sys_i2c_status_t SYS_I2C_MasterWrite(sys_i2c_port_t dev, uint8_t u8Addr, uint8_t *pu8Data, uint16_t u16Size)
{
    USER_ASSERT(dev < SYS_I2C_MAX_NUM_PORT);
    USER_ASSERT(pu8Data != NULL);

    sys_i2c_status_t eRetval = SYS_I2C_STATUS_ERROR;
    I2C_HandleTypeDef *pxDevHandler = NULL;

    if (dev == SYS_I2C_0)
    {
        pxDevHandler = &hi2c1;
    }

    if (pxDevHandler != NULL)
    {
        uint16_t u16DevAddr = (uint16_t)u8Addr;

        HAL_StatusTypeDef xOpStatus = HAL_I2C_Master_Transmit_DMA(pxDevHandler, u16DevAddr, pu8Data, u16Size);

        if (xOpStatus == HAL_OK)
        {
            eRetval = SYS_I2C_STATUS_OK;
        }
        else if (xOpStatus == HAL_BUSY)
        {
            eRetval = SYS_I2C_STATUS_BUSY;
        }
        else
        {
            eRetval = SYS_I2C_STATUS_ERROR;
        }
    }

    return eRetval;
}

sys_i2c_status_t SYS_I2C_MasterRead(sys_i2c_port_t dev, uint8_t u8Addr,  uint8_t *pu8Data, uint16_t u16Size)
{
    USER_ASSERT(dev < SYS_I2C_MAX_NUM_PORT);
    USER_ASSERT(pu8Data != NULL);

    sys_i2c_status_t eRetval = SYS_I2C_STATUS_ERROR;
    I2C_HandleTypeDef *pxDevHandler = NULL;

    if (dev == SYS_I2C_0)
    {
        pxDevHandler = &hi2c1;
    }

    if (pxDevHandler != NULL)
    {
        uint16_t u16DevAddr = (uint16_t)u8Addr;

        HAL_StatusTypeDef xOpStatus = HAL_I2C_Master_Receive_DMA(pxDevHandler, u16DevAddr, pu8Data, u16Size);

        if (xOpStatus == HAL_OK)
        {
            eRetval = SYS_I2C_STATUS_OK;
        }
        else if (xOpStatus == HAL_BUSY)
        {
            eRetval = SYS_I2C_STATUS_BUSY;
        }
        else
        {
            eRetval = SYS_I2C_STATUS_ERROR;
        }
    }

    return eRetval;
}

/* EOF */