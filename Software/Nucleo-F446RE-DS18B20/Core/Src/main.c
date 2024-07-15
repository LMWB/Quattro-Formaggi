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
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ds18b20.h"
#include "app_can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_CAN2_Init();
  MX_I2C1_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  /* CAN Stuff */
  app_can_set_msg_filter();

  app_can_start();

  HAL_Delay(100);
  if( app_can_send_test_msg() != 0){
    my_printf("Sending Test CAN Msg Failed\n");
  }
  /* CAN Stuff End */


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  app_can_send_test_msg();

  HAL_TIM_Base_Start_IT(&htim7);

  HAL_Delay(300);
  my_printf("DS18B20 One Wire Temperature Sensor Demo\n");

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* two ore more 1Wire-Device on the bus! */
    ds18b20_demo_multi();

    /* one single 1Wire-Device on the bus only! */
    //ds18b20_demo();

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
{
  my_printf(" CAN fifo full \n");
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
  uint32_t error = hcan->ErrorCode; /*!< This parameter can be a value of @ref CAN_Error_Code */

  my_printf("CAN Error %d\n", error);

}

void HAL_CAN_RxFifo0MsgPendingCallback( CAN_HandleTypeDef *hcan ) {
  CAN_RxHeaderTypeDef sRxHeader;
  HAL_StatusTypeDef TX_RX_status;
  uint32_t fifoFillLevel;
  uint8_t RxData[8];
  uint8_t canHW = 0;
  uint32_t canID = 0;

  if ( hcan->Instance == CAN2 ) {
    fifoFillLevel = HAL_CAN_GetRxFifoFillLevel( &hcan2, CAN_RX_FIFO0 );
    TX_RX_status = HAL_CAN_GetRxMessage( &hcan2, CAN_RX_FIFO0, &sRxHeader, RxData );
    canHW = 2;
  }

  if( CAN_ID_STD == sRxHeader.IDE )
  {
    canID = sRxHeader.StdId;
  }

  if( CAN_ID_EXT == sRxHeader.IDE )
  {
    canID = sRxHeader.ExtId;
  }

  char buf[128];
  uint16_t s = 0;
  s = sprintf(buf, "Rec from CAN %d with ID 0x%lX Msg: %d %d %d %d %d %d %d %d\n", canHW, canID,
      RxData[0],RxData[1],RxData[2],RxData[3],
      RxData[4],RxData[5],RxData[6],RxData[7]);
  HAL_UART_Transmit( &huart2, (uint8_t*)buf, s, 100 );

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

  if(htim->Instance == TIM7){
    app_can_send_test_msg();
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
