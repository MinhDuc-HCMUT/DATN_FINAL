/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "KeyPad.h"
#include "CLCD_I2C.h"
#include "rc522.h"
#include "flash.h"
#include "fingerprint.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define StartAddressUID 0x0800F000
#define Delaymenu 20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
int8_t Rx_Buffer[128];
char Tx_Buffer[6];
CLCD_I2C_Name LCD1;
uint8_t CardID[MFRC522_MAX_LEN];
uint8_t exitmenu = 255;
uint32_t AddressUID = StartAddressUID;

uint32_t time_cho;
char mess[10];
extern uint8_t pID;
int tmp;
uint8_t ID=0;
uint8_t fingerprint_detected = 0; // Biến c�? để đánh dấu trạng thái vân tay
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void RFID(void);
void FACEID(void);
void FINGER(void);
void PASSWORD(void);
uint8_t CheckUID(uint8_t *data, uint32_t address);
uint8_t CheckListUID(uint8_t *data);
uint8_t checkcountUID(void);
void adduid(uint8_t key);
void checkthe(void);
uint32_t CheckKey(uint8_t key);
void removeuid(uint32_t addressrm);
void startadd(void);
void setaddress(void);
void remoall(void);
void resetflash(void);
uint8_t checkfaceid(uint8_t key);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(exitmenu > 0)
		exitmenu --;
	else exitmenu = 0;
}
void check_fingerprint_status()
{
    uint8_t status = collect_finger(); // Hàm kiểm tra trạng thái vân tay
    if (status == 0x00) // Nếu phát hiện vân tay
    {
        fingerprint_detected = 1; // �?ặt c�? báo hiệu
    }
}
void process_fingerprint()
{
    if (fingerprint_detected) // Nếu có vân tay
    {
        fingerprint_detected = 0; // Xóa c�?
        read_finger(); // G�?i hàm xử lý vân tay
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  KeyPad_Init();
  CLCD_I2C_Init(&LCD1, &hi2c2, 0x4E, 16, 2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_TIM_Base_Start_IT(&htim2);
	if (checkcountUID() == 0)
	{
		startadd();
	}
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      CLCD_I2C_SetCursor(&LCD1, 0, 0);
      CLCD_I2C_WriteString(&LCD1, " SCAN YOUR CARD");

      char selected_key = KeyPad_WaitForKeyGetChar(10); // Ch�? vô hạn cho đến khi có phím nhấn.

      if (selected_key != 0)
      {
          uint8_t key = 0; // Key quản lý quy�?n truy cập.
          exitmenu = 15;

          switch (selected_key)
          {
          case 'A': // RFID
          case 'B': // FACEID
          case 'C': // FINGER
          case 'D': // PASSWORD
              CLCD_I2C_SetCursor(&LCD1, 0, 1);
              CLCD_I2C_WriteString(&LCD1, "   ADMIN CARD");

              while (exitmenu)
              {
                  if (TM_MFRC522_Check(CardID) == MI_OK)
                  {
                      key = CheckListUID(CardID);
                      key = key >> 4;
                      break;
                  }
              }

              switch (key)
              {
              case 1:
                  if (selected_key == 'A')
                      RFID();
                  else if (selected_key == 'B')
                      FACEID();
                  else if (selected_key == 'C')
                      FINGER();
                  else if (selected_key == 'D')
                      PASSWORD();
                  break;
              default:
                  CLCD_I2C_Clear(&LCD1);
                  CLCD_I2C_SetCursor(&LCD1, 0, 0);
                  CLCD_I2C_WriteString(&LCD1, "NOT ACCESSIBLE");
                  HAL_Delay(2000);
                  CLCD_I2C_Clear(&LCD1);
                  break;
              }
              break;

          default:
              break;
          }
      }
      else if (TM_MFRC522_Check(CardID) == MI_OK)
      {
          if (CheckListUID(CardID) != 0)
          {
              CLCD_I2C_Clear(&LCD1);
              CLCD_I2C_SetCursor(&LCD1, 0, 0);
              CLCD_I2C_WriteString(&LCD1, "    WELCOME");
              HAL_Delay(500);
          }
          else
          {
              CLCD_I2C_Clear(&LCD1);
              CLCD_I2C_SetCursor(&LCD1, 0, 0);
              CLCD_I2C_WriteString(&LCD1, "   WRONG CARD");
              HAL_Delay(3000);
          }
      }

      // Kiểm tra trạng thái vân tay định kỳ
      check_fingerprint_status();

      // Xử lý vân tay nếu phát hiện
      process_fingerprint();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 35999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CS_Pin|buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, R1_Pin|R2_Pin|R3_Pin|R4_Pin
                          |chotkhoa_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : C4_Pin C3_Pin */
  GPIO_InitStruct.Pin = C4_Pin|C3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : C2_Pin C1_Pin */
  GPIO_InitStruct.Pin = C2_Pin|C1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin buzzer_Pin */
  GPIO_InitStruct.Pin = CS_Pin|buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : R1_Pin R2_Pin R3_Pin R4_Pin */
  GPIO_InitStruct.Pin = R1_Pin|R2_Pin|R3_Pin|R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : chotkhoa_Pin */
  GPIO_InitStruct.Pin = chotkhoa_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(chotkhoa_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
void RFID(void)
{
	exitmenu = 15;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"  SELECT MENU","PLS PRESS DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed =='*')
		{
			exitmenu = 15;
			status++;
			status = (status > 3) ? 0 : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"=>  ADD CARD","    REMOVE CARD");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"    ADD CARD","=>  REMOVE CARD");
				break;
			case 2:
				CLCD_I2C_Display(&LCD1,"    REMOVE CARD","=>  CHECK CARD");
				break;
			default:
				CLCD_I2C_Display(&LCD1,"    CHECK CARD","=>  BACK");
				break;
			}
		}
		if (key_pressed =='#')
		{
			exitmenu = 15;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"   RFID__CARD   ","PLS PRESS DOWN");
				uint8_t statusadd = 0;
				uint8_t back = 1;
				while (back == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed =='*')
					{
						exitmenu = 15;
						statusadd++;
						statusadd = (statusadd > 2) ? 0 : statusadd;
						switch (statusadd)
						{
						case 1:
							CLCD_I2C_Display(&LCD1,"=> ADMIN CARD","   USER CARD");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"   ADMIN CARD","=> USER CARD");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"   USER CARD","=> BACK");
							break;
						}
					}
					if (key_pressed =='#')
					{
						exitmenu = 15;
						switch (statusadd)
						{
						case 1:
							CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
							uint8_t statusadd1 = 1;
							uint8_t back11 = 1;
							while (back11 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed =='*')
								{
									exitmenu = 15;
									statusadd1++;
									statusadd1 = (statusadd1 > 3) ? 0 : statusadd1;
									switch (statusadd1)
									{
									case 1:
										CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
										break;
									case 2:
										CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
										break;
									case 3:
										CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
										break;
									default:
										CLCD_I2C_Display(&LCD1,"    ADMIN CARD 3 ","=>  BACK");
										break;
									}
								}
								if (key_pressed =='#')
								{
									exitmenu = 15;
									uint8_t keyadd1 = (statusadd << 4) + statusadd1;
									switch (statusadd1)
									{
									case 1:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1,"ADMIN 1","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
										}
										break;
									case 2:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1,"ADMIN 2","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
										}
										break;
									case 3:
										if (CheckKey(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1,"ADMIN 3","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
										}
										else
										{
											adduid(keyadd1);
											CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
										}
										break;
									default:
										back11 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1,"=> ADMIN CARD","   USER CARD");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
							uint8_t statusadd2 = 1;
							uint8_t back12 = 1;
							while (back12 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed =='*')
								{
									exitmenu = 15;
									statusadd2++;
									statusadd2 = (statusadd2 > 3) ? 0 : statusadd2;
									switch (statusadd2)
									{
									case 1:
										CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
										break;
									case 2:
										CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
										break;
									case 3:
										CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
										break;
									default:
										CLCD_I2C_Display(&LCD1,"    USER CARD 3 ","=>  BACK");
										break;
									}
								}
								if (key_pressed =='#')
								{
									exitmenu = 15;
									uint8_t keyadd2 = (statusadd << 4) + statusadd2;
									switch (statusadd2)
									{
									case 1:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1,"USER 1","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
										}
										break;
									case 2:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1,"USER 2","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
										}
										break;
									case 3:
										if (CheckKey(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1,"USER 3","AVAILABLE");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
										}
										else
										{
											adduid(keyadd2);
											CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
										}
										break;
									default:
										back12 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1,"   ADMIN CARD","=> USER CARD");
							break;
						default:
							back = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1,"=>  ADD CARD","    REMOVE CARD");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"   RFID__CARD   ","PLS PRESS DOWN");
				uint8_t statusremove = -1;
				uint8_t backrm = 1;
				while (backrm == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed =='*')
					{
						exitmenu = 15;
						statusremove++;
						statusremove = (statusremove > 2) ? 0 : statusremove;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"=> REMOVE CARD","   REMOVE ALL");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"   REMOVE CARD","=> REMOVE ALL");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"   REMOVE ALL","=> BACK");
							break;
						}
					}
					if (key_pressed =='#')
					{
						exitmenu = 15;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"=> SELECT CARD","   SCAN CARD");
							uint8_t statusrm1 = 0;
							uint8_t backrm1 = 1;
							while (backrm1 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed =='*')
								{
									statusrm1++;
									statusrm1 = (statusrm1 > 2) ? 0 : statusrm1;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1,"=> SELECT CARD","   SCAN CARD");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1,"   SELECT CARD","=> SCAN CARD");
										break;
									default:
										CLCD_I2C_Display(&LCD1,"   SCAN CARD","=> BACK");
										break;
									}
								}
								if (key_pressed =='#')
								{
									exitmenu = 15;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1,"=> ADMIN CARD","   USER CARD");
										uint8_t statusadd = 1;
										uint8_t backrm10 = 1;
										while (backrm10 == 1)
										{
											key_pressed = KeyPad_WaitForKeyGetChar(10);
											if (exitmenu == 0)
											{
												CLCD_I2C_Clear(&LCD1);
												HAL_Delay(1000);
												return;
											}
											if (key_pressed =='*')
											{
												exitmenu = 15;
												statusadd++;
												statusadd = (statusadd > 2) ? 0 : statusadd;
												switch (statusadd)
												{
												case 1:
													CLCD_I2C_Display(&LCD1,"=> ADMIN CARD","   USER CARD");
													break;
												case 2:
													CLCD_I2C_Display(&LCD1,"   ADMIN CARD","=> USER CARD");
													break;
												default:
													CLCD_I2C_Display(&LCD1,"   USER CARD","=> BACK");
													break;
												}
											}
											if (key_pressed =='#')
											{
												exitmenu = 15;
												switch (statusadd)
												{
												case 1:
													CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
													uint8_t statusadd1 = 1;
													uint8_t back11 = 1;
													while (back11 == 1)
													{
														key_pressed = KeyPad_WaitForKeyGetChar(10);
														if (exitmenu == 0)
														{
															CLCD_I2C_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (key_pressed =='*')
														{
															exitmenu = 15;
															statusadd1++;
															statusadd1 = (statusadd1 > 3) ? 0 : statusadd1;
															switch (statusadd1)
															{
															case 1:
																CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
																break;
															case 2:
																CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
																break;
															case 3:
																CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
																break;
															default:
																CLCD_I2C_Display(&LCD1,"    ADMIN CARD 3 ","=>  BACK");
																break;
															}
														}
														if (key_pressed =='#')
														{
															exitmenu = 15;
															uint8_t keyadd1 = (statusadd << 4) + statusadd1;
															switch (statusadd1)
															{
															case 1:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO ADMIN CARD 1 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 1 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 1 ","    ADMIN CARD 2 ");
																	}
																}
																break;
															case 2:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO ADMIN CARD 2 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 2 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 2 ","    ADMIN CARD 3 ");
																	}
																}
																break;
															case 3:
																if (CheckKey(keyadd1) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO ADMIN CARD 3 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
																}
																else
																{
																	removeuid(CheckKey(keyadd1));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 3 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1,"=>  ADMIN CARD 3 ","    BACK ");
																	}
																}
																break;
															default:
																back11 = 0;
																break;
															}
														}
													}
													CLCD_I2C_Display(&LCD1,"=> ADMIN CARD","   USER CARD");
													break;
												case 2:
													CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
													uint8_t statusadd2 = 1;
													uint8_t back12 = 1;
													while (back12 == 1)
													{
														key_pressed = KeyPad_WaitForKeyGetChar(10);
														if (exitmenu == 0)
														{
															CLCD_I2C_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (key_pressed =='*')
														{
															exitmenu = 15;
															statusadd2++;
															statusadd2 = (statusadd2 > 3) ? 0 : statusadd2;
															switch (statusadd2)
															{
															case 1:
																CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
																break;
															case 2:
																CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
																break;
															case 3:
																CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
																break;
															default:
																CLCD_I2C_Display(&LCD1,"    USER CARD 3 ","=>  BACK");
																break;
															}
														}
														if (key_pressed =='#')
														{
															exitmenu = 15;
															uint8_t keyadd2 = (statusadd << 4) + statusadd2;
															switch (statusadd2)
															{
															case 1:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO USER CARD 1 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 1 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 1 ","    USER CARD 2 ");
																}
																break;
															case 2:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO USER CARD 2 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 2 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 2 ","    USER CARD 3 ");
																}
																break;
															case 3:
																if (CheckKey(keyadd2) == 0)
																{
																	CLCD_I2C_Clear(&LCD1);
																	CLCD_I2C_SetCursor(&LCD1,0,0);
																	CLCD_I2C_WriteString(&LCD1, "NO USER CARD 3 ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
																}
																else
																{
																	removeuid(CheckKey(keyadd2));
																	CLCD_I2C_Display(&LCD1,"  DELETE CARD 3 ","   SUCCESSFUL  ");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1,"=>  USER CARD 3 ","    BACK ");
																}
																break;
															default:
																back12 = 0;
																break;
															}
														}
													}
													CLCD_I2C_Display(&LCD1,"   ADMIN CARD","=> USER CARD");
													break;
												default:
													backrm10 = 0;
													break;
												}
											}
										}
										CLCD_I2C_Display(&LCD1,"=> SELECT CARD","   SCAN CARD");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1,"SCAN CARD","=>  BACK");
										uint8_t rmquet = 1;
										while (rmquet)
										{
											key_pressed = KeyPad_WaitForKeyGetChar(10);
											if (TM_MFRC522_Check(CardID) == MI_OK)
											{
												if (CheckListUID(CardID) != 0)
												{
													removeuid(CheckKey(CheckListUID(CardID)));
													CLCD_I2C_Display(&LCD1,"  DELETE CARD ","   SUCCESSFUL  ");
													HAL_Delay(1000);
													if (checkcountUID() == 0)
													{
														startadd();
														rmquet = 1;
														exitmenu = 0;
														return;
													}else{
														CLCD_I2C_Display(&LCD1,"SCAN CARD","=>  BACK");
													}

												}
												else
												{
													CLCD_I2C_Clear(&LCD1);
													CLCD_I2C_SetCursor(&LCD1,0,0);
													CLCD_I2C_WriteString(&LCD1, "CARD UNAVAILABLE");
													HAL_Delay(1000);
													CLCD_I2C_Display(&LCD1,"SCAN CARD","=>  BACK");
												}
											}
											if (key_pressed =='#')
											{
												rmquet = 0;
											}
										}
										CLCD_I2C_Display(&LCD1,"   SELECT CARD","=> SCAN CARD");
										break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1,"=> REMOVE CARD","   REMOVE ALL");
							break;
						case 1:
							remoall();
							startadd();
							exitmenu = 0;
							break;
						default:
							backrm = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1,"    ADD CARD","=>  REMOVE CARD");
				break;
			case 2:
				checkthe();
				CLCD_I2C_Display(&LCD1,"    REMOVE CARD","=>  CHECK CARD");
				break;
			default:
				exitmenu = 0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
	HAL_Delay(1000);
}
void FACEID(void) {
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1, "     CARDID     ", "    THEM FACE    ");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed == '*')
		{
			exitmenu = Delaymenu;
			status++;
			status = (status > 3) ? 0 : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1, "=>  THEM FACE", "    XOA FACE");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1, "    THEM FACE", "=>  XOA FACE");
				break;
			case 2:
				CLCD_I2C_Display(&LCD1, "    XOA FACE", "=>  TRA FACE");
				break;
			default:
				CLCD_I2C_Display(&LCD1, "    TRA FACE", "=>  BACK");
				break;
			}
		}
		if (key_pressed == '#')
		{
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1, "    SELECT ", "FACE NGUOI LON");
				uint8_t statusadd = 0;
				uint8_t back = 1;
				while (back == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed == '*')
					{
						exitmenu = Delaymenu;
						statusadd++;
						statusadd = (statusadd > 2) ? 0 : statusadd;
						switch (statusadd)
						{
						case 1:
							CLCD_I2C_Display(&LCD1, "=>FACE NGUOI LON", "  FACE TRE EM");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1, "  FACE NGUOI LON", "=>FACE TRE EM");
							break;
						default:
							CLCD_I2C_Display(&LCD1, "  FACE TRE EM", "=> BACK");
							break;
						}
					}
					if (key_pressed == '#')
					{
						exitmenu = Delaymenu;
						switch (statusadd)
						{
						case 1:
							CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
							uint8_t statusadd1 = 1;
							uint8_t back11 = 1;
							while (back11 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed == '*')
								{
									exitmenu = Delaymenu;
									statusadd1++;
									statusadd1 = (statusadd1 > 4) ? 0 : statusadd1;
									switch (statusadd1)
									{
									case 1:
										CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										break;
									case 2:
										CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										break;
									case 3:
										CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										break;
									case 4:
										CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										break;
									default:
										CLCD_I2C_Display(&LCD1, "    FACE 4 ", "=>  BACK ");
										break;
									}
								}
								if (key_pressed == '#')
								{
									exitmenu = Delaymenu;
									uint8_t keyadd1 = (statusadd << 4) + statusadd1;
									switch (statusadd1)
									{
									case 1:
										if (checkfaceid(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 1 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										}
										else
										{
											addface(keyadd1);
											CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										}
										break;
									case 2:
										if (checkfaceid(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 2 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										}
										else
										{
											addface(keyadd1);
											CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										}
										break;
									case 3:
										if (checkfaceid(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 3 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										}
										else
										{
											addface(keyadd1);
											CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										}
										break;
									case 4:
										if (checkfaceid(keyadd1) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 4 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										}
										else
										{
											addface(keyadd1);
											CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										}
										break;
									default:
										back11 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1, "=>FACE NGUOI LON", "  FACE TRE EM");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
							uint8_t statusadd2 = 1;
							uint8_t back12 = 1;
							while (back12 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed == '*')
								{
									exitmenu = Delaymenu;
									statusadd2++;
									statusadd2 = (statusadd2 > 4) ? 0 : statusadd2;
									switch (statusadd2)
									{
									case 1:
										CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										break;
									case 2:
										CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										break;
									case 3:
										CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										break;
									case 4:
										CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										break;
									default:
										CLCD_I2C_Display(&LCD1, "    FACE 4 ", "=>  BACK ");
										break;
									}
								}
								if (key_pressed == '#')
								{
									exitmenu = Delaymenu;
									uint8_t keyadd2 = (statusadd << 4) + statusadd2;
									switch (statusadd2)
									{
									case 1:
										if (checkfaceid(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 1 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										}
										else
										{
											addface(keyadd2);
											CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
										}
										break;
									case 2:
										if (checkfaceid(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 2 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										}
										else
										{
											addface(keyadd2);
											CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
										}
										break;
									case 3:
										if (checkfaceid(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 3 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										}
										else
										{
											addface(keyadd2);
											CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
										}
										break;
									case 4:
										if (checkfaceid(keyadd2) != 0)
										{
											CLCD_I2C_Display(&LCD1, "  DA CO FACE 4 ", "");
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										}
										else
										{
											addface(keyadd2);
											CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
										}
										break;
									default:
										back12 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1, "  FACE NGUOI LON", "=>FACE TRE EM");
						default:
							back = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1, "=>  THEM FACE", "    XOA FACE");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1, "     SELECT ", "   XOA 1 FACE");
				uint8_t statusremove = -1;
				uint8_t backrm = 1;
				while (backrm == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed == '*')
					{
						exitmenu = Delaymenu;
						statusremove++;
						statusremove = (statusremove > 2) ? 0 : statusremove;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1, "=> XOA 1 FACE", "   XOA TAT CA");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1, "   XOA 1 FACE", "=> XOA TAT CA");
							CLCD_I2C_Clear(&LCD1);
							CLCD_I2C_SetCursor(&LCD1, 0, 0);
							CLCD_I2C_WriteString(&LCD1, "   XOA 1 FACE");
							CLCD_I2C_SetCursor(&LCD1, 0, 1);
							CLCD_I2C_WriteString(&LCD1, "=> XOA TAT CA");
							break;
						default:
							CLCD_I2C_Display(&LCD1, "   XOA TAT CA", "=> BACK");
							break;
						}
					}
					if (key_pressed == '#')
					{
						exitmenu = Delaymenu;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1, "=> CHON FACE", "   QUET FACE");
							uint8_t statusrm1 = 0;
							uint8_t backrm1 = 1;
							while (backrm1 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed == '*')
								{
									statusrm1++;
									statusrm1 = (statusrm1 > 2) ? 0 : statusrm1;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1, "=> CHON FACE", "   QUET FACE");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1, "   CHON FACE", "=> QUET FACE");
										;
										break;
									default:
										CLCD_I2C_Display(&LCD1, "   QUET FACE", "=> BACK");
										break;
									}
								}
								if (key_pressed == '#')
								{
									exitmenu = Delaymenu;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1, "=>FACE NGUOI LON", "  FACE TRE EM");
										uint8_t statusadd = 1;
										uint8_t backrm10 = 1;
										while (backrm10 == 1)
										{
											key_pressed = KeyPad_WaitForKeyGetChar(10);
											if (exitmenu == 0)
											{
												CLCD_I2C_Clear(&LCD1);
												HAL_Delay(1000);
												return;
											}
											if (key_pressed == '*')
											{
												exitmenu = Delaymenu;
												statusadd++;
												statusadd = (statusadd > 2) ? 0 : statusadd;
												switch (statusadd)
												{
												case 1:
													CLCD_I2C_Display(&LCD1, "=>FACE NGUOI LON", "  FACE TRE EM");
													break;
												case 2:
													CLCD_I2C_Display(&LCD1, "  FACE NGUOI LON", "=>FACE TRE EM");
													break;
												default:
													CLCD_I2C_Display(&LCD1, "  FACE TRE EM", "=> BACK");
													break;
												}
											}
											if (key_pressed == '#')
											{
												exitmenu = Delaymenu;
												switch (statusadd)
												{
												case 1:
													CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
													uint8_t statusadd1 = 1;
													uint8_t back11 = 1;
													while (back11 == 1)
													{
														key_pressed = KeyPad_WaitForKeyGetChar(10);
														if (exitmenu == 0)
														{
															CLCD_I2C_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (key_pressed == '*')
														{
															exitmenu = Delaymenu;
															statusadd1++;
															statusadd1 = (statusadd1 > 4) ? 0 : statusadd1;
															switch (statusadd1)
															{
															case 1:
																CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																break;
															case 2:
																CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																break;
															case 3:
																CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																break;
															case 4:
																CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																break;
															default:
																CLCD_I2C_Display(&LCD1, "    FACE 4 ", "=>  BACK ");
																break;
															}
														}
														if (key_pressed == '#')
														{
															exitmenu = Delaymenu;
															uint8_t keyadd1 = (statusadd << 4) + statusadd1;
															switch (statusadd1)
															{
															case 1:
																if (checkfaceid(keyadd1) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 1 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd1));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																	}
																}
																break;
															case 2:
																if (checkfaceid(keyadd1) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 2 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd1));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																	}
																}
																break;
															case 3:
																if (checkfaceid(keyadd1) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 3 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd1));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																	}
																}
																break;
															case 4:
																if (checkfaceid(keyadd1) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 4 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																}
																else
																{
																	removeface(checkfaceid(keyadd1));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	if (checkcountUID() == 0)
																	{
																		startadd();
																		exitmenu = 0;
																	}
																	else
																	{
																		CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																	}
																}
																break;
															default:
																back11 = 0;
																break;
															}
														}
													}
													CLCD_I2C_Display(&LCD1, "=>FACE NGUOI LON", "  FACE TRE EM");
													break;
												case 2:
													CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
													uint8_t statusadd2 = 1;
													uint8_t back12 = 1;
													while (back12 == 1)
													{
														key_pressed = KeyPad_WaitForKeyGetChar(10);
														if (exitmenu == 0)
														{
															CLCD_I2C_Clear(&LCD1);
															HAL_Delay(1000);
															return;
														}
														if (key_pressed == '*')
														{
															exitmenu = Delaymenu;
															statusadd2++;
															statusadd2 = (statusadd2 > 4) ? 0 : statusadd2;
															switch (statusadd2)
															{
															case 1:
																CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																break;
															case 2:
																CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																break;
															case 3:
																CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																break;
															case 4:
																CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																break;
															default:
																CLCD_I2C_Display(&LCD1, "    FACE 4 ", "=>  BACK ");
																break;
															}
														}
														if (key_pressed == '#')
														{
															exitmenu = Delaymenu;
															uint8_t keyadd2 = (statusadd << 4) + statusadd2;
															switch (statusadd2)
															{
															case 1:
																if (checkfaceid(keyadd2) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 1 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd2));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 1 ", "    FACE 2 ");
																}
																break;
															case 2:
																if (checkfaceid(keyadd2) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 2 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd2));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 2 ", "    FACE 3 ");
																}
																break;
															case 3:
																if (checkfaceid(keyadd2) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 3 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																}
																else
																{
																	removeface(checkfaceid(keyadd2));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 3 ", "    FACE 4 ");
																}
																break;
															case 4:
																if (checkfaceid(keyadd2) == 0)
																{
																	CLCD_I2C_Display(&LCD1, "CHUA CO FACE 4 ", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																}
																else
																{
																	removeface(checkfaceid(keyadd2));
																	CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
																	HAL_Delay(1000);
																	CLCD_I2C_Display(&LCD1, "=>  FACE 4 ", "    BACK ");
																}
																break;
															default:
																back12 = 0;
																break;
															}
														}
													}
													CLCD_I2C_Display(&LCD1, "  FACE NGUOI LON", "=>FACE TRE EM");
													break;
												default:
													backrm10 = 0;
													break;
												}
											}
										}
										CLCD_I2C_Display(&LCD1, "=> CHON FACE", "   QUET FACE");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1, "QUET FACE", "=>  BACK ");
										CDC_Transmit_FS("Rem.00", 6);
										uint8_t rmquet = 1;
										while (rmquet)
										{
											key_pressed = KeyPad_WaitForKeyGetChar(10);
											if( Rx_Buffer[0] == 'T'){
												CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
												HAL_Delay(2000);
												memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
												CLCD_I2C_Display(&LCD1, "QUET FACE", "=>  BACK ");
											}
											if (key_pressed == '#')
											{
												rmquet = 0;
											}
										}
										CDC_Transmit_FS("Exit  ", 6);
										CLCD_I2C_Display(&LCD1, "   CHON FACE", "=> QUET FACE");;
										break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1, "=> XOA 1 FACE", "   XOA TAT CA");
							break;
						case 1:
							sprintf(Tx_Buffer , "Rem.99" );
							CDC_Transmit_FS(Tx_Buffer, 6);
							CLCD_I2C_Display(&LCD1, "WAITING....", "");
							exitmenu = 60;
							while(exitmenu != 0){
								if(Rx_Buffer[0] == 'T'){
									CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
									memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
									break;
								}
							}
							exitmenu = 0;
							break;
						default:
							backrm = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1, "    THEM FACE", "=>  XOA FACE");
				break;
			case 2:
				checkface();
				CDC_Transmit_FS("Exit  ", 6);
				CLCD_I2C_Display(&LCD1, "    XOA FACE", "=>  TRA FACE");
				break;
			default:
				exitmenu = 0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}void FINGER()
{
	add_finger();
}
void PASSWORD(){}
uint8_t CheckUID(uint8_t *data, uint32_t address)
{
	uint8_t arr[8];
	Flash_Read_Array(address, arr, 8);
	if (arr[6] != 0xFF)
		return 0;
	for (uint8_t i = 0; i < 5; i++)
	{
		if (data[i] != arr[i])
			return 0;
	}
	return 1;
}

uint8_t CheckListUID(uint8_t *data)
{
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if (CheckUID(data, pt) == 1)
				return *(uint8_t *)(pt + 5);
		}
		pt = pt + 8;
	}
	return 0;
}

uint8_t checkcountUID(void)
{
	uint32_t pt = StartAddressUID;
	uint8_t count = 0;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if ((Flash_Read_Byte(pt + 5) >> 4) == 1)
			{
				count++;
			}
		}
		pt = pt + 8;
	}
	return count;
}

void adduid(uint8_t key)
{
	setaddress();
	CLCD_I2C_Display(&LCD1, "SCAN CARD", "=>  BACK");
	while (exitmenu)
	{
		if (TM_MFRC522_Check(CardID) == MI_OK)
		{
			HAL_Delay(100);
			if (CheckListUID(CardID) == 0)
			{
				CardID[5] = key;
				Flash_Write_Array(AddressUID, CardID, 6);
				AddressUID += 8;
				CLCD_I2C_Clear(&LCD1);
				CLCD_I2C_SetCursor(&LCD1, 0, 0);
				CLCD_I2C_WriteString(&LCD1, "SUCCESSFUL");
				HAL_Delay(1000);
				return;
			}
			else
			{
				CLCD_I2C_Clear(&LCD1);
				CLCD_I2C_SetCursor(&LCD1, 0, 0);
				CLCD_I2C_WriteString(&LCD1, "CARD AVAILABLE");
				HAL_Delay(1000);
				CLCD_I2C_Display(&LCD1, "SCAN CARD", "=>  BACK");
			}
		}
		if (KeyPad_WaitForKeyGetChar(100)=='#')
		{
			return;
		}
	}
}

void checkthe(void)
{
	exitmenu = 30;
	CLCD_I2C_Display(&LCD1, "SCAN CARD", "=>  BACK");
	while (exitmenu )
	{
		if (TM_MFRC522_Check(CardID) == MI_OK)
		{
			if (CheckListUID(CardID) == 0)
			{
				CLCD_I2C_Clear(&LCD1);
				CLCD_I2C_SetCursor(&LCD1, 0, 0);
				CLCD_I2C_WriteString(&LCD1, "CARD NOT ADDED");
				HAL_Delay(1000);
				CLCD_I2C_Display(&LCD1, "SCAN CARD", "=>  BACK");
				HAL_Delay(1000);
			}
			else
			{
				uint8_t key = CheckListUID(CardID);
				uint8_t key2 = key & 0x0f;
				uint8_t key1 = key >> 4;
				CLCD_I2C_Clear(&LCD1);
				switch (key1)
				{
				case 1:
					CLCD_I2C_SetCursor(&LCD1, 0, 0);
					CLCD_I2C_WriteString(&LCD1, "ADMIN CARD");
					break;
				default:
					CLCD_I2C_SetCursor(&LCD1, 0, 0);
					CLCD_I2C_WriteString(&LCD1, "GUEST CARD");
					break;
				}
				switch (key2)
				{
				case 1:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "CARD 1");
					break;
				case 2:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "CARD 2");
					break;
				default:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "CARD 3");
					break;
				}
				HAL_Delay(1000);
				CLCD_I2C_Display(&LCD1, "SCAN CARD", "=>  BACK");
			}
		}
		if (KeyPad_WaitForKeyGetChar(100)=='#')
		{
			return;
		}
	}
}
uint32_t CheckKey(uint8_t key)
{
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
			if (*(uint8_t *)(pt + 5) == key)
				return pt;
		}
		pt = pt + 8;
	}
	return 0;
}
void removeuid(uint32_t addressrm)
{
	Flash_Write_2Byte(addressrm + 6, 0x0000);
}
void startadd(void)
{
	CLCD_I2C_Display(&LCD1, "SCAN CARD","ADMIN CARD");
	setaddress();
	while (1)
		{
			if (TM_MFRC522_Check(CardID) == MI_OK)
			{
				if (CheckListUID(CardID) == 0)
				{
					CardID[5] = 0x11;
					Flash_Write_Array(AddressUID, CardID, 6);
					AddressUID += 8;
					break;
				}
				else
				{
					CLCD_I2C_Clear(&LCD1);
					CLCD_I2C_SetCursor(&LCD1, 0, 0);
					CLCD_I2C_WriteString(&LCD1, "CARD AVAILABLE");
					HAL_Delay(1000);
					CLCD_I2C_Display(&LCD1, "SCAN CARD","ADMIN CARD");
				}
			}
		}
	CLCD_I2C_Display(&LCD1, "ADD SUCCESSFUL","ADMIN CARD");
	HAL_Delay(1000);
	CLCD_I2C_Clear(&LCD1);
}
void setaddress(void){
	uint32_t pt = StartAddressUID;
	while (Flash_Read_Byte(pt + 5) != 0xFF)
	{
		pt = pt + 8;
	}
	AddressUID = pt;
}

void remoall(){
	uint32_t pt = StartAddressUID;
	while(Flash_Read_8Byte(pt) != 0xFFFFFFFFFFFFFFFF){
		Flash_Erase(pt);
		pt = pt + 0x400;
		if(pt == 0x800FC00)
			break;
	}
}
void resetflash(void){
	if(AddressUID == 0x800FC00){
		uint32_t pt = StartAddressUID;
		uint8_t uidcard[8][6];
		uint8_t k = 0;
		while(pt != 0x800FC00){
			if(Flash_Read_2Byte(pt + 6) == 0xFFFF){
				for(uint8_t i = 0; i < 6; i++){
					uidcard[k][i] = Flash_Read_Byte(pt+i);
				}
				k++;
			}

			pt += 8;
		}
		remoall();
		pt = StartAddressUID;
		for(uint8_t i = 0; i < k; i++ ){
			Flash_Write_Array(pt, uidcard[i], 6);
			pt += 8;
		}
	}
}

//---------- them van tay---------------
void add_finger()
{
	vitri2:
	while(1)
	{
		collect_finger();
		CLCD_I2C_Display(&LCD1, "  Them Van Tay!!     ", "Dat Van Tay!!     ");
		HAL_Delay(1000);
	// dat tay vao
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Reading finger...!!     ");
		tmp=0xff;
		while(tmp!=0x00){
			collect_finger();
			collect_finger();
			tmp= collect_finger();
		}
		tmp=0xff;
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Remove Finger!!   ");HAL_Delay(100);
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Processing Finger!!   ");
		tmp=0xff;
		while(tmp!=0x00){
		tmp=img2tz(0x01);
		}
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"dat lai van tay !!   ");HAL_Delay(100);
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Reading finger...!!     ");
		tmp=0xff;
		while(tmp!=0x00)	{
			collect_finger();
			collect_finger();
			tmp=collect_finger();
		}
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Remove Finger!!   ");HAL_Delay(100);
		tmp=0xff;
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Processing Finger!!   ");
		while(tmp!=0x00)	{tmp=img2tz(0x02);}
		tmp=0xff;
		// kiem tra 2 buff co trung nhau khong
		while(tmp!=0x00)
		{
			tmp=match();	//HAL_Delay(100);
			if(tmp==0x08||tmp==0x01)
			{
				// loi, lam lai
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1,"LOI, Lam Lai!!   ");HAL_Delay(1500);
				goto vitri2;
			}
		}
		tmp=0xff;
		while(tmp!=0x00){tmp=regmodel();HAL_Delay(100);}
		tmp=0xff;
		while(tmp!=0x00){tmp=store(ID);HAL_Delay(100);}			// luu id
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"  Save Finger!    ");
				/***************** DA LUU XONG**************************/
		HAL_Delay(1500);
		tmp=0xff;
		CLCD_I2C_Clear(&LCD1);
		break;
	}
}
						//----------end them van tay---------------
void read_finger()
{
/**************************BEgin Doc van tay*****************************/
	tmp=0xff;
	time_cho=HAL_GetTick();
	while(tmp!=0x00){
		tmp=collect_finger();
		if(HAL_GetTick()-time_cho>=1600) {
		time_cho=HAL_GetTick();
		return;}

	}
	tmp=0xff;
	if(tmp!=0x00){tmp=img2tz(0x01);}
	tmp=0xff;
	tmp=search();
	if(tmp==0x00)
	{
		tmp=0xff;	// co van tay
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Mo Cua!");
		sprintf(mess," #id = %c  ",pID);
		CLCD_I2C_WriteString(&LCD1,mess);
		HAL_Delay(1000);
		CLCD_I2C_Clear(&LCD1);
	}
	if(tmp==0x09)	// khong co van tay
	{
		tmp=0xff;
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1," Van Tay Sai!!     "); HAL_Delay(1000);
		CLCD_I2C_WriteString(&LCD1,mess);
		HAL_Delay(1000);
		CLCD_I2C_Clear(&LCD1);
	}
}
void startface(void)
{
	if(Rx_Buffer[0] == 'T'){
        CLCD_I2C_Clear(&LCD1);
        CLCD_I2C_SetCursor(&LCD1, 0, 0);
        CLCD_I2C_WriteString(&LCD1, "    WELCOME");
	}else if(Rx_Buffer[0] == 'F'){
        CLCD_I2C_Clear(&LCD1);
        CLCD_I2C_SetCursor(&LCD1, 0, 0);
        CLCD_I2C_WriteString(&LCD1, "FALSE FACE");
	}
	memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}
void addface(uint8_t key)
{
	sprintf(Tx_Buffer , "Add.%2d", key );
	CDC_Transmit_FS(Tx_Buffer, 6);
	CLCD_I2C_Display(&LCD1, "WAITING....", "");
	exitmenu = 60;
	while(exitmenu != 0){
		if(Rx_Buffer[0] == 'T'){
			CLCD_I2C_Display(&LCD1, "THEM THANH CONG", "");
			break;
		}
		else if(Rx_Buffer[0] == 'F'){
			CLCD_I2C_Display(&LCD1, "FACE DA TON TAI", "");
			break;
		}
	}
	memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}
void removeface(uint8_t key)
{
	sprintf(Tx_Buffer , "Rem.%2d", key );
	CDC_Transmit_FS(Tx_Buffer, 6);
	CLCD_I2C_Display(&LCD1, "WAITING....", "");
	exitmenu = 60;
	while(exitmenu != 0){
		if(Rx_Buffer[0] == 'T'){
			CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
			break;
		}
		else if(Rx_Buffer[0] == 'F'){
			CLCD_I2C_Display(&LCD1, "FACE CHUA THEM", "");
			break;
		}
	}
	memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}
uint8_t checkfaceid(uint8_t key){
	sprintf(Tx_Buffer , "Che.%2d", key );
	CDC_Transmit_FS(Tx_Buffer, 6);
	while(Rx_Buffer[0] ==0){
		continue;
	}
	CLCD_I2C_Display(&LCD1, Rx_Buffer, "");
	HAL_Delay(1000);
	if(Rx_Buffer[0] == '0'){
		return 0;
	}
	return key;

}

void checkface(void)
{
	CDC_Transmit_FS("Che.00", 6);
	exitmenu = 60;
	CLCD_I2C_Display(&LCD1, "QUET FACE", "=>  BACK");
	while (exitmenu )
	{
		if (Rx_Buffer[0] != 0)
		{

			if (Rx_Buffer[0] == 'F')
			{
				CLCD_I2C_Display(&LCD1, "FACE CHUA THEM", "");
				HAL_Delay(1000);
				CLCD_I2C_Display(&LCD1, "QUET FACE", "=>  BACK");
				HAL_Delay(1000);
			}
			else
			{
				uint8_t key = (Rx_Buffer[5] -48)*10 + (Rx_Buffer[6] -48) ;
				uint8_t key2 = key & 0x0f;
				uint8_t key1 = key >> 4;
				CLCD_I2C_Clear(&LCD1);
				switch (key1)
				{
				case 1:
					CLCD_I2C_SetCursor(&LCD1, 0, 0);
					CLCD_I2C_WriteString(&LCD1, "FACE NGUOI LON");
					break;
				default:
					CLCD_I2C_SetCursor(&LCD1, 0, 0);
					CLCD_I2C_WriteString(&LCD1, "FACE TRE EM");
					break;
				}
				switch (key2)
				{
				case 1:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "FACE 1");
					break;
				case 2:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "FACE 2");
					break;
				case 3:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "FACE 3");
					break;
				default:
					CLCD_I2C_SetCursor(&LCD1, 0, 1);
					CLCD_I2C_WriteString(&LCD1, "FACE 4");
					break;
				}
				HAL_Delay(2000);
				CLCD_I2C_Display(&LCD1, "QUET FACE", "=>  BACK");
			}
			memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
		}
		if (KeyPad_WaitForKeyGetChar(100)=='#')
		{
			break;
		}
	}
	CDC_Transmit_FS("Exit  ", 6);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
