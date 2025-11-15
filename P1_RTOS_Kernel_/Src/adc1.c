#include "adc1.h"

#include "stm32f4xx.h"

#define GPIOAEN 		(1U<<0)
#define ADC1EN  		(1U<<8)
#define ADC_CH1 		(1U<<0)
#define ADC_SEQ_LEN_1 	 0x00
#define CR2_ADON		(1U<<0)
#define CR2_SWSTART		(1U<<30)
#define SR_EOC			(1U<<1)
#define CR2_CONT		(1U<<1)

void pa1_adc_init()
{
	/* 1. Configure ADC GPIO pin
	 * 1.1 Enable clock access to adc pin port GPIOA (because PA1 is in port A)
	 * 1.2. Set the mode of PA1 to analog
	 *
	 */

	// 1.1 Enable clock access to adc pin port GPIOA (because PA1 is in port A)
	RCC->AHB1ENR |= GPIOAEN;

	// 1.2. Set the mode of PA1 to analog
	GPIOA->MODER |= (1U<<2);
	GPIOA->MODER |= (1U<<3);


	/* 2. Configure ADC module
	 * 2.1 Enable clock access to ADC module
	 * 2.2 Configure ADC parameters
	 * 2.2.1 conversion sequence start (start at channel 1 in our case)
	 * 2.2.2 conversion sequence length
	 * 2.2.3 Enable ADC module
	 */

	// 2.1 Enable clock access to ADC module
	RCC->APB2ENR |= ADC1EN;

	// 2.2 Configure ADC parameters
	ADC1->SQR3 = ADC_CH1;
	ADC1->SQR1 = ADC_SEQ_LEN_1;
	ADC1->CR2 |= CR2_ADON;

	start_conversion();
}

void start_conversion(void)
{
	// Enable continuous conversion
	ADC1->CR2 |= CR2_CONT;

	// Start conversion
	ADC1->CR2 |= CR2_SWSTART;
}

uint32_t adc_read(void)
{
	// Wait for conversion to be compete, so while not completed we get stuck here
	while(!(ADC1->SR & SR_EOC)){}

	// Read converted result
	return (ADC1->DR);
}
