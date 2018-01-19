/**
 ******************************************************************************
 * @file    Templates/Src/main.c
 * @author  MCD Application Team
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define REDPORT GPIOB
#define REDPIN GPIO_PIN_0
#define RED CCR3

#define BLUEPORT GPIOA
#define BLUEPIN GPIO_PIN_7
#define BLUE CCR2

#define GREENPORT GPIOA
#define GREENPIN GPIO_PIN_15
#define GREEN CCR1

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/** @addtogroup STM32L4xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef	uartHandle;
TIM_HandleTypeDef	TimHandleR;
TIM_HandleTypeDef	TimHandleB;
TIM_HandleTypeDef	TimHandleG;
TIM_OC_InitTypeDef	sConfigR;
TIM_OC_InitTypeDef	sConfigB;
TIM_OC_InitTypeDef	sConfigG;
GPIO_InitTypeDef	LEDRED;            // create a config structure
GPIO_InitTypeDef	LEDBLUE;
GPIO_InitTypeDef	LEDGREEN;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/
static void Peripherals_Init(void);
static void UART_Init(void);
static void LED_Init_RED(void);
static void LED_Init_BLUE(void);
static void LED_Init_GREEN(void);

static void TIMER_Init_RED(void);
static void PWM_Init_RED(void);

static void TIMER_Init_BLUE(void);
static void PWM_Init_BLUE(void);

static void TIMER_Init_GREEN(void);
static void PWM_Init_GREEN(void);

static void LED_ON (int _red, int _blue, int _green);

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {

	Peripherals_Init();

	int red = 0x80;
	int blue = 0x80;
	int green = 0x80;

	while(1){
		printf("TIM3R: %d    TIM3B: %d   TIM2G: %d \n\n" , TIM3->CNT , TIM3->CNT , TIM2->CNT);
		HAL_Delay(500);

		BSP_LED_Toggle(LED_GREEN);
		HAL_Delay(500);

			LED_ON(red , blue , green);

		/*
		TIM3->RED = 0;
		TIM3->BLUE = 0;
		TIM2->GREEN = 0;

		HAL_Delay(500);
		TIM3->RED = 1500;
		TIM3->BLUE = 1500;
		TIM2->GREEN = 1500;
		HAL_Delay(500);
		TIM3->RED = 500;
		TIM3->BLUE = 500;
		TIM2->GREEN = 500;

*/
	}
}

static void LED_ON (int _red, int _blue, int _green){

	TIM3->RED = _red;
	TIM3->BLUE = _blue;
	TIM2->GREEN = _green;

}

static void Peripherals_Init(void) {
	/* STM32L4xx HAL library initialization:
	 - Configure the Flash prefetch, Flash preread and Buffer caches
	 - Systick timer is configured by default as source of time base, but user
	 can eventually implement his proper time base source (a general purpose
	 timer for example or other time source), keeping in mind that Time base
	 duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
	 handled in milliseconds basis.
	 - Low Level Initialization
	 */
	HAL_Init();

	/* Configure the System clock to have a frequency of 80 MHz */
	SystemClock_Config();

	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
	BSP_LED_Init(LED_GREEN);

	UART_Init();

	TIMER_Init_RED();
	PWM_Init_RED();
	TIMER_Init_BLUE();
	PWM_Init_BLUE();
	TIMER_Init_GREEN();
	PWM_Init_GREEN();

	LED_Init_RED();
	LED_Init_BLUE();
	LED_Init_GREEN();

}

static void UART_Init(void) {
	uartHandle.Init.BaudRate = 115200;
	uartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	uartHandle.Init.StopBits = UART_STOPBITS_1;
	uartHandle.Init.Parity = UART_PARITY_NONE;
	uartHandle.Init.Mode = UART_MODE_TX_RX;

	BSP_COM_Init(COM1, &uartHandle);
}

static void PWM_Init_RED(void) {
	HAL_TIM_PWM_Init(&TimHandleR);

	sConfigR.OCMode = TIM_OCMODE_PWM1;
	sConfigR.Pulse = 257;
	HAL_TIM_PWM_ConfigChannel(&TimHandleR , &sConfigR , TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&TimHandleR , TIM_CHANNEL_3);
}


static void PWM_Init_BLUE(void) {
	HAL_TIM_PWM_Init(&TimHandleB);

	sConfigB.OCMode = TIM_OCMODE_PWM1;
	sConfigB.Pulse = 257;
	HAL_TIM_PWM_ConfigChannel(&TimHandleB , &sConfigB , TIM_CHANNEL_2);

	HAL_TIM_PWM_Start(&TimHandleB , TIM_CHANNEL_2);
}

static void PWM_Init_GREEN(void) {
	HAL_TIM_PWM_Init(&TimHandleG);

	sConfigG.OCMode = TIM_OCMODE_PWM1;
	sConfigG.Pulse = 257;
	HAL_TIM_PWM_ConfigChannel(&TimHandleG , &sConfigG , TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&TimHandleG , TIM_CHANNEL_1);
}


static void TIMER_Init_RED(void) {
	__HAL_RCC_TIM3_CLK_ENABLE();

	TimHandleR.Instance               = TIM3;
	TimHandleR.Init.Period            = 255; //16bit number max value 0xFFFF
	TimHandleR.Init.Prescaler         = 0;
	TimHandleR.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	TimHandleR.Init.CounterMode       = TIM_COUNTERMODE_UP;

	HAL_TIM_Base_Init(&TimHandleR); //Configure the timer

	//HAL_TIM_Base_Start(&TimHandle);

}

static void TIMER_Init_BLUE(void) {
	//__HAL_RCC_TIM3_CLK_ENABLE();

	TimHandleB.Instance               = TIM3;
	TimHandleB.Init.Period            = 255; //16bit number max value 0xFFFF
	TimHandleB.Init.Prescaler         = 0;
	TimHandleB.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	TimHandleB.Init.CounterMode       = TIM_COUNTERMODE_UP;

	HAL_TIM_Base_Init(&TimHandleB); //Configure the timer

	//HAL_TIM_Base_Start(&TimHandleB);
}

static void TIMER_Init_GREEN(void) {
	__HAL_RCC_TIM2_CLK_ENABLE();

	TimHandleG.Instance               = TIM2;
	TimHandleG.Init.Period            = 255; //16bit number max value 0xFFFF
	TimHandleG.Init.Prescaler         = 0;
	TimHandleG.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	TimHandleG.Init.CounterMode       = TIM_COUNTERMODE_UP;

	HAL_TIM_Base_Init(&TimHandleG); //Configure the timer

	//HAL_TIM_Base_Start(&TimHandleG);
}

static void LED_Init_RED(void) {

	__HAL_RCC_GPIOB_CLK_ENABLE();    // we need to enable the GPIO* port's clock first

	LEDRED.Pin = REDPIN;            // this is about PIN 1
	LEDRED.Mode = GPIO_MODE_AF_OD; // Configure as output with push-up-down enabled
	LEDRED.Pull = GPIO_NOPULL;      // the push-up-down should work as pulldown
	LEDRED.Speed = GPIO_SPEED_HIGH;     // we need a high-speed output
	LEDRED.Alternate = GPIO_AF2_TIM3;   //Alterante function to set PWM timer

	HAL_GPIO_Init(REDPORT, &LEDRED);   // initialize the pin on GPIO* port with HAL
}

static void LED_Init_BLUE(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();    // we need to enable the GPIO* port's clock first

	LEDBLUE.Pin = BLUEPIN;            // this is about PIN 1
	LEDBLUE.Mode = GPIO_MODE_AF_OD; // Configure as output with push-up-down enabled
	LEDBLUE.Pull = GPIO_NOPULL;      // the push-up-down should work as pulldown
	LEDBLUE.Speed = GPIO_SPEED_HIGH;     // we need a high-speed output
	LEDBLUE.Alternate = GPIO_AF2_TIM3;   //Alterante function to set PWM timer

	HAL_GPIO_Init(BLUEPORT, &LEDBLUE);   // initialize the pin on GPIO* port with HAL
}
static void LED_Init_GREEN(void) {
	//__HAL_RCC_GPIOA_CLK_ENABLE();    // we need to enable the GPIO* port's clock first

	LEDGREEN.Pin = GREENPIN;            // this is about PIN 1
	LEDGREEN.Mode = GPIO_MODE_AF_OD; // Configure as output with push-up-down enabled
	LEDGREEN.Pull = GPIO_NOPULL;      // the push-up-down should work as pulldown
	LEDGREEN.Speed = GPIO_SPEED_HIGH;     // we need a high-speed output
	LEDGREEN.Alternate = GPIO_AF1_TIM2;   //Alterante function to set PWM timer

	HAL_GPIO_Init(GREENPORT, &LEDGREEN);   // initialize the pin on GPIO* port with HAL
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (MSI)
 *            SYSCLK(Hz)                     = 80000000
 *            HCLK(Hz)                       = 80000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            APB2 Prescaler                 = 1
 *            MSI Frequency(Hz)              = 4000000
 *            PLL_M                          = 1
 *            PLL_N                          = 40
 *            PLL_R                          = 2
 *            PLL_P                          = 7
 *            PLL_Q                          = 4
 *            Flash Latency(WS)              = 4
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* MSI is enabled after System reset, activate PLL with MSI as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 40;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLP = 7;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		/* Initialization Error */
		while (1)
			;
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		/* Initialization Error */
		while (1)
			;
	}
}

PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
	HAL_UART_Transmit(&uartHandle, (uint8_t *) &ch, 1, 0xFFFF);

	return ch;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
