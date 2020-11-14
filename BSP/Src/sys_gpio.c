/**
 * @file sys_gpio.c
 * @author Sebastián Del Moral Gallardo (serbmorgal@gmail.com)
 * @brief Sustem Support Packet for GPIO control.
 * @version 0.1
 * @date 2020-11-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Private includes ---------------------------------------------------------*/
#include "sys_gpio.h"
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

/* Private defines ----------------------------------------------------------*/

#define GPIO_0_PORT     GPIOC
#define GPIO_1_PORT     GPIOA

#define GPIO_0_PIN      ( GPIO_PIN_13 )
#define GPIO_1_PIN      ( GPIO_PIN_5 )

#define GPIO_0_CLK_ON   __HAL_RCC_GPIOC_CLK_ENABLE()
#define GPIO_1_CLK_ON   __HAL_RCC_GPIOA_CLK_ENABLE()

/* Private vairables --------------------------------------------------------*/

/* Array with all gpio callbacks*/
static sys_gpio_event_cb gpio_cb_list[SYS_GPIO_MAX_NUM] = { 0 };

/* Private types ------------------------------------------------------------*/
/* Private functions prototypes ---------------------------------------------*/
/* Private functions definition ---------------------------------------------*/
/* HAL Callback -------------------------------------------------------------*/

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    sys_gpio_port_t eGpioId = SYS_SERIAL_NOTDEF;

    if (GPIO_Pin == GPIO_0_PIN)
    {
        eGpioId = SYS_GPIO_00;
    }
    else if (GPIO_Pin == GPIO_1_PIN)
    {
        eGpioId = SYS_GPIO_01;
    }

    if (eGpioId != SYS_SERIAL_NOTDEF)
    {
        if (gpio_cb_list[eGpioId] != NULL)
        {
            gpio_cb_list[eGpioId](SYS_GPIO_EXTI_EVENT);
        }
    }
}

/* Public functions definition ----------------------------------------------*/

sys_gpio_status_t SYS_GPIO_Init(sys_gpio_port_t gpioId, sys_gpio_mode_t mode, sys_gpio_event_cb event_cb)
{
    sys_gpio_status_t eRetval = SYS_GPIO_STATUS_NOTDEF;

    if (gpioId < SYS_GPIO_MAX_NUM)
    {
        GPIO_InitTypeDef  GPIO_InitStruct = {0};

        /* Apply gpio setup */
        if (mode == SYS_GPIO_MODE_OUT)
        {
            GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull  = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        }
        else if (mode == SYS_GPIO_MODE_IN)
        {
            GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        }
        else if (mode == SYS_GPIO_MODE_EXTI)
        {
            gpio_cb_list[gpioId] = event_cb;
            GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
        }
        else
        {
            /* If not valid mode, set status as error to not process */
            eRetval = SYS_GPIO_STATUS_ERROR;
        }

        if (eRetval != SYS_GPIO_STATUS_ERROR)
        {
            switch (gpioId)
            {
                case SYS_GPIO_00:
                    {
                        GPIO_0_CLK_ON;

                        GPIO_InitStruct.Pin = GPIO_0_PIN;
                        HAL_GPIO_Init(GPIO_0_PORT, &GPIO_InitStruct);

                        if (mode == SYS_GPIO_MODE_EXTI)
                        {
                            HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
                            HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
                        }

                        eRetval = SYS_GPIO_STATUS_OK;
                    }
                    break;

                case SYS_GPIO_01:
                    {
                        GPIO_1_CLK_ON;

                        GPIO_InitStruct.Pin = GPIO_1_PIN;
                        HAL_GPIO_Init(GPIO_1_PORT, &GPIO_InitStruct);

                        if (mode == SYS_GPIO_MODE_EXTI)
                        {
                            HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
                            HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
                        }

                        eRetval = SYS_GPIO_STATUS_OK;
                    }
                    break;

                default:
                    {
                        /* Unknown gpio def */
                        eRetval = SYS_GPIO_STATUS_NOTDEF;
                    }
                    break;
            }
        }
    }

    return eRetval;
}

sys_gpio_status_t SYS_GPIO_DeInit(sys_gpio_port_t gpioId)
{
    sys_gpio_status_t eRetval = SYS_GPIO_STATUS_NOTDEF;

    if (gpioId < SYS_GPIO_MAX_NUM)
    {
        switch (gpioId)
        {
            case SYS_GPIO_00:
                {
                    HAL_GPIO_DeInit(GPIO_0_PORT, GPIO_0_PIN);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            case SYS_GPIO_01:
                {
                    HAL_GPIO_DeInit(GPIO_1_PORT, GPIO_1_PIN);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            default:
                {
                    /* Unknown gpio def */
                    eRetval = SYS_GPIO_STATUS_NOTDEF;
                }
                break;
        }
    }

    return eRetval;
}

sys_gpio_state_t SYS_GPIO_Read(sys_gpio_port_t gpioId)
{
    sys_gpio_state_t eState = SYS_GPIO_STATE_NOTDEF;

    if (gpioId < SYS_GPIO_MAX_NUM)
    {
        switch (gpioId)
        {
            case SYS_GPIO_00:
                {
                    eState = ( HAL_GPIO_ReadPin(GPIO_0_PORT, GPIO_0_PIN) == GPIO_PIN_RESET ) ? SYS_GPIO_STATE_RESET : SYS_GPIO_STATE_SET;
                }
                break;

            case SYS_GPIO_01:
                {
                    eState = ( HAL_GPIO_ReadPin(GPIO_1_PORT, GPIO_1_PIN) == GPIO_PIN_RESET ) ? SYS_GPIO_STATE_RESET : SYS_GPIO_STATE_SET;
                }
                break;

            default:
                break;
        }
    }

    return eState;
}

sys_gpio_status_t SYS_GPIO_Set(sys_gpio_port_t gpioId, sys_gpio_state_t state)
{
    sys_gpio_status_t eRetval = SYS_GPIO_STATUS_NOTDEF;

    if (gpioId < SYS_GPIO_MAX_NUM)
    {
        GPIO_PinState eHalState = (state == SYS_GPIO_STATE_RESET) ? GPIO_PIN_RESET : GPIO_PIN_SET;

        switch (gpioId)
        {
            case SYS_GPIO_00:
                {
                    HAL_GPIO_WritePin(GPIO_0_PORT, GPIO_0_PIN, eHalState);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            case SYS_GPIO_01:
                {
                    HAL_GPIO_WritePin(GPIO_1_PORT, GPIO_1_PIN, eHalState);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            default:
                break;
        }
    }

    return eRetval;
}

sys_gpio_status_t SYS_GPIO_Toggle(sys_gpio_port_t gpioId)
{
    sys_gpio_status_t eRetval = SYS_GPIO_STATUS_NOTDEF;

    if (gpioId < SYS_GPIO_MAX_NUM)
    {
        switch (gpioId)
        {
            case SYS_GPIO_00:
                {
                    HAL_GPIO_TogglePin(GPIO_0_PORT, GPIO_0_PIN);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            case SYS_GPIO_01:
                {
                    HAL_GPIO_TogglePin(GPIO_1_PORT, GPIO_1_PIN);
                    eRetval = SYS_GPIO_STATUS_OK;
                }
                break;

            default:
                break;
        }
    }

    return eRetval;
}

/*EOF*/