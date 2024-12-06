/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


 #define FP_OK 0x00
 #define FP_ERROR 0xFE
 #define FP_NOFINGER 0x02
 #define FP_FINGER_NOTMATCH 0x0A
 #define FP_FINGER_NOTMATCH 0x0A
 #define FP_FINGER_NOTFOUND 0x09


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */


uint8_t FPHeader[6]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF};
	 uint8_t FPGetImage[6]={0x01,0x00,0x03,0x01,0x00,0x05};
	 uint8_t FPCreateCharFile1[7]={0x01,0x00,0x04,0x02,0x01,0x00,0x08};
	 uint8_t FPCreateCharFile2[7]={0x01,0x00,0x04,0x02,0x02,0x00,0x09};
	 uint8_t FPCreateTemplate[6]={0x01,0x00,0x03,0x05,0x00,0x09};
	 uint8_t FPDeleteAllFinger[6]={0x01,0x00,0x03,0x0D,0x00,0x11};
uint8_t FPSearchFinger[11]={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x40,0x00,0x4E};
	 uint8_t FPGetNumberOfFinger[6]={0x01,0x00,0x03,0x1D,0x00,0x21};


uint8_t IDFromFinger;
uint8_t CurrentNumberFinger;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void SendFPHeader()
{
	HAL_UART_Transmit(&huart5,FPHeader,6,1000);
}

void SendFPGetImage()
{
	 HAL_UART_Transmit(&huart5,FPGetImage,6,1000);
}

void SendFPCreateCharFile1()
{
	HAL_UART_Transmit(&huart5,FPCreateCharFile1,7,1000);
}

void SendFPCreateCharFile2()
{
	HAL_UART_Transmit(&huart5,FPCreateCharFile2,7,1000);
}

void SendFPCreateTemplate()
{
	HAL_UART_Transmit(&huart5,FPCreateTemplate,6,1000);
}

void SendFPDeleteAllFinger()
{
	HAL_UART_Transmit(&huart5,FPDeleteAllFinger,6,1000);
}

void SendFPDSearchFinger()
{
	HAL_UART_Transmit(&huart5,FPSearchFinger,11,1000);
}

void SendFGetNumberOfFinger()
{
	HAL_UART_Transmit(&huart5,FPGetNumberOfFinger,6,1000);
}

void SendStoreFinger(uint16_t IDStore)
{
	uint16_t Sum=0;
	uint8_t DataSend[9]={0};

	DataSend[0]=0x01;
	Sum=Sum+DataSend[0];
	DataSend[1]=0x00;
	Sum=Sum+DataSend[1];
	DataSend[2]=0x06;
	Sum=Sum+DataSend[2];
	DataSend[3]=0x06;
	Sum=Sum+DataSend[3];
	DataSend[4]=0x01;
	Sum=Sum+DataSend[4];
	DataSend[5]= (uint8_t)(IDStore>> 8);
	Sum=Sum+DataSend[5];
	DataSend[6]=(uint8_t) (IDStore&0xFF);
	Sum=Sum+DataSend[6];
  DataSend[7]=(uint8_t)(Sum>> 8);
	DataSend[8]=(uint8_t)(Sum&0xFF);
	HAL_UART_Transmit(&huart5,DataSend,9,1000);
}

void SendDeleteFinger(uint16_t IDDelete)
{
	uint16_t Sum=0;
	uint8_t DataSend[10]={0};

	DataSend[0]=0x01;
	Sum=Sum+DataSend[0];
	DataSend[1]=0x00;
	Sum=Sum+DataSend[1];
	DataSend[2]=0x07;
	Sum=Sum+DataSend[2];
	DataSend[3]=0x0C;
	Sum=Sum+DataSend[3];
	DataSend[4]=(uint8_t)(IDDelete>> 8);
	Sum=Sum+DataSend[4];
	DataSend[5]= (uint8_t) (IDDelete&0xFF);
	Sum=Sum+DataSend[5];
	DataSend[6]=0x00;
	Sum=Sum+DataSend[6];
	DataSend[7]=0x001;
	Sum=Sum+DataSend[7];
  DataSend[8]=(uint8_t)(Sum>> 8);
	DataSend[9]=(uint8_t)(Sum&0xFF);
	HAL_UART_Transmit(&huart5,DataSend,10,1000);
}




uint8_t CheckFPRespsone(uint8_t MaxRead)
{
	uint8_t ByteCount=0;
	uint8_t FPRXData[20]={0xFF};
	uint8_t UARTData[1]={0};
	uint32_t TimeOut = HAL_GetTick();
	uint8_t Result;
	IDFromFinger=0xFF;
	while((HAL_GetTick() - TimeOut < 1000) && ByteCount<MaxRead) // time out is 1000 ms
	{
	    if(HAL_UART_Receive(&huart5, (uint8_t *)UARTData, 1, 1000) == HAL_OK)
			{
				FPRXData[ByteCount] = UARTData[0];
				ByteCount++;
			}
	}

	if(ByteCount==0)
	{
		Result=FP_ERROR;
		return Result;
	}
	else if(ByteCount<MaxRead)
	{
		Result=FP_ERROR;
		return Result;
	}
	else // valid data return
	{
		Result=FPRXData[9];
		IDFromFinger=FPRXData[11];
		return Result;
	}
}

uint8_t GetNumberOfFinger()
{
	uint8_t Result;
	SendFPHeader();
	SendFGetNumberOfFinger();
	Result=CheckFPRespsone(14);
	if(Result!=FP_OK) return 0xFF;

	return IDFromFinger;
}





uint8_t RegistryNewFinger(uint16_t LocationID)
{

	uint8_t Result=FP_NOFINGER;
	uint32_t TimeOut = HAL_GetTick();


	LCD_SetPos(0,1);
	LCD_String("AS608 STM32");

	while(Result==FP_NOFINGER&&(HAL_GetTick() - TimeOut < 5000)) // time out is 5000 ms
	{

		SendFPHeader();
		SendFPGetImage();
		Result=CheckFPRespsone(12);
	}
	if(Result!=FP_OK) return FP_ERROR;
	// continue if detect finger;
	SendFPHeader();
	SendFPCreateCharFile1();
	Result=CheckFPRespsone(12);
	if(Result!=FP_OK) return FP_ERROR;

	LCD_SetPos(0,1);
	LCD_String(" Or hands out  ");

	HAL_Delay(2000);
	Result=FP_NOFINGER;
	TimeOut = HAL_GetTick();
	LCD_SetPos(0,1);
	LCD_String("Delicate hand");

	while(Result==FP_NOFINGER&&(HAL_GetTick() - TimeOut < 5000)) // time out is 5000 ms
	{

		SendFPHeader();
		SendFPGetImage();
		Result=CheckFPRespsone(12);
	}
	if(Result!=FP_OK) return FP_ERROR;

	// continue if detect finger;
	SendFPHeader();
	SendFPCreateCharFile2();
	Result=CheckFPRespsone(12);
	if(Result!=FP_OK) return FP_ERROR;

	// Compare finger, create template
	SendFPHeader();
	SendFPCreateTemplate();
	Result=CheckFPRespsone(12);
	if(Result==FP_FINGER_NOTMATCH)
	{

		return FP_FINGER_NOTMATCH;
	}
	else if(Result!=FP_OK) return FP_ERROR;

	// save finger
	SendFPHeader();
	SendStoreFinger(LocationID);
	Result=CheckFPRespsone(12);
	if(Result!=FP_OK) return FP_ERROR;
	else
	{
		return FP_OK;
	}

}

uint8_t CheckFinger()
{
	uint8_t Result=FP_NOFINGER;
	uint32_t TimeOut = HAL_GetTick();





	while(Result==FP_NOFINGER&&(HAL_GetTick() - TimeOut < 5000)&&HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==0) // time out is 5000 ms and no button press
	{

		SendFPHeader();
		SendFPGetImage();
		Result=CheckFPRespsone(12);
	}
	if(Result==FP_NOFINGER)
	{

		return FP_NOFINGER;
	}
	if(Result!=FP_OK) return FP_ERROR;
	// continue if detect finger;
	SendFPHeader();
	SendFPCreateCharFile1();
	Result=CheckFPRespsone(12);
	if(Result!=FP_OK) return FP_ERROR;

	// Search Fingger
	SendFPHeader();
	SendFPDSearchFinger();
	Result=CheckFPRespsone(16);
	return Result;


}


uint8_t ProcessRegistryNewFinger()
{
  uint8_t FingerResult;
  LCD_SetPos(0,1);
	LCD_String("Save Finger");

	HAL_Delay(1000);
  FingerResult=RegistryNewFinger(CurrentNumberFinger+1);
	if(FingerResult==FP_OK)
	{
		LCD_SetPos(0,1);
	  LCD_String("Finger saved");
		HAL_Delay(1000);
		LCD_SetPos(0,1);
		LCD_String("                ");
		CurrentNumberFinger++;
		if(CurrentNumberFinger==100) CurrentNumberFinger=1;
		HAL_Delay(1500);
	}
	else if(FingerResult==FP_FINGER_NOTMATCH)
	{
		LCD_SetPos(0,1);
		LCD_String("Finger n save");
		HAL_Delay(1000);
		LCD_SetPos(0,1);
		LCD_String("                ");
	}
	else
	{
		LCD_SetPos(0,1);
		LCD_String(" ERROR    ");
		HAL_Delay(1000);
		LCD_SetPos(0,1);
		LCD_String("                ");

	}
	return FingerResult;

}

void DeleteAllFinger()
{

	uint8_t FingerResult;
	SendFPHeader();
	SendFPDeleteAllFinger();
	FingerResult=CheckFPRespsone(12);
	if(FingerResult==FP_OK)
	{
		LCD_SetPos(0,1);
		LCD_String("All FING DEL");
		HAL_Delay(1500);
		LCD_SetPos(0,1);
		LCD_String("                ");
		CurrentNumberFinger=0;
	}
	else
	{
		LCD_SetPos(0,1);
		LCD_String("FINGER N DEL");
		HAL_Delay(1500);
		LCD_SetPos(0,1);
		LCD_String("                ");
	}

}


void Delay_us(uint32_t TimeDelay)
{
	uint32_t TimeCount;
	for(TimeCount=0;TimeCount<TimeDelay;TimeCount++)
	{
		__ASM volatile ("NOP");
		__ASM volatile ("NOP");
		__ASM volatile ("NOP");
	}
}

void CloseDoor()
{
	  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
	  Delay_us(1000);
	  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);
}

void OpenDoor()
{
		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
	  Delay_us(2100);
	  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


	  uint8_t FingerResult;
		 uint32_t TimeCount;


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
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
	 LCD_SetPos(0,0);
	 LCD_String("AS608 STM32");


	 HAL_Delay(1000);

		OpenDoor(); // need to active motor
		HAL_Delay(500);
		CloseDoor();
		HAL_Delay(500);
		OpenDoor();
		HAL_Delay(500);
		CloseDoor();
		CurrentNumberFinger=GetNumberOfFinger();
		if(CurrentNumberFinger>100)
		{
			CurrentNumberFinger=1;
		}

//	 LCD_SetPos(0,1);
//	 LCD_String("   SANG SANG    ");

	 ProcessRegistryNewFinger();
//	 SendStoreFinger(1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		FingerResult=CheckFinger();
	  if(FingerResult==FP_OK)
		{

			OpenDoor();
			LCD_SetPos(0,1);
	    LCD_String("   OpenDoor  ");

			HAL_Delay(3000);
			CloseDoor();
			LCD_SetPos(0,1);
			LCD_String("               ");
		}
		else if(FingerResult==FP_FINGER_NOTFOUND)
		{
			LCD_SetPos(0,1);
	    LCD_String("   CloseDoor ");
			HAL_Delay(1000);
			LCD_SetPos(0,1);
			LCD_String("               ");
		}

		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==1)
		{
			TimeCount = HAL_GetTick();
			while(HAL_GetTick()-TimeCount<3000&&HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==1) // check hold button in 3 second to erase
			{

			}
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==1) // still hold button
			{
				DeleteAllFinger();
			}
			else
		  {
				FingerResult=ProcessRegistryNewFinger();
			}

		}
		HAL_Delay(100);
    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 57600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB12 PB13
                           PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
