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

/* Private declarations -----------------------------------------------------*/
/* Private definitions ------------------------------------------------------*/

void vApplicationTickHook(void)
{
  HAL_IncTick();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  ERR_ASSERT(0U);
}

/* Public functions ---------------------------------------------------------*/

void SYS_RTOS_ClockConfig(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    USER_ASSERT(0);
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    USER_ASSERT(0);
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    USER_ASSERT(0);
  }
}

/*EOF*/