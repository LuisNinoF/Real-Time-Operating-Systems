#include <stdint.h>
#include "uart.h"
#include "stm32f4xx.h"

#define GPIOAEN (1U<<0)
#define UART2EN (1U<<17)

#define CR1_TE 	(1U<<3)
#define CR1_UE 	(1U<<13)
#define SR_TXE 	(1U<<7)

#define SYS_FREQ		16000000
#define APB1_CLK		SYS_FREQ
#define UART_BAUDRATE	115200

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PriphClk, uint32_t BaudRate);
static void uart2_write(int ch);

// re-target printf
int __io_putchar(int ch)
{
	uart2_write(ch);
	return ch;
}

void uart2_tx_init(void)
{
	/* 1. Configure UART GPIO PIN
	 * 1.1 Enable clock access to GPIOA (because PA2 is in Port A)
	 * 1.2 Set PA2 mode to alternate function mode
	 * 1.3 Set PA2 alternate function type to UART_TX
	 */

	// 1.1 Enable clock access to GPIOA
	RCC->AHB1ENR |= GPIOAEN;

	// 1.2 Set PA2 mode to alternate function mode
	GPIOA->MODER &=~ (1U<<4);
	GPIOA->MODER |=  (1U<<5);

	// 1.3 Set PA2 alternate function type to UART_TX
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &=~ (1U<<11);

	/* 2. Configure UART module
	 * 2.1 Enable clock access to UART2
	 * 2.2 Configure baudrate
	 * 2.3 Configure the transfer direction
	 * 2.4 Enable UART module
	 */

	// 2.1 Enable clock access to UART2
	RCC->APB1ENR |= UART2EN;

	// 2.2 Configure baudrate, by computing the value of a function
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDRATE);

	// 2.3 Configure the transfer direction
	USART2->CR1 = CR1_TE;

	// 2.4 Enable UART module
	USART2->CR1 |= CR1_UE;

}

// function to write UART2
static void uart2_write(int ch)
{
	// Make sure the transmit data register is empty
	while(!(USART2->SR & SR_TXE)){}

	// Write to transmit data register (DR)
	USART2->DR = (ch & 0xFF);

}

// auxiliary function to set baudrate
static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t PriphClk, uint32_t BaudRate)
{
	USARTx->BRR = compute_uart_bd(PriphClk, BaudRate);
}

// auxiliary function to compute baudrate
static uint16_t compute_uart_bd(uint32_t PriphClk, uint32_t BaudRate)
{
	return ((PriphClk + (BaudRate/2U))/BaudRate);
}
