/*
 * Purpose of this program:
 * Create osKernel with osScheduler for task scheduling and context switching
 *
 * Application:
 * Task 0 - Read sensor data: Continuously monitor the water level of a tank with a level sensor via ADC.
 * Task 1 - Process sensor data: adjust sensor to real life meaning with characteristic curve, which is the water level in a tank.
 * Task 2 - Control pump: if the water level is above a threshold, turn on the pump. Otherwise, turn it off. Also, notify via UART every time the pump status changes between on/off.
 *
 * Created by: Luis Nino
 * Last revision: 2025.10.30
 *
 */

// Include drivers
#include <stdint.h>
#include "stm32f4xx.h"
#include "uart.h"
#include "adc1.h"
#include "gpio_out.h"
#include "osKernel.h"

// Declare Scheduling and Context Switching Parameters
#define QUANTA 10

// Declare prototype functions for the threads
void task0_read_sensor_data(void);			// function to read data from the real world
void task1_process_sensor_data(void);		// function to process data
void task2_control_pump(void);				// function to take action with the data

// Declare global variables
typedef uint32_t Act_Task;
Act_Task Act_Task0, Act_Task1, Act_Task2;	// Task profilers
uint32_t level_sensor_signal;				// Sensor signal from 0 to 4096, because of 12 bits adc conversion (2^12 = 4096)
uint32_t water_level_in_tank;				// Water level in tanks scaled from 0 to 1500 mm
uint32_t pump_status = 0;					// Pump status, 0 = off, 1 = on
const uint32_t MIN_WATER_LEVEL = 300;		// 300 mm


// RUN MAIN
int main(void)
{
	// Initialize drivers
	uart2_tx_init();			// UART at PA2 (same as USB connector in Nucleo board) with baudrate 115200
	pa1_adc_init();				// ADC at PA1
	GPIO_OUT_init();			// GPIO out at PA5

	// 1. Initialize Kernel
	osKernelInit();

	// 2. Add threads
	osKernelAddThreads(&task0_read_sensor_data, &task1_process_sensor_data, &task2_control_pump);

	// 3. Set Round Robin time quanta
	osKernelLaunch(QUANTA);

	while(1)
	{
		// EMPTY! (The magic of the osKernel)
	}
}


// Define the functions for the threads
void task0_read_sensor_data(void)
{
	while(1)
	{
		Act_Task0++;
		level_sensor_signal = adc_read();							// Read data from sensor
		osThreadYield();											// Exit thread once read to save resources
	}
}

void task1_process_sensor_data(void)
{
	while(1)
	{
		Act_Task1++;
		water_level_in_tank = (level_sensor_signal*1500)/4096;		// Scale sensor signal to water level (max 1.5 meters = 1500 mm)
		osThreadYield();											// Exit thread once data processed to save resources
	}
}

void task2_control_pump(void)
{
	while(1)
	{
		Act_Task2++;
		if(water_level_in_tank > MIN_WATER_LEVEL)					// Check condition of water level
		{
			GPIO_OUT_on();											// Turn pump on if level > MIN level
			if(pump_status == 0)
			{
				printf("Pump status changed: turned on \n\r");		// Notify via UART if pump status changed
			}
			pump_status = 1;										// Update status
		}
		else
		{
			GPIO_OUT_off();											// Turn pump of if level <= MIN level
			if(pump_status == 1)
				{
					printf("Pump status changed: turned off \n\r");	// Notify via UART if pump status changed
				}
			pump_status = 0;										// Update status
		}
		osThreadYield();											// Exit thread once the pump is controlled to save resources
	}
}
