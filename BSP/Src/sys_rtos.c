/**
 * @file sys_rtos.c
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief Support code for RTOS.
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes -----------------------------------------------------------------*/
#include "sys_rtos.h"
#include "stm32g0xx_hal.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif
#include "task.h"

/* Private variables --------------------------------------------------------*/
/** Control timer var to handle RTOS clock */
TIM_HandleTypeDef htim3;

/* Private macro -----------------------------------------------------------*/
#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private defines -----------------------------------------------------------*/

/*
  In this example TIM3 input clock (TIM3CLK)  is set to APB1 clock (PCLK1),
  since APB1 prescaler is equal to 1.
    TIM3CLK = PCLK1
    PCLK1 = HCLK
    => TIM3CLK = HCLK = SystemCoreClock
  To get TIM3 counter clock at 10 KHz, the Prescaler is computed as following:
  Prescaler = (TIM3CLK / TIM3 counter clock) - 1
  Prescaler = (SystemCoreClock /10 KHz) - 1

  Note:
    SystemCoreClock variable holds HCLK frequency and is defined in system_stm32g0xx.c file.
    Each time the core clock (HCLK) changes, user had to update SystemCoreClock
    variable value. Otherwise, any configuration based on this variable will be incorrect.
    This variable is updated in three ways:
    1) by calling CMSIS function SystemCoreClockUpdate()
    2) by calling HAL API function HAL_RCC_GetSysClockFreq()
    3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
*/

/* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
#define PRESCALER_VALUE (uint32_t)((SystemCoreClock / 10000) - 1)

/*
  Initialize TIMx peripheral as follows:
        + Period = 10000 - 1
        + Prescaler = (SystemCoreClock/10000) - 1
        + ClockDivision = 0
        + Counter direction = Up
*/
#define PERIOD_VALUE (10 - 1);

/* Private declarations -----------------------------------------------------*/

/**
  * @brief Low level timer init
  * @param None
  * @retval None
  */
static void BSP_SYS_TIMER_Init(void);

/**
  * @brief Init timer count
  * @param None
  * @retval None
  */
static void BSP_SYS_TIMER_Start(void);

/* Private definitions ------------------------------------------------------*/

static void BSP_SYS_TIMER_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = PRESCALER_VALUE;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = PERIOD_VALUE;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    ERR_ASSERT(0U);
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    ERR_ASSERT(0U);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    ERR_ASSERT(0U);
  }
}

static void BSP_SYS_TIMER_Start(void)
{
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
    /* Starting Error */
    ERR_ASSERT(0U);
  }
}

/* RTOS app hook ------------------------------------------------------------*/

#ifdef USE_USER_RTOS_TICK
void vPortSetupTimerInterrupt(void)
{
  /* In case of use of other timer source, init here */
  BSP_SYS_TIMER_Init();
  BSP_SYS_TIMER_Start();
}
#endif

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  ERR_ASSERT(0U);
}

/* Public functions ---------------------------------------------------------*/

void SYS_RTOS_ClockConfig(void)
{
  /* Rtos clock setup done in vPortSetupTimerInterrupt */
}

void SYS_RTOS_TimerTick(void)
{
  uint32_t ulPreviousMask;

  ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
  {
      /* Increment the RTOS tick. */
      if( xTaskIncrementTick() != pdFALSE )
      {
          /* Pend a context switch. */
          portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
      }
  }

  portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
}

/*EOF*/