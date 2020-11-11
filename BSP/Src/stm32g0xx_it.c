/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "stm32g0xx_it.h"
#include "sys_rtos.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart3;

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/

/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

#ifndef CUSTOM_HARD_FAULT
/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}
#endif

#ifndef USE_USER_RTOS
/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}
#endif

#ifndef USE_USER_RTOS
/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}
#endif

#ifdef USE_USER_RTOS_TICK
/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}
#endif

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

/**
  * @brief This function handles DMA1 channel 4 to channel 7 interrupts.
  */
void DMA1_Ch4_7_DMAMUX1_OVR_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim3);
}

/**
  * @brief Map timer update event.
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM3)
  {
    SYS_RTOS_TimerTick();
  }
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);

  /* Handle idle event on usart */
  if (__HAL_UART_GET_IT(&huart2, UART_IT_IDLE))
  {
    __HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_IDLEF);

    /* Abort and retrigger reception */
    HAL_UART_AbortReceive_IT(&huart2);
  }
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_4_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart3);

  /* Handle idle event on usart */
  if (__HAL_UART_GET_IT(&huart3, UART_IT_IDLE))
  {
    __HAL_UART_CLEAR_IT(&huart3, UART_CLEAR_IDLEF);

    /* Abort and retrigger reception */
    HAL_UART_AbortReceive_IT(&huart3);
  }
}

/*EOF*/
