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
#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/logging.h"
#include "wifi.h"
#include "net_transport.h"
#include "net_secure.h"
#include "google_iot.h"
#include "ntp_client.h"
#include "rtc_utils.h"


// 0 = MQTT example, 1 = TLS example
#define EXAMPLE_KIND	EXAMPLE_MQTT
#define NEED_WIFI		1

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

typedef struct WolfSocketContext {
	uint32_t id;
} WolfSocketContext;

uint32_t socketId = 0;

typedef enum ExampleKind {
	EXAMPLE_MQTT, EXAMPLE_TLS
} ExampleKind;

/* Private define ------------------------------------------------------------*/
#define SSID     "Cethal"
#define PASSWORD "ideahelysegben"
//#define SSID     "A66 Guest"
//#define PASSWORD "Hello123"
//#define SSID     "AndroidAP"
//#define PASSWORD "Buzi3vagy"
//#define SSID		"Doctusoft"
//#define PASSWORD	"KTvEqa4bz2"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef uartHandle;
RNG_HandleTypeDef rngHandle;
GGL_InitDef gglConfig;
NetSecure_InitTypeDef secConfig;

uint8_t MAC_Addr[6];
uint8_t IP_Addr[4];

/**
 * MQTT configuration
 */
#define DEFAULT_TIMEOUT			1000
#define MAX_BUFFER_SIZE         512

WOLFSSL *ssl;
WOLFSSL_CTX *ctx;

WolfSocketContext mqttContext = { 0 };
NetTransportContext netContext = { 0 };

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/
static void Peripherals_Init(void);
static void SW_STACK_Init(void);
static void UART_Init(void);
static void RNG_Init(void);
static void WIFI_GoOnline(void);

int MQTT_MessageArrivedCallback(const char* topic, const char* message) {
	printf("Message arrived in topic: %s\r\nMessage:%s", topic, message);
	return 0;
}

void wolfSSL_Logging_cb_f(const int logLevel, const char * const logMessage) {
	printf("ssl > [%d] - %s\r\n", logLevel, logMessage);
}

static void Wolfmqtt_PublishReceive(const char *host, int port) {

	GGL_MQTT_Connect();
	GGL_MQTT_Publish("events/report", "{\"state\": \"off\"}");

	GGL_MQTT_Subscribe("state");

	while (1) {
		GGL_MQTT_Ping();
		HAL_Delay(10000);
	}

	GGL_MQTT_Disconnect();
}

static int Wolfssl_TlsConnect(const char *host, int port) {

	if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
		return -1;
	}

	//wolfSSL_SetIORecv(ctx, WolfsslReadCallback);
	//wolfSSL_SetIOSend(ctx, WolfsslWriteCallback);

	if ((ssl = wolfSSL_new(ctx)) == NULL) {
		return -3;
	}

	wolfSSL_SetIOReadCtx(ssl, &mqttContext);
	wolfSSL_SetIOWriteCtx(ssl, &mqttContext);

	uint8_t destIp[4];
	if (WIFI_GetHostAddress((char*) host, destIp) != WIFI_STATUS_OK) {
		printf("FAIL DNS\r\n");
		return -4;
	}

	if (WIFI_OpenClientConnection(mqttContext.id, WIFI_TCP_PROTOCOL,
			"TCP_CLIENT", destIp, port, DEFAULT_TIMEOUT) != WIFI_STATUS_OK) {
		return -5;
	}

	int resCode = wolfSSL_connect(ssl);
	printf("wolfSSL_connect() - RS: %d\r\n", resCode);
	if (resCode != SSL_SUCCESS) {
		return -6;
	}

	wolfSSL_free(ssl); /* Free the wolfSSL object                  */
	wolfSSL_CTX_free(ctx); /* Free the wolfSSL context object          */
	wolfSSL_Cleanup(); /* Cleanup the wolfSSL environment          */
	WIFI_CloseClientConnection(mqttContext.id); /* Close the connection to the server       */

	return 0;
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
	Peripherals_Init();

	SW_STACK_Init();

	WIFI_GoOnline();

	NTPClient_Init("hu.pool.ntp.org", 123);
	uint32_t ntpTimestamp;
	int rc;
	if ((rc = NTPClient_GetTimeSeconds(&ntpTimestamp)) != 0) {
		printf("ERROR: could not get NTP timestamp\r\n");
	}

	printf("NTP timestamp is: %lu\r\n", ntpTimestamp);
	RTCUtils_SetEpochTimestamp(ntpTimestamp);

	int res;
	switch (EXAMPLE_KIND) {
	case EXAMPLE_TLS:
		res = Wolfssl_TlsConnect("mqtt.googleapis.com", 8883);
		printf("---- TLS RESULT: %d ----\r\n", res);
		break;
	case EXAMPLE_MQTT:
		Wolfmqtt_PublishReceive("mqtt.googleapis.com", 8883);
		break;
	}

}

static void WIFI_GoOnline(void) {
	if (WIFI_Connect(SSID, PASSWORD, WIFI_ECN_WPA2_PSK) != WIFI_STATUS_OK) {
		printf(
				"ERROR: Couldn't connect to WiFi network %s with password %s\r\n",
				SSID, PASSWORD);
		while (1) {
		}
	}

	if (WIFI_GetIP_Address(IP_Addr) == WIFI_STATUS_OK) {
		printf("WiFi successfully joined with IP address: %d.%d.%d.%d\r\n",
				IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3]);
	} else {
		printf(
				"ERROR: Couldn't get IP address on network %s with password %s\r\n",
				SSID, PASSWORD);
		while (1) {
		}
	}
}

static void SW_STACK_Init() {
	// enable debug messages
	SHOME_DebugEnable();

	// initialize network layer (WiFi in our case)
	net_Init(&netContext);

	secConfig.rngHandle = &rngHandle;
	net_SecureInit(&secConfig);

	// initialize security layer (WolfSSL)
	wolfSSL_Init();
	wolfSSL_Debugging_ON();
	wolfSSL_SetLoggingCb(wolfSSL_Logging_cb_f);
	wolfSSL_set_verify(ssl, WOLFSSL_VERIFY_NONE, NULL);
	wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);

	// initialize google stack
	GGL_DeviceDef device;
	device.deviceId = "test-iot-device-2";
	device.deviceRegistry = "greenfox-device-registry";
	device.projectId = "static-aventurin-fuchsit";
	device.region = "europe-west1";

	GGL_NetworkDef network;
	network.mqttHost = "mqtt.googleapis.com";
	network.mqttPort = 8883;

	gglConfig.callback = MQTT_MessageArrivedCallback;
	gglConfig.device = device;
	gglConfig.network = network;
	GGL_IOT_Init(&gglConfig);
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

	UART_Init();

	RNG_Init();

	BSP_LED_Init(LED_GREEN);

	// initialize real time clock peripheral
	RTCUtils_RTCInit();
}

static void RNG_Init(void) {

	__HAL_RCC_RNG_CLK_ENABLE()
	;

	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/*Select PLLQ output as RNG clock source */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
	PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	rngHandle.Instance = RNG;

	if (HAL_RNG_Init(&rngHandle) != HAL_OK) {
		printf("ERROR: could not configure RNG");
	}
}

static void UART_Init(void) {
	uartHandle.Init.BaudRate = 115200;
	uartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	uartHandle.Init.StopBits = UART_STOPBITS_1;
	uartHandle.Init.Parity = UART_PARITY_NONE;
	uartHandle.Init.Mode = UART_MODE_TX_RX;

	BSP_COM_Init(COM1, &uartHandle);
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
