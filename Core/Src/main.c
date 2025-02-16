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
#define StartAddressPassword 0x0800F400
#define StartAddressFingerID 0x0800F800
#define Delaymenu 20
#define opendoortime 3000
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
volatile int8_t Rx_Buffer[128];
char Tx_Buffer[10];
CLCD_I2C_Name LCD1;
uint8_t CardID[MFRC522_MAX_LEN];
uint8_t FingerID[1];
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
void startface(void);
void addface(uint8_t key);
void removeface(uint8_t key);
uint8_t checkfaceid(uint8_t key);
void add_finger();
void read_finger();
void remove_id_finger();
void remove_all_finger();
uint32_t CheckKeyFinger(uint8_t key);
void AddFingerID(uint8_t key);
void RemoveFingerID(uint32_t addressrm);
void enter_password(char *password);
void change_password(void);
uint8_t check_password(char *password);
void set_default_password(void);
void opendoor(void);
void buzzer( uint8_t countbeep);
uint8_t InputID_ADMIN(void);
uint8_t InputID_USER(void);
uint8_t InputID_FINGER();
uint8_t InputID_FACE();
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

uint8_t InputID(const char *prompt, uint8_t min, uint8_t max) {
    uint16_t id = 0;
    char id_str[4] = {0};
    uint8_t index = 0;
    char display_prompt[16];
    snprintf(display_prompt, sizeof(display_prompt), "Enter ID (%d-%d):", min, max);
    CLCD_I2C_Display(&LCD1, display_prompt, "ID= ");
    uint32_t start_time = HAL_GetTick();
    while (1) {
        if (HAL_GetTick() - start_time > 15000) { // 15 seconds timeout
            CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
            buzzer(5);
            HAL_Delay(2000);
            exitmenu = Delaymenu;
            return 0; // Return 0 to indicate timeout
        }
        char key = KeyPad_WaitForKeyGetChar(10);
        if (key >= '0' && key <= '9' && index < 3) {
            buzzer(1);
            id_str[index++] = key;
            CLCD_I2C_WriteChar(&LCD1, key);
        } else if (key == '#' && index > 0) {
            buzzer(1);
            id = atoi(id_str);
            if (id >= min && id <= max) {
                break;
            } else {
                CLCD_I2C_Display(&LCD1, "Invalid ID", display_prompt);
                buzzer(5);
                HAL_Delay(2000);
                CLCD_I2C_Display(&LCD1, display_prompt, "ID= ");
                memset(id_str, 0, sizeof(id_str));
                index = 0;
                start_time = HAL_GetTick(); // Reset the timer
            }
        }
    }
    CLCD_I2C_SetCursor(&LCD1, 4, 1);
    CLCD_I2C_WriteString(&LCD1, id_str);
    HAL_Delay(1000);
    return id;
}

uint8_t InputID_ADMIN() {
    return InputID("EnterID (1-27):", 1, 27);
}

uint8_t InputID_USER() {
    return InputID("EnterID (1-100):", 1, 100);
}

uint8_t InputID_FINGER() {
    return InputID("EnterID (1-127):", 1, 127);
}

uint8_t InputID_FACE() {
    return InputID("EnterID (1-127):", 1, 127);
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
  TM_MFRC522_Init();
  KeyPad_Init();
  CLCD_I2C_Init(&LCD1, &hi2c2, 0x4E, 16, 2);

  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin,1);
  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 0);

  // Check if the password is set, if not, set the default password
  char stored_password[7] = {0};
  Flash_Read_Array(StartAddressPassword, (uint8_t *)stored_password, 6);
  stored_password[6] = '\0';
  int is_empty = 1;
  for (int i = 0; i < 6; i++) {
      if (stored_password[i] != (char)0xFF) {
          is_empty = 0;
          break;
      }
  }
  if (is_empty) {
      set_default_password();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_Base_Start_IT(&htim2);
  if (checkcountUID() == 0)
  {
      startadd();
  }
  int incorrect_attempts = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    CLCD_I2C_Clear(&LCD1);
    CLCD_I2C_SetCursor(&LCD1, 0, 0);
    CLCD_I2C_WriteString(&LCD1, " SCAN YOUR CARD");

    char selected_key = KeyPad_WaitForKeyGetChar(10); // Ch�? vô hạn cho đ���n khi có phím nhấn.

    if (selected_key == '#')
    {
    	buzzer(1);
        char entered_password[7] = {0};
        CLCD_I2C_Display(&LCD1, "ENTER PASSWORD", "");
        enter_password(entered_password);
        if (check_password(entered_password)) {
            CLCD_I2C_Display(&LCD1, "    WELCOME", "");
            opendoor();
            incorrect_attempts = 0; // Reset incorrect attempts on successful login
        } else {
            incorrect_attempts++;
            CLCD_I2C_Display(&LCD1, "WRONG PASSWORD", "");
            buzzer(5);
            int delay_time = 0;
            if (incorrect_attempts == 1) {
                delay_time = 5;
            } else if (incorrect_attempts == 2) {
                delay_time = 10;
            } else if (incorrect_attempts >= 3) {
                delay_time = 20;
            }
            for (int i = delay_time; i > 0; i--) {
                char buffer[16];
                snprintf(buffer, sizeof(buffer), "     WAIT %ds", i);
                CLCD_I2C_Display(&LCD1, buffer, "  TO TRY AGAIN");
                HAL_Delay(1000);
            }
        }
        HAL_Delay(2000);
        CLCD_I2C_Clear(&LCD1);
    }
    else if (selected_key != 0)
    {
    	buzzer(1);
        uint8_t key = 0; // Key quản lý quy�?n truy cập.
        exitmenu = 15;

        switch (selected_key)
        {
        case 'A': // RFID
        case 'B': // FACEID
        case 'C': // FINGER
        case 'D': // PASSWORD
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1, "   Admin Card");

            while (exitmenu)
            {
                if (TM_MFRC522_Check(CardID) == MI_OK)
                {
                    key = CheckListUID(CardID);
                    key = key >> 7;
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
                buzzer(5);
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
            opendoor();
        }
        else
        {
            CLCD_I2C_Clear(&LCD1);
            CLCD_I2C_SetCursor(&LCD1, 0, 0);
            CLCD_I2C_WriteString(&LCD1, "   WRONG CARD");
            buzzer(5);
            HAL_Delay(2000);
        }
    }
    else if(Rx_Buffer[0]!= 0)
    {
        startface();
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
  HAL_GPIO_WritePin(GPIOA, CS_Pin|BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, R1_Pin|R2_Pin|R3_Pin|R4_Pin
                          |chotkhoa_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_SET);

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

  /*Configure GPIO pin : CS_Pin */
  GPIO_InitStruct.Pin = CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : R1_Pin R2_Pin R3_Pin R4_Pin */
  GPIO_InitStruct.Pin = R1_Pin|R2_Pin|R3_Pin|R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LOCK_Pin */
  GPIO_InitStruct.Pin = LOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LOCK_GPIO_Port, &GPIO_InitStruct);

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
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1," RFID SETTINGS ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed =='*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 2) ? (-1) : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Add Card");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Remove Card");
				break;
			case 2:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Check Card");
				break;
			default:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Back");
				break;
			}
		}
		if (key_pressed =='#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusadd = -1;
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
						buzzer(1);
						exitmenu = Delaymenu;
						statusadd++;
						statusadd = (statusadd > 1) ? (-1) : statusadd;
						switch (statusadd)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Admin Card");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> User Card");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Back");
							break;
						}
					}
					if (key_pressed =='#')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						switch (statusadd)
						{
						case 0:
							uint8_t AdminID = InputID_ADMIN();
							uint8_t keyadd_admin = (1 << 7) + AdminID;
							if (CheckKey(keyadd_admin)!=0)
							{
								CLCD_I2C_Display(&LCD1," ID is existing"," Pick another ID");
								buzzer(3);
								HAL_Delay(1000);
							}
							else
							{
								adduid(keyadd_admin);
							}
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Admin Card");
							break;
						case 1:
							uint8_t UserID = InputID_USER();
							uint8_t keyadd_user = (0 << 7) + UserID;
							if (CheckKey(keyadd_user)!=0)
							{
								CLCD_I2C_Display(&LCD1," ID is existing"," Pick another ID");
								buzzer(3);
								HAL_Delay(1000);
							}
							else
							{
								adduid(keyadd_user);
							}
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> User Card");
							break;
						default:
							back = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Add Card");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
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
						buzzer(1);
						exitmenu = Delaymenu;
						statusremove++;
						statusremove = (statusremove > 1) ? (-1) : statusremove;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Remove 1 Card");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Remove ALL");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Back");
							break;
						}
					}
					if (key_pressed =='#')
					{
						buzzer(1);
						CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
						exitmenu = Delaymenu;
						switch (statusremove)
						{
						case 0:
							uint8_t statusrm1 = -1;
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
									buzzer(1);
									statusrm1++;
									statusrm1 = (statusrm1 > 2) ? 0 : statusrm1;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Select Card");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Scan Card");
										break;
									default:
										CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Back");
										break;
									}
								}
								if (key_pressed =='#')
								{
									buzzer(1);
									exitmenu = Delaymenu;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
										uint8_t statusadd = -1;
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
												buzzer(1);
												exitmenu = Delaymenu;
												statusadd++;
												statusadd = (statusadd > 1) ? (-1) : statusadd;
												switch (statusadd)
												{
												case 0:
													CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
													break;
												case 1:
													CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM User Card");
													break;
												default:
													CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> Back");
													break;
												}
											}
											if (key_pressed =='#')
											{
												buzzer(1);
												exitmenu = Delaymenu;
												switch (statusadd)
												{
													case 0:
														uint8_t AdminID = InputID_ADMIN();
														uint8_t keyadd_admin = (1 << 7) + AdminID;
														if (CheckKey(keyadd_admin)==0)
														{
															CLCD_I2C_Display(&LCD1,"ID doesnt exist"," Pick another ID");
															buzzer(3);
															HAL_Delay(1000);
															CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
														}
														else
														{
															removeuid(CheckKey(keyadd_admin));
															CLCD_I2C_Display(&LCD1,"REMOVE ADMIN CARD","   SUCCESSFUL  ");
															HAL_Delay(1000);
															if (checkcountUID() == 0)
															{
																startadd();
																exitmenu = 0;
															}
														}
														CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
														break;
													case 1:
														uint8_t UserID = InputID_USER();
														uint8_t keyadd_user = (0 << 7) + UserID;
														if (CheckKey(keyadd_user)==0)
														{
															CLCD_I2C_Display(&LCD1,"ID doesnt exist"," Pick another ID");
															buzzer(3);
															HAL_Delay(1000);
														}
														else
														{
															removeuid(CheckKey(keyadd_user));
															CLCD_I2C_Display(&LCD1,"REMOVE USER CARD","   SUCCESSFUL  ");
															HAL_Delay(1000);
															if (checkcountUID() == 0)
															{
																startadd();
																exitmenu = 0;
															}
														}
														CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM User Card");
														break;
													default:
														backrm10 = 0;
														break;
												}
											}
										}
										CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Select Card");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
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
														CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
													}
												}
												else
												{
													CLCD_I2C_Display(&LCD1, "   This card","  Do not exist");
													buzzer(3);
													HAL_Delay(1000);
													CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
												}
											}
											if (key_pressed =='#')
											{
												buzzer(1);
												rmquet = 0;
											}
										}
										CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Scan Card");
										break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Remove 1 Card");
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
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Remove Card");
				break;
			case 2:
				checkthe();
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Check Card");
				break;
			default:
				exitmenu = 0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}

uint8_t CheckListUID(uint8_t *data)
{
    uint32_t pt = StartAddressUID;
    while (Flash_Read_Byte(pt + 5) != 0xFF)
    {
        if (Flash_Read_2Byte(pt + 6) == 0xFFFF)
        {
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
        if (Flash_Read_2Byte(pt + 6) == 0xFFFF)
        {
            if ((Flash_Read_Byte(pt + 5) >> 7) == 1)
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
    CLCD_I2C_Display(&LCD1, "SCAN CARD", "=> Back");
    while (exitmenu)
    {
        if (TM_MFRC522_Check(CardID) == MI_OK)
        {
            HAL_Delay(100);
            if (CheckListUID(CardID) == 0)
            {
                buzzer(1);
                CardID[5] = key;
                Flash_Write_Array(AddressUID, CardID, 6);
                AddressUID += 8;
                CLCD_I2C_Clear(&LCD1);
                CLCD_I2C_SetCursor(&LCD1, 0, 0);
                CLCD_I2C_WriteString(&LCD1, "   SUCCESSFUL");
                HAL_Delay(1000);
                return;
            }
            else
            {
                CLCD_I2C_Clear(&LCD1);
                CLCD_I2C_SetCursor(&LCD1, 0, 0);
                CLCD_I2C_WriteString(&LCD1, "CARD EXISTED");
                buzzer(3);
                HAL_Delay(1000);
                CLCD_I2C_Display(&LCD1, "SCAN CARD", "=> Back");
            }
        }
        if (KeyPad_WaitForKeyGetChar(100) == '#')
        {
            buzzer(1);
            return;
        }
    }
}

void checkthe(void)
{
    exitmenu = 30;
    CLCD_I2C_Display(&LCD1, "SCAN CARD", "=> Back");
    while (exitmenu)
    {
        if (TM_MFRC522_Check(CardID) == MI_OK)
        {
            if (CheckListUID(CardID) == 0)
            {
                CLCD_I2C_Clear(&LCD1);
                CLCD_I2C_SetCursor(&LCD1, 0, 0);
                CLCD_I2C_WriteString(&LCD1, "CARD DONT EXIST");
                buzzer(3);
                HAL_Delay(1000);
                CLCD_I2C_Display(&LCD1, "SCAN CARD", "=> Back");
                HAL_Delay(1000);
            }
            else
            {
                uint8_t key = CheckListUID(CardID);
                uint8_t key2 = key & 0x7F;
                uint8_t key1 = (key & 0x80) >> 7;
                CLCD_I2C_Clear(&LCD1);
                buzzer(1);
                switch (key1)
                {
                case 1:
                    CLCD_I2C_SetCursor(&LCD1, 0, 0);
                    CLCD_I2C_WriteString(&LCD1, "ADMIN CARD");
                    break;
                default:
                    CLCD_I2C_SetCursor(&LCD1, 0, 0);
                    CLCD_I2C_WriteString(&LCD1, "USER CARD");
                    break;
                }
                char buffer[16];
                snprintf(buffer, sizeof(buffer), "Card ID: %d", key2);
                CLCD_I2C_SetCursor(&LCD1, 0, 1);
                CLCD_I2C_WriteString(&LCD1, buffer);
                HAL_Delay(1000);
                CLCD_I2C_Display(&LCD1, "PLS SCAN CARD", "=> Back");
            }
        }
        if (KeyPad_WaitForKeyGetChar(100) == '#')
        {
            buzzer(1);
            return;
        }
    }
}

uint32_t CheckKey(uint8_t key)
{
    uint32_t pt = StartAddressUID;
    while (Flash_Read_Byte(pt + 5) != 0xFF)
    {
        if (Flash_Read_2Byte(pt + 6) == 0xFFFF)
        {
            if (*(uint8_t *)(pt + 5) == key)
                return pt;
        }
        pt = pt + 8;
    }
    return 0;
}

void FACEID(void) {
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed == '*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 1) ? (-1) : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Add FaceID");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Remove FaceID");
				break;
			default:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Back");
				break;
			}
		}
		if (key_pressed == '#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
				uint8_t FaceID = InputID_FACE();
				if (checkfaceid(FaceID) != 0)
				{
					CLCD_I2C_Display(&LCD1,"    FACEID "," Face Existed ");
					buzzer(3);
					HAL_Delay(1000);
				}
				else
				{
					addface(FaceID);
				}
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Add FaceID");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusrm = 0;
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
						buzzer(1);
						exitmenu = Delaymenu;
						statusrm++;
						statusrm = (statusrm > 2) ? 0 : statusrm;
						switch (statusrm)
						{
						case 1:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove 1 Face");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove All");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Back");
							break;
						}
					}
					if (key_pressed == '#')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						switch (statusrm)
						{
						case 1:
							uint8_t FaceID = InputID_FACE();
							if (checkfaceid(FaceID) == 0)
							{
								CLCD_I2C_Display(&LCD1, "    FaceID ", "  Do Not Exist");
								buzzer(3);
								HAL_Delay(1000);
							}
							else
							{
								removeface(FaceID);
								CLCD_I2C_Display(&LCD1,"REMOVE FACEID ","   SUCCESSFUL  ");
								HAL_Delay(1000);
							}
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove 1 Face");
							break;
						case 2:
							sprintf(Tx_Buffer , "Del.ALL" );
							CDC_Transmit_FS(Tx_Buffer, 7);
							CLCD_I2C_Display(&LCD1, "WAITING....", "");
							exitmenu = 60;
							memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
							while(exitmenu != 0){
								if(Rx_Buffer[0] == 'T'){
									CLCD_I2C_Display(&LCD1, "REMOVE ALL FACE","   SUCCESSFUL  ");
									HAL_Delay(2000);
									memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
									break;
								}
							}
							exitmenu = 0;
							break;
						default:
							backrm=0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Remove FaceID");
				break;
			default:
				exitmenu=0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}

void FINGER(void) {
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"FINGER SETTING ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed == '*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 1) ? (-1) : status;
			switch (status)
			{
			case 0:
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Add Finger");
				break;
			case 1:
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove Finger");
				break;
			default:
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Back");
				break;
			}
		}
		if (key_pressed == '#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
			    add_finger();
				CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Add Finger");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusrm = 0;
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
						buzzer(1);
						exitmenu = Delaymenu;
						statusrm++;
						statusrm = (statusrm > 2) ? 0 : statusrm;
						switch (statusrm)
						{
						case 1:
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Remove 1 Finger");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Remove All");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Back");
							break;
						}
					}
					if (key_pressed == '#')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						switch (statusrm)
						{
						case 1:
							remove_id_finger();
							CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove 1 Finger");
							break;
						case 2:
							remove_all_finger();
							CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove All");
                			break;
						default:
							backrm=0;
							break;
						}
					}
				}
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove Finger");
				break;
			default:
				exitmenu=0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}

void PASSWORD(void) {
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed == '*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 2) ? 0 : status;
			switch (status)
			{
			case 0:
	            CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","=> Change Pass");
				break;
			case 1:
	            CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","=> Reset Pass");
				break;
			default:
	            CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","=> Back");
				break;
			}
		}
		if (key_pressed == '#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
                change_password();
	            CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","=> Change Pass");
                break;
            case 1:
                set_default_password();
	            CLCD_I2C_Display(&LCD1,"PASSWORD SETTING ","=> Reset Pass");
                break;
            default:
                exitmenu = 0;
                break;
            }
        }
    }
}

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

void removeuid(uint32_t addressrm)
{
	Flash_Write_2Byte(addressrm + 6, 0x0000);
}
void startadd(void)
{
	CLCD_I2C_Display(&LCD1, "PLS SCAN CARD","First Admin Card");
	setaddress();
	while (1)
		{
			if (TM_MFRC522_Check(CardID) == MI_OK)
			{
				if (CheckListUID(CardID) == 0)
				{
					CardID[5] = 0x81;
					Flash_Write_Array(AddressUID, CardID, 6);
					AddressUID += 8;
					break;
				}
				else
				{
					CLCD_I2C_Display(&LCD1, "    WARNING!", "Try another card");
					buzzer(5);
					HAL_Delay(1000);
					CLCD_I2C_Display(&LCD1, "PLS SCAN CARD","First Admin Card");
				}
			}
		}
	CLCD_I2C_Display(&LCD1, "ADD SUCCESSFUL","Admin Card 1");
	buzzer(1);
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
	Flash_Erase(pt);
	set_default_password();
	remove_all_finger();
	CLCD_I2C_Display(&LCD1, " REMOVE ALL CARD", "  SUCCESSFULLY");
	HAL_Delay(1000);
}

void addface(uint8_t key)
{
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
    snprintf(Tx_Buffer, sizeof(Tx_Buffer), "Add.%d", key);
    CDC_Transmit_FS(Tx_Buffer, strlen(Tx_Buffer));
    CLCD_I2C_Display(&LCD1, "WAITING....", "");
    exitmenu = 60;
    while(exitmenu != 0){
        if(Rx_Buffer[0] == 'T'){
            CLCD_I2C_Display(&LCD1, "   ADD FACEID", "   SUCCESSFUL");
            buzzer(1);
            HAL_Delay(2000);
            break;
        }
        else if(Rx_Buffer[0] == 'F'){
            CLCD_I2C_Display(&LCD1, "ERROR: UNKNOWN", "");
            buzzer(5);
            HAL_Delay(2000);
            break;
        }
    }
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}

void removeface(uint8_t key)
{
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
    snprintf(Tx_Buffer, sizeof(Tx_Buffer), "Rem.%d", key);
    CDC_Transmit_FS(Tx_Buffer, strlen(Tx_Buffer));
    CLCD_I2C_Display(&LCD1, "WAITING....", "");
    exitmenu = 60;
    while(exitmenu != 0){
        if(Rx_Buffer[0] == 'T'){
            CLCD_I2C_Display(&LCD1, "XOA THANH CONG", "");
            buzzer(1);
            HAL_Delay(2000);
            break;
        }
        else if(Rx_Buffer[0] == 'F'){
            CLCD_I2C_Display(&LCD1, "ERROR: UNKOWN", "");
            buzzer(5);
            HAL_Delay(2000);
            break;
        }
    }
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}

uint8_t checkfaceid(uint8_t key){
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
    snprintf(Tx_Buffer, sizeof(Tx_Buffer), "Che.%d", key);
    CDC_Transmit_FS(Tx_Buffer, strlen(Tx_Buffer));
    while(Rx_Buffer[0] == 0){
        continue;
    }
    HAL_Delay(1000);
    if(Rx_Buffer[0] == 'T'){
        return key;
    } else if(Rx_Buffer[0] == 'F'){
        return 0;
    }
    memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
    return 0;
}
//---------- them van tay---------------
void add_finger()
{
    ID = InputID_FINGER();
    if(CheckKeyFinger(ID)!=0)
    {
		CLCD_I2C_Display(&LCD1," ID is existing"," Pick another ID");
		buzzer(3);
		HAL_Delay(1000);
    }
    else{
        uint32_t start_time_finger = HAL_GetTick();
        while (1)
        {
            if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
            {
                CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                buzzer(5);
                HAL_Delay(2000);
                return;
            }

            collect_finger();
            CLCD_I2C_Display(&LCD1, "Add Finger Print", "Put your finger!!     ");
            HAL_Delay(1000);

            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Reading finger...!!     ");
            tmp=0xff;
            while(tmp!=0x00){
                collect_finger();
                collect_finger();
                tmp= collect_finger();
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            tmp=0xff;
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Remove Finger!!   ");HAL_Delay(100);
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Processing Finger!!   ");
            tmp=0xff;
            while(tmp!=0x00)    {
                tmp=img2tz(0x01);
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"put finger again");HAL_Delay(100);
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Reading finger...!!     ");
            tmp=0xff;
            while(tmp!=0x00)    {
                collect_finger();
                collect_finger();
                tmp=collect_finger();
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Remove Finger!!   ");HAL_Delay(100);
            tmp=0xff;
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"Processing Finger!!   ");
            while(tmp!=0x00)    {
                tmp=img2tz(0x02);
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            tmp=0xff;
            while(tmp!=0x00)
            {
                tmp=match();
                if (tmp==0x08 || tmp==0x01)
                {
                    CLCD_I2C_SetCursor(&LCD1, 0, 1);
                    CLCD_I2C_WriteString(&LCD1," ER: try again!");buzzer(5);HAL_Delay(1500);
                    return;
                }
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            tmp=0xff;
            while(tmp!=0x00){
                tmp=regmodel();
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            tmp=0xff;
            while(tmp!=0x00){
                tmp=store(ID);
                if (HAL_GetTick() - start_time_finger > 20000) // 15 seconds timeout
                {
                    CLCD_I2C_Display(&LCD1, "TIMEOUT", "Try again");
                    buzzer(5);
                    HAL_Delay(2000);
                    exitmenu = Delaymenu;
                    return;
                }
            }
            CLCD_I2C_SetCursor(&LCD1, 0, 1);
            CLCD_I2C_WriteString(&LCD1,"  Save Finger!    ");
            buzzer(1);
        	AddFingerID(ID);
            HAL_Delay(1500);
            CLCD_I2C_Clear(&LCD1);
            break;
        }
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
		CLCD_I2C_Display(&LCD1, "    WELCOME", " Finger");
		sprintf(mess,"-ID = %d  ", pID); // Use %d for integer
		CLCD_I2C_WriteString(&LCD1,mess);
		opendoor();
		CLCD_I2C_Clear(&LCD1);
	}
	if(tmp==0x09)	// khong co van tay
	{
		tmp=0xff;
		CLCD_I2C_SetCursor(&LCD1, 0, 1);
		CLCD_I2C_WriteString(&LCD1,"Wrong Fingerprint"); buzzer(5);HAL_Delay(1000);
		CLCD_I2C_WriteString(&LCD1,mess);
		HAL_Delay(1000);
		CLCD_I2C_Clear(&LCD1);
	}
}
void remove_id_finger()
{
    ID = InputID_FINGER();
    if(CheckKeyFinger(ID)==0)
    {
		CLCD_I2C_Display(&LCD1,"ID doesnt exist"," Pick another ID");
		buzzer(3);
		HAL_Delay(1000);
    }
    else{
		CLCD_I2C_Display(&LCD1, "Removing Finger", "");
		uint8_t result = delete_id_finger(ID);
		if (result == 0x00)
		{
			CLCD_I2C_Display(&LCD1, "Remove Finger", "Successfully");
			buzzer(1);
	    	RemoveFingerID(CheckKeyFinger(ID));
			// Ensure the fingerprint is removed from memory
			fingerprint_detected = 0;
		}
		else
		{
			char buffer[16];
			snprintf(buffer, sizeof(buffer), "Error Code: %02X", result);
			buzzer(5);
			CLCD_I2C_Display(&LCD1, "Remove Finger", buffer);
		}
		HAL_Delay(2000);
		CLCD_I2C_Clear(&LCD1);
    }
}

void remove_all_finger()
{
    CLCD_I2C_Display(&LCD1, "  RM ALL FINGER", "  Processing...");
    uint8_t result = empty();
    if (result == 0x00)
    {
        CLCD_I2C_Display(&LCD1, "   REMOVE ALL", "  SUCCESSFULLY");
        buzzer(1);
    	Flash_Erase(StartAddressFingerID);
        // Ensure all fingerprints are removed from memory
        fingerprint_detected = 0;
    }
    else
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Error Code: %02X", result);
        buzzer(5);
        CLCD_I2C_Display(&LCD1, "Remove Finger", buffer);
    }
    HAL_Delay(2000);
    CLCD_I2C_Clear(&LCD1);
}
uint32_t CheckKeyFinger(uint8_t key)
{
    uint32_t pt = StartAddressFingerID;
    while (Flash_Read_2Byte(pt) != 0xFFFF)
    {
        if (*(uint8_t *)(pt) == key)
            return pt;
        pt = pt + 2;
	    }
    return 0;
}
void AddFingerID(uint8_t key)
{
	uint32_t pt = StartAddressFingerID;
	while (Flash_Read_2Byte(pt) != 0xFFFF)
	{
		pt = pt + 2;
	}
	FingerID[0] = key;
	Flash_Write_Array(pt, FingerID, 1);
}

void RemoveFingerID(uint32_t addressrm)
{
	Flash_Write_2Byte(addressrm, 0xFFFF);
}


void startface(void)
{
	if(Rx_Buffer[0] == 'Y'){
        CLCD_I2C_Clear(&LCD1);
        CLCD_I2C_SetCursor(&LCD1, 0, 0);
        CLCD_I2C_WriteString(&LCD1, "    WELCOME");
        opendoor();
//        HAL_Delay(2000);
	}else if(Rx_Buffer[0] == 'N'){
		CLCD_I2C_Display(&LCD1, "  WRONG FACEID", "CAN NOT ACCESS");
		buzzer(5);
        HAL_Delay(2000);
	}
	memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
}

void enter_password(char *password) {
    for (int i = 0; i < 6; i++) {
        char key;
        do {
            key = KeyPad_WaitForKeyGetChar(10);
        } while (key == 0 || (key < '0' || key > '9')); // Only accept numeric keys
        buzzer(1);
        password[i] = key;
        CLCD_I2C_WriteChar(&LCD1, '*');
    }
    password[6] = '\0';
}

void change_password(void) {
    char new_password[7] = {0};
    CLCD_I2C_Display(&LCD1, " ENTER NEW PASS", "     ");
    enter_password(new_password);
    // Erase the flash memory at the password address before writing the new password
    Flash_Erase(StartAddressPassword);
    Flash_Write_Array(StartAddressPassword, (uint8_t *)new_password, 6);
    CLCD_I2C_Display(&LCD1, "PASSWORD CHANGED", "  SUCCESSFULLY");
    buzzer(1);
    HAL_Delay(2000);
}

uint8_t check_password(char *password) {
    char stored_password[7] = {0};
    Flash_Read_Array(StartAddressPassword, (uint8_t *)stored_password, 6);
    stored_password[6] = '\0';
    return strcmp(password, stored_password) == 0;
}

void set_default_password(void) {
    char default_password[6] = "111111";
    // Erase the flash memory at the password address before writing the default password
    Flash_Erase(StartAddressPassword);
    Flash_Write_Array(StartAddressPassword, (uint8_t *)default_password, 6);
    CLCD_I2C_Display(&LCD1, " RESET PASSWORD", "  SUCCESSFULLY");
    buzzer(1);
    HAL_Delay(1500);
    CLCD_I2C_Display(&LCD1, "  NEW PASSWORD:", "     111111");
    HAL_Delay(1000);
    exitmenu=0;
}

void opendoor(void)
{
    buzzer(1);
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 1);
    HAL_Delay(1500);
    uint32_t door_open_time = HAL_GetTick();
    while ((HAL_GetTick() - door_open_time) < opendoortime)
    {
        uint32_t remaining_time = (opendoortime - (HAL_GetTick() - door_open_time) + 999) / 1000; // Adjust to include 2s
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "       %lus", remaining_time);
        CLCD_I2C_Display(&LCD1, " DOOR IS OPENING", buffer);
        HAL_Delay(1000);
    }
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 0);
    CLCD_I2C_Clear(&LCD1);
}
void buzzer( uint8_t countbeep)
{

	while(countbeep--)
	{
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin,0);
		HAL_Delay(120);
		HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin,1);
		HAL_Delay(50);
	}
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
