/**
 * @file sys_serial.c
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief System support packet to handle serial interfaces
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Private includes --------------------------------------------------------*/
#include "sys_serial.h"
#include "circular_buffer.h"
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
/* Serial 0 circular buffer size */
#define SERIAL_0_CBUF_SIZE  (350U)

/* Serial 0 peripheral buffer size */
#define SERIAL_0_RX_SIZE    (16U)

/* Serial 0 baudrate */
#define SERIAL_0_BAUDRATE   (115200U)

/* Serial 1 circular buffer size */
#define SERIAL_1_CBUF_SIZE  (350U)

/* Serial 1 peripheral buffer size */
#define SERIAL_1_RX_SIZE    (16U)

/* Serial 0 baudrate */
#define SERIAL_1_BAUDRATE   (31250U)

/* Private variable ---------------------------------------------------------*/

/* Serial 0 resources */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

static circular_buf_t cbuff_uart2;
static uint8_t rx_buf_uart2[SERIAL_0_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart2[SERIAL_0_CBUF_SIZE] = {0};

static sys_serial_event_cb uart2_event_cb = NULL;

/* Serial 1 resources */
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart3_rx;

static circular_buf_t cbuff_uart3;
static uint8_t rx_buf_uart3[SERIAL_1_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart3[SERIAL_1_CBUF_SIZE] = {0};

static sys_serial_event_cb uart3_event_cb = NULL;

/* Private macro -----------------------------------------------------------*/
/* Private functions prototypes --------------------------------------------*/

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void BSP_USART2_UART_Init(void);

/**
  * @brief USART2 DeInitialization Function
  * @param None
  * @retval None
  */
static void BSP_USART2_UART_Deinit(void);

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void BSP_USART3_UART_Init(void);

/**
  * @brief USART3 DeInitialization Function
  * @param None
  * @retval None
  */
static void BSP_USART3_UART_Deinit(void);

/* Private functions definition --------------------------------------------*/

static void BSP_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = SERIAL_0_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        USER_ASSERT(0);
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        USER_ASSERT(0);
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        USER_ASSERT(0);
    }
    if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
    {
        USER_ASSERT(0);
    }

    /* Init additional resurces */
    circular_buf_init(&cbuff_uart2, rx_cbuf_uart2, SERIAL_0_CBUF_SIZE);

    /* Enable idle irq */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

static void BSP_USART2_UART_Deinit(void)
{
    /* Disable associated IRQ */
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart2) != HAL_OK)
    {
        USER_ASSERT(0);
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart2);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
}

static void BSP_USART3_UART_Init(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = SERIAL_1_BAUDRATE;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        USER_ASSERT(0);
    }

    /* Init additional resurces */
    circular_buf_init(&cbuff_uart3, rx_cbuf_uart3, SERIAL_1_CBUF_SIZE);

    /* Enable idle irq */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

static void BSP_USART3_UART_Deinit(void)
{
    /* Disable associated IRQ */
    HAL_NVIC_DisableIRQ(USART3_4_IRQn);

    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart3) != HAL_OK)
    {
        USER_ASSERT(0);
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart3);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart3, UART_IT_IDLE);
}

/* HAL Callback -------------------------------------------------------------*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SYS_SERIAL_EVENT_TX_DONE);
        }
    }
    else if (huart->Instance == USART3)
    {
        if (uart3_event_cb != NULL)
        {
            uart3_event_cb(SYS_SERIAL_EVENT_TX_DONE);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uint8_t * pdata = rx_buf_uart2;
        uint32_t rx_size = SERIAL_0_RX_SIZE;
        while (rx_size-- != 0)
        {
            if (circular_buf_put2(&cbuff_uart2, *pdata++) != 0)
            {
                if (uart2_event_cb != NULL)
                {
                    uart2_event_cb(SYS_SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_0_RX_SIZE);
    }
    else if (huart->Instance == USART3)
    {
        uint8_t * pdata = rx_buf_uart3;
        uint32_t rx_size = SERIAL_1_RX_SIZE;
        while (rx_size-- != 0)
        {
            if (circular_buf_put2(&cbuff_uart3, *pdata++) != 0)
            {
                if (uart3_event_cb != NULL)
                {
                    uart3_event_cb(SYS_SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_1_RX_SIZE);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SYS_SERIAL_EVENT_ERROR);
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_0_RX_SIZE);
    }
    else if (huart->Instance == USART3)
    {
        if (uart3_event_cb != NULL)
        {
            uart3_event_cb(SYS_SERIAL_EVENT_ERROR);
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_1_RX_SIZE);
    }
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uint8_t * pdata = rx_buf_uart2;
        uint32_t rx_size = SERIAL_0_RX_SIZE - huart->hdmarx->Instance->CNDTR;
        while (rx_size != 0)
        {
            if (circular_buf_put2(&cbuff_uart2, *pdata++) != 0)
            {
                if (uart2_event_cb != NULL)
                {
                    uart2_event_cb(SYS_SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
            rx_size--;
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_0_RX_SIZE);

        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SYS_SERIAL_EVENT_RX_IDLE);
        }
    }
    else if (huart->Instance == USART3)
    {
        uint8_t * pdata = rx_buf_uart3;
        uint32_t rx_size = SERIAL_1_RX_SIZE - huart->hdmarx->Instance->CNDTR;
        while (rx_size != 0)
        {
            if (circular_buf_put2(&cbuff_uart3, *pdata++) != 0)
            {
                if (uart3_event_cb != NULL)
                {
                    uart3_event_cb(SYS_SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
            rx_size--;
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_1_RX_SIZE);

        if (uart3_event_cb != NULL)
        {
            uart3_event_cb(SYS_SERIAL_EVENT_RX_IDLE);
        }
    }
}

/* Public function definition ----------------------------------------------*/

sys_serial_status_t SYS_SERIAL_Init(sys_serial_port_t dev, sys_serial_event_cb event_cb)
{
    USER_ASSERT((dev == SYS_SERIAL_0) || (dev == SYS_SERIAL_1));

    sys_serial_status_t eRetval = SYS_SERIAL_STATUS_ERROR;

    if (dev == SYS_SERIAL_0)
    {
        /* Init hardware */
        BSP_USART2_UART_Init();

        if (event_cb != NULL)
        {
            uart2_event_cb = event_cb;
        }

        /* Enable reading */
        if (HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_0_RX_SIZE) == HAL_OK)
        {
            eRetval = SYS_SERIAL_STATUS_OK;
        }
    }
    else if (dev == SYS_SERIAL_1)
    {
        /* Init hardware */
        BSP_USART3_UART_Init();

        if (event_cb != NULL)
        {
            uart3_event_cb = event_cb;
        }

        /* Enable reading */
        if (HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_1_RX_SIZE) == HAL_OK)
        {
            eRetval = SYS_SERIAL_STATUS_OK;
        }
    }
    else
    {
        /* No action */
    }

    return eRetval;
}

sys_serial_status_t SERIAL_DeInit(sys_serial_port_t dev)
{
    USER_ASSERT((dev == SYS_SERIAL_0) || (dev == SYS_SERIAL_1));

    sys_serial_status_t eRetval = SYS_SERIAL_STATUS_ERROR;

    if (dev == SYS_SERIAL_0)
    {
        BSP_USART2_UART_Deinit();

        if (uart2_event_cb != NULL)
        {
            uart2_event_cb = NULL;
        }

        eRetval = SYS_SERIAL_STATUS_OK;
    }
    else if (dev == SYS_SERIAL_1)
    {
        BSP_USART3_UART_Deinit();

        if (uart3_event_cb != NULL)
        {
            uart3_event_cb = NULL;
        }

        eRetval = SYS_SERIAL_STATUS_OK;
    }
    else
    {
        /* No action */
    }

    return eRetval;
}

sys_serial_status_t SYS_SERIAL_Send(sys_serial_port_t dev, uint8_t *pdata, uint16_t len)
{
    USER_ASSERT((dev == SYS_SERIAL_0) || (dev == SYS_SERIAL_1));
    USER_ASSERT(pdata != NULL);

    sys_serial_status_t retval = SYS_SERIAL_STATUS_NODEF;
    UART_HandleTypeDef * phuart = NULL;

    /* Get serial handler */
    if (dev == SYS_SERIAL_0)
    {
        phuart = &huart2;
    }
    else if (dev == SYS_SERIAL_1)
    {
        phuart = &huart3;
    }
    else
    {
        /* Nothing to do */
    }

    /* If handler defined, process data */
    if (phuart != NULL)
    {
        HAL_StatusTypeDef hal_ret = HAL_UART_Transmit_DMA(phuart, pdata, len);

        if (hal_ret == HAL_OK)
        {
            retval = SYS_SERIAL_STATUS_OK;
        }
        else if (hal_ret == HAL_BUSY)
        {
            retval = SYS_SERIAL_STATUS_BUSY;
        }
        else
        {
            retval = SYS_SERIAL_STATUS_ERROR;
        }
    }

    return retval;
}

uint16_t SYS_SERIAL_Read(sys_serial_port_t dev, uint8_t *pdata, uint16_t max_len)
{
    USER_ASSERT((dev == SYS_SERIAL_0) || (dev == SYS_SERIAL_1));
    USER_ASSERT(pdata != NULL);

    uint16_t u16ReadCount = 0;

    /* Get serial handler */
    if (dev == SYS_SERIAL_0)
    {
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        for (uint32_t i_data = 0; i_data < max_len; i_data++)
        {
            if (circular_buf_get(&cbuff_uart2, &pdata[i_data]) == 0)
            {
                u16ReadCount++;
            }
            else
            {
                break;
            }
        }
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if (dev == SYS_SERIAL_1)
    {
        HAL_NVIC_DisableIRQ(USART3_4_IRQn);
        for (uint32_t i_data = 0; i_data < max_len; i_data++)
        {
            if (circular_buf_get(&cbuff_uart3, &pdata[i_data]) == 0)
            {
                u16ReadCount++;
            }
            else
            {
                break;
            }
        }
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
    else
    {
        /* Nothing to do */
    }

    return u16ReadCount;
}

uint16_t SYS_SERIAL_GetReadCount(sys_serial_port_t dev)
{
    USER_ASSERT((dev == SYS_SERIAL_0) || (dev == SYS_SERIAL_1));

    uint16_t u16ReadCount = 0;

    /* Get serial handler */
    if (dev == SYS_SERIAL_0)
    {
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        u16ReadCount = circular_buf_size(&cbuff_uart2);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if (dev == SYS_SERIAL_1)
    {
        HAL_NVIC_DisableIRQ(USART3_4_IRQn);
        u16ReadCount = circular_buf_size(&cbuff_uart3);
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
    else
    {
        /* Nothing to do */
    }

    return u16ReadCount;
}

/*EOF*/