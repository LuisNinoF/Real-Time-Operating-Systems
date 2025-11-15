#include  "adc_interrupt.h"
#include  "stm32f4xx_hal.h"

#define GPIOAEN 		(1U<<0)
#define ADC1EN  		(1U<<8)
#define ADC_CH1 		(1U<<0)
#define ADC_SEQ_LEN_1 	 0x00
#define CR2_ADON		(1U<<0)
#define CR2_SWSTART		(1U<<30)
#define CR2_CONT		(1U<<1)
#define CR1_EOCIE		(1U<<5)

void adc_interrupt_init()
{
	/* 1. Configure ADC GPIO pin */

	RCC->AHB1ENR |= GPIOAEN;			// Enable clock for AHB1EN GPIOA

	GPIOA->MODER |= (1U<<2);			// Set GPIO mode to analog mode
	GPIOA->MODER |= (1U<<3);

	/* 2. Configure ADC module */

	RCC->APB2ENR |= ADC1EN;				// Enable clock access to ADC module

	ADC1->CR1 |= CR1_EOCIE;				// Enable ADC end of conversion interrupt (EOCIE)

	NVIC_SetPriority(ADC_IRQn,5);		// Set ADC interrupt request priority
	NVIC_EnableIRQ(ADC_IRQn);			// Enable ADC interrupt in NVIC

	ADC1->SQR3 = ADC_CH1;				// Configure ADC parameters: Conversion sequence start
	ADC1->SQR1 = ADC_SEQ_LEN_1;			// Configure ADC parameters: Conversion sequence length
	ADC1->CR2 |= CR2_ADON;				// Enable ADC module
}

void adc_start_conversion(void)
{

	//ADC1->CR2 |= CR2_CONT;			// Enable continuous conversion (DISABLED)
	ADC1->CR2 |= CR2_SWSTART;			// Start conversion
}
