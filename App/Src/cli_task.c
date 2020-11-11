/**
  ******************************************************************************
  * @file           : cli_task.c
  * @brief          : Task to handle cli actions and serial debug prints
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cli_task.h"
#include "sys_serial.h"
#include "printf.h"
#include "FreeRTOS_CLI.h"
#include "cli_cmd.h"
#ifdef USE_USER_ASSERT
#include "user_error.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Init message */
#define CLI_INIT_MSG_SPACER "----------"
#define CLI_INIT_MSG_APP    " USER APP:\t"
#define CLI_INIT_MSG_VER    " VERSION:\tv"
#define CLI_INIT_MSG_RTOS   " FreeRTOS:\t"

/* End of line terminator */
#define CLI_EOL         "\r\n"

/* Initial wait */
#define CLI_INIT_DELAY      (0U)

/* Serial port to use */
#define CLI_SERIAL          SYS_SERIAL_0

/* Serial signals */
#define CLI_SIGNAL_TX_DONE  (1UL << 0)
#define CLI_SIGNAL_RX_DONE  (1UL << 1)
#define CLI_SIGNAL_RX_IDLE  (1UL << 2)
#define CLI_SIGNAL_ERROR    (1UL << 3)

/* Private macro -------------------------------------------------------------*/
#ifdef USE_USER_ASSERT
#define USER_ASSERT(A)      ERR_ASSERT(A)
#else
#define USER_ASSERT(A)      (void)(A)
#endif

/* Private variables ---------------------------------------------------------*/

TaskHandle_t cli_task_handle = NULL;
volatile SemaphoreHandle_t cli_serial_mutex = NULL;

static volatile bool ser_tx_done = false;

static char print_output_buffer[CLI_OUTPUT_BUFFER_SIZE];
static char cCliOutputBuffer[configCOMMAND_INT_MAX_OUTPUT_SIZE];
static char cInputBuffer[configCOMMAND_INT_MAX_INPUT_SIZE];

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Init resources for CLI tasks
  * @param pvParameters function paramters
  * @retval None
  */
void _cli_main(void *pvParameters);

/**
  * @brief Callback to handle serial events
  * @param event serial event generated
  * @retval None
  */
void _event_cb(sys_serial_event_t event);

/**
 * @brief Clear buffer and fill it with 0
 * 
 * @param pu8Buff pointer to buffer to clear
 * @param u32BuffLen size of buffer
 */
void _clear_buff(char *pcBuff, uint32_t u32BuffLen);

/**
 * @brief Print initial msg
 * 
 */
void _init_msg(void);

/* Private fuctions ----------------------------------------------------------*/

void _event_cb(sys_serial_event_t event)
{
    BaseType_t wakeTask;

    if (event == SYS_SERIAL_EVENT_RX_IDLE)
    {
        xTaskNotifyFromISR(cli_task_handle, CLI_SIGNAL_RX_IDLE, eSetBits, &wakeTask);
    }
    else if (event == SYS_SERIAL_EVENT_ERROR)
    {
        ser_tx_done = true;
    }
    else if (event == SYS_SERIAL_EVENT_TX_DONE)
    {
        ser_tx_done = true;
    }
    else
    {
        /* code */
    }
}

void _clear_buff(char *pcBuff, uint32_t u32BuffLen)
{
    while (u32BuffLen-- != 0)
    {
        *pcBuff++ = 0;
    }
}

void _init_msg(void)
{
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_INIT_MSG_SPACER);
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_INIT_MSG_APP);
    vCliRawPrintf(MAIN_APP_NAME);
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_INIT_MSG_VER);
    vCliRawPrintf(MAIN_APP_VERSION);
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_INIT_MSG_RTOS);
    vCliRawPrintf(tskKERNEL_VERSION_NUMBER);
    vCliRawPrintf(CLI_EOL);
    vCliRawPrintf(CLI_INIT_MSG_SPACER);
    vCliRawPrintf(CLI_EOL);
}

void _cli_main( void *pvParameters )
{
    uint8_t u8RxData = 0;
    uint32_t i_rx_buff = 0;
    uint32_t tmp_event;

    /* Register used functions */
    cli_cmd_init();

    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(CLI_INIT_DELAY));

    /* Start message */
    _init_msg();

    /* Show init msg */
    vCliPrintf(CLI_TASK_NAME, "Init");

    /* Infinite loop */
    for(;;)
    {
        BaseType_t event_wait = xTaskNotifyWait(0, CLI_SIGNAL_RX_IDLE, &tmp_event, portMAX_DELAY);
        if (event_wait == pdPASS)
        {
            /* Fill input buffer */
            while (SYS_SERIAL_Read(CLI_SERIAL, &u8RxData, 1U) != 0)
            {
                /* End of command detected */
                if ((u8RxData == '\r') || (u8RxData == '\n'))
                {
                    if (i_rx_buff != 0)
                    {
                        vCliPrintf(CLI_TASK_NAME, "cmd: \"%s\"", cInputBuffer);

                        BaseType_t xReturned;

                        do {
                            xReturned = FreeRTOS_CLIProcessCommand(cInputBuffer, cCliOutputBuffer, configCOMMAND_INT_MAX_OUTPUT_SIZE);
                            vCliPrintf(CLI_TASK_NAME, "%s", cCliOutputBuffer);
                            _clear_buff(cCliOutputBuffer, configCOMMAND_INT_MAX_OUTPUT_SIZE);
                        } while(xReturned != pdFALSE);

                        i_rx_buff = 0;
                    }
                    else
                    {
                        vCliPrintf(CLI_TASK_NAME, "$", cInputBuffer);
                    }

                    _clear_buff(cInputBuffer, configCOMMAND_INT_MAX_INPUT_SIZE);
                }
                /* Still saving cli command */
                else
                {
                    if (i_rx_buff < configCOMMAND_INT_MAX_INPUT_SIZE)
                    {
                        cInputBuffer[i_rx_buff++] = u8RxData;
                    }
                    else
                    {
                        vCliPrintf(CLI_TASK_NAME, "Flush input buffer");
                        _clear_buff(cInputBuffer, configCOMMAND_INT_MAX_INPUT_SIZE);
                        i_rx_buff = 0;
                    }

                }
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bCliTaskInit(void)
{
    bool bRetval = false;

    /* Init HW resources */
    (void)SYS_SERIAL_Init(CLI_SERIAL, _event_cb);

    /* Create mutex */
    cli_serial_mutex = xSemaphoreCreateMutex();

    /* Create task */
    xTaskCreate(_cli_main, CLI_TASK_NAME, CLI_TASK_STACK, NULL, CLI_TASK_PRIO, &cli_task_handle);

    /* Check resources */
    if ((cli_serial_mutex != NULL) && (cli_task_handle != NULL))
    {
        bRetval = true;
    }
    else
    {
        ERR_ASSERT(0U);
    }
    

    return bRetval;
}

/* CLI printf implementation */
void vCliPrintf(const char *module_name, const char *Format, ...)
{
    if (cli_serial_mutex != NULL)
    {
        if (xSemaphoreTake(cli_serial_mutex, portMAX_DELAY) != pdTRUE)
        {
            ERR_ASSERT(0U);
        }

        int32_t len_data = 0;
        va_list Args;

        /* Print task header */
        _clear_buff(print_output_buffer, CLI_OUTPUT_BUFFER_SIZE);
        len_data = snprintf(
            (char *)print_output_buffer, 
            CLI_OUTPUT_BUFFER_SIZE, 
            "%s%08x, %s, ", 
            CLI_EOL, 
            (unsigned int)xTaskGetTickCount(), 
            module_name);

        if (len_data > 0)
        {
            ser_tx_done = false;
            SYS_SERIAL_Send(CLI_SERIAL, (uint8_t *)print_output_buffer, len_data);
            while (ser_tx_done != true)
            {
                vTaskDelay(pdMS_TO_TICKS(1U));
            }
        }

        /* Print cli message */
        _clear_buff(print_output_buffer, CLI_OUTPUT_BUFFER_SIZE);
        va_start(Args, Format);
        len_data = vsnprintf((char *)print_output_buffer, CLI_OUTPUT_BUFFER_SIZE, Format, Args);
        va_end(Args);

        if (len_data > 0)
        {
            SYS_SERIAL_Send(CLI_SERIAL, (uint8_t *)print_output_buffer, len_data);
            ser_tx_done = false;
            SYS_SERIAL_Send(CLI_SERIAL, (uint8_t *)print_output_buffer, len_data);
            while (ser_tx_done != true)
            {
                vTaskDelay(pdMS_TO_TICKS(1U));
            }
        }

        if (xSemaphoreGive(cli_serial_mutex) != pdTRUE)
        {
            ERR_ASSERT(0U);
        }
    }
}

/* CLI printf implementation */
void vCliRawPrintf(const char *Format, ...)
{
    if (cli_serial_mutex != NULL)
    {
        if (xSemaphoreTake(cli_serial_mutex, portMAX_DELAY) != pdTRUE)
        {
            ERR_ASSERT(0U);
        }

        int32_t len_data = 0;
        va_list Args;

        /* Print cli message */
        _clear_buff(print_output_buffer, CLI_OUTPUT_BUFFER_SIZE);
        va_start(Args, Format);
        len_data = vsnprintf((char *)print_output_buffer, CLI_OUTPUT_BUFFER_SIZE, Format, Args);
        va_end(Args);

        if (len_data > 0)
        {
            ser_tx_done = false;
            SYS_SERIAL_Send(CLI_SERIAL, (uint8_t *)print_output_buffer, len_data);
            while (ser_tx_done != true);
        }

        if (xSemaphoreGive(cli_serial_mutex) != pdTRUE)
        {
            ERR_ASSERT(0U);
        }
    }
}

bool bCliTaskNotify(uint32_t u32Event)
{
    bool bRetval = false;

    /* Check if task has been init */
    if (cli_task_handle != NULL)
    {
      xTaskNotify(cli_task_handle, u32Event, eSetBits);
      bRetval = true;
    }

    return bRetval;
}

/*****END OF FILE****/
