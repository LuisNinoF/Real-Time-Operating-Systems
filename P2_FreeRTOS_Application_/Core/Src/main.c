/*
 * *** Purpose:
 * Demonstrate a Free RTOS Application, including Tasks, Queues, Semaphores and Notifications
 *
 * *** How it works
 * The application follows these functions
 * 1. ADC Handle interrupt, to read sensor data from the ADC Module 1 when data conversion is ready.
 * 2. Task 1 - Read data, to send sensor data to the next task using a queue.
 * 3. Task 2 - Process data, to calculate the average of 10 sensor measurement values.
 * 4. Task 3 - Take action, to take an action based on the average of the sensor readings. For example, set an output high if the average if above a threshold.
 *
 * The application uses following FreeRTOS objects
 * 1. Notification, to indicate from ADC Handle Interrupt to Task 1 (Read data) that new data has been read.
 * 2. Queue, to send a packet of 10 data from the Task 1 (Read data) to Task 2 (Process data).
 * 3. Mutex, to protect the access to the variable sensor_average, where the average of the 10 data is stored.
 * 4. Semaphore, to communicate from Task 2 (Process data) to Task 3 (Take Action) that the new average has been calculated.
 *
 * *** Copyrights:
 * Created by Luis Nino
 * Last revision 2025.11.05
 */

/* Includes */
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "uart.h"
#include "adc_interrupt.h"
#include "gpio_out.h"

/* Define macros */
#define STACK_SIZE 		256
//#define MAX_BLOCK_TIME	pdMS_TO_TICKS(100)
//#define MAX_WAIT_TIME		pdMS_TO_TICKS(10)
#define	QUEUE_LENGTH	10
#define THRESHOLD		2500

const TickType_t MAX_BLOCK_TIME = pdMS_TO_TICKS(100);
const TickType_t MAX_WAIT_TIME = pdMS_TO_TICKS(50);

/* Private function prototypes */
void SystemClock_Config(void);

/* Declare global variables */
uint32_t sensor_reading;							// this variable stores each sensor reading
uint32_t sensor_average;							// this variable stores each sensor readings average, from 10 sensor readings

/* Declare global functions */
void vTaskReadData(void *pvParameters);
void vTaskProcessData(void *pvParameters);
void vTaskTakeAction(void *pvParameters);

/* Declare Task Handles */
TaskHandle_t xTaskHandleReadData;
TaskHandle_t xTaskHandleProcessData;
TaskHandle_t xTaskHandleTakeAction;

/* Declare Queue Handles */
QueueHandle_t xQueueHandleSensorPacket;				// this Queue contains a packet with 10 sensor readings

/* Declare Semaphores and Mutex Handles */
SemaphoreHandle_t xMutexHandleSensorAverage;		// this Mutex protects access to the variable sensor_average
SemaphoreHandle_t xSemaphoreHandleNewAverageReady;	// this Semaphore communicates that a new sensor average has been calculated

/* Task Profilers, for debugging only*/
typedef uint32_t TaskProfiler;
TaskProfiler 	taskProfilerReadData, taskProfilerReadDataInside,
				taskProfilerProcessData, taskProfilerProcessDataInside,
				taskProfilerTakeAction, taskProfilerTakeActionInside,
				taskProfilerADC_IRQ_Handler, taskProfilerADC_IRQ_Handler2,
				taskProfilerBeforeScheduler;

int __io_putchar(int ch);


/* main function */
int main(void)
{
	/* Reset all peripherals */
	HAL_Init();

	/* Configure system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	adc_interrupt_init();			// ADC at PA1
	GPIO_OUT_init();				// GPIO out at PA5

	/* Create Tasks */
	xTaskCreate(vTaskReadData, 	  "Task read sensor data",   	STACK_SIZE, NULL, 1, &xTaskHandleReadData);
	xTaskCreate(vTaskProcessData, "Task process sensor data",	STACK_SIZE, NULL, 1, &xTaskHandleProcessData);
	xTaskCreate(vTaskTakeAction,  "Task take action with data", STACK_SIZE, NULL, 1, &xTaskHandleTakeAction);

	/* Create Queues */
	xQueueHandleSensorPacket = xQueueCreate(QUEUE_LENGTH, sizeof(int32_t));

	/* Create Semaphores and Mutex */
	xMutexHandleSensorAverage = xSemaphoreCreateMutex();
	xSemaphoreHandleNewAverageReady = xSemaphoreCreateBinary();

	/* Start Scheduler */
	taskProfilerBeforeScheduler++;
	vTaskStartScheduler();

	/* Infinite loop */
	while (1)
	  {
		// EMPTY! (The magic of RTOS)
	  }
}

/*** task functions */

/* task function 1 */
void vTaskReadData(void *pvParameters)
{

	while(1)
	{
		adc_start_conversion();

		taskProfilerReadData++;

		if(ulTaskNotifyTake(pdTRUE, MAX_BLOCK_TIME) != 0)
		{
			xQueueSend(xQueueHandleSensorPacket, &sensor_reading, MAX_WAIT_TIME);
			taskProfilerReadDataInside++;
		}
		vTaskDelay(1);
	}
}

/* task function 2 */
void vTaskProcessData(void *pvParameters)
{
	uint32_t data_received;
	BaseType_t status_received;

	int8_t 	 data_counter = 0;
	uint32_t data_sum = 0;

	while(1)
	{
		status_received = xQueueReceive(xQueueHandleSensorPacket, &data_received, MAX_WAIT_TIME);
		taskProfilerProcessData++;

		if (status_received == pdPASS)
		{
			data_sum += data_received;
			data_counter++;

			if(data_counter == QUEUE_LENGTH)
			{
				if (xSemaphoreTake(xMutexHandleSensorAverage, MAX_WAIT_TIME) == pdTRUE)
				{
					sensor_average = data_sum/data_counter;
					taskProfilerProcessDataInside++;
					xSemaphoreGive(xMutexHandleSensorAverage);
				}

				xSemaphoreGive(xSemaphoreHandleNewAverageReady);

				data_counter = 0;
				data_sum = 0;
			}
		}
	}
}

/* task function 3 */
void vTaskTakeAction(void *pvParameters)
{

	while(1)
	{
		taskProfilerTakeAction++;
		if(xSemaphoreTake(xSemaphoreHandleNewAverageReady, MAX_WAIT_TIME) == pdTRUE)
		{
			if(xSemaphoreTake(xMutexHandleSensorAverage, MAX_WAIT_TIME) == pdTRUE)
			{
				taskProfilerTakeActionInside++;

				if(sensor_average >= THRESHOLD)
				{
					GPIO_OUT_on();
				}
				else
				{
					GPIO_OUT_off();
				}

				xSemaphoreGive(xMutexHandleSensorAverage);
			}
		}
	}
}

/* ADC interrupt handler */
void ADC_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(ADC1->SR & ADC_SR_EOC)
	{
		sensor_reading = ADC1->DR;

		taskProfilerADC_IRQ_Handler++;

		vTaskNotifyGiveFromISR(xTaskHandleReadData, &xHigherPriorityTaskWoken);

		taskProfilerADC_IRQ_Handler2++;

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

	}
}


/* Clock Configuration */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/** This function is executed in case of error occurrence. */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
