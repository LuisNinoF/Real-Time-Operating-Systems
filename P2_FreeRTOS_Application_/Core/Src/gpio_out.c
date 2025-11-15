#include "gpio_out.h"

#define GPIOAEN (1U<<0)
#define GPIO_OUT_PIN	(1U<<5)

void GPIO_OUT_init(void)
{
	RCC->AHB1ENR |= GPIOAEN;
	GPIOA->MODER &=~ (1<<11);
	GPIOA->MODER |= (1<<10);
}

void GPIO_OUT_on(void)
{
	GPIOA->ODR |= GPIO_OUT_PIN;
}

void GPIO_OUT_off(void)
{
	GPIOA->ODR &=~ GPIO_OUT_PIN;
}
