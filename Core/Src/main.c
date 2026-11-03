/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Pomodoro Punish Timer
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <string.h>
#include <stdio.h>

/* USER CODE BEGIN PTD */
typedef enum {
    STATE_IDLE,
    STATE_FOCUS,
    STATE_REST,
    STATE_PAUSE,
    STATE_TEST,
    STATE_PUNISH_WAIT,
    STATE_PUNISH
} SystemState;
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define RX_BUFFER_SIZE 20
/* USER CODE END PD */

UART_HandleTypeDef hlpuart1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
SystemState currentState = STATE_IDLE;
SystemState previousState = STATE_IDLE;

uint8_t rx_data;
char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;

uint32_t timer_seconds = 0;
int16_t pwm_value = 0;
int16_t pwm_step = 50;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_LPUART1_UART_Init(void);

/* USER CODE BEGIN PFP */
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim); // 補齊遺失的硬體宣告
/* USER CODE END PFP */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_LPUART1_UART_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim3);

  HAL_UART_Receive_IT(&hlpuart1, &rx_data, 1);

  char *start_msg = "\r\n--- STM32 Pomodoro Ready ---\r\nType 'focus', 'rest', 'test' or 'pause':\r\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)start_msg, strlen(start_msg), 1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (currentState == STATE_FOCUS || currentState == STATE_TEST) {
          // 平滑呼吸燈
          pwm_value += pwm_step;
          if (pwm_value >= 10000) { pwm_value = 10000; pwm_step = -50; }
          else if (pwm_value <= 0) { pwm_value = 0; pwm_step = 50; }

          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_value);
          HAL_Delay(15);
      }
      else if (currentState == STATE_REST) {
          // 輕快雙閃爍
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000);
          HAL_Delay(100);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
          HAL_Delay(100);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000);
          HAL_Delay(100);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
          HAL_Delay(700);
      }
      else if (currentState == STATE_PAUSE) {
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1000);
          HAL_Delay(100);
      }
      else if (currentState == STATE_PUNISH_WAIT) {
          // 等待懲罰：每秒短響
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 500);
          HAL_Delay(100);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
          HAL_Delay(900);
      }
      else if (currentState == STATE_PUNISH) {
          // 懲罰模式：快速爆閃與長鳴
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 500);
          HAL_Delay(50);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
          HAL_Delay(50);
      }
      else {
          // 待機熄滅
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
          HAL_Delay(100);
      }
    /* USER CODE END WHILE */
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_LPUART1_UART_Init(void)
{
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 16-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16000-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM4_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 16-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim4);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == LPUART1) {
        if (rx_data == '\r' || rx_data == '\n') {
            rx_buffer[rx_index] = '\0';
            char msg[60];

            if (strstr(rx_buffer, "focus") != NULL) {
                currentState = STATE_FOCUS;
                timer_seconds = 25 * 60;
                sprintf(msg, "\r\n[SYS] Mode: FOCUS (25 min started)\r\n");
            } else if (strstr(rx_buffer, "rest") != NULL) {
                currentState = STATE_REST;
                timer_seconds = 5 * 60;
                sprintf(msg, "\r\n[SYS] Mode: REST (5 min started)\r\n");
            } else if (strstr(rx_buffer, "idle") != NULL) {
                currentState = STATE_IDLE;
                sprintf(msg, "\r\n[SYS] Mode: IDLE (Timer stopped)\r\n");
            } else if (strstr(rx_buffer, "pause") != NULL) {
                if (currentState != STATE_PAUSE) {
                    previousState = currentState;
                    currentState = STATE_PAUSE;
                    sprintf(msg, "\r\n[SYS] Mode: PAUSED\r\n");
                } else {
                    currentState = previousState;
                    sprintf(msg, "\r\n[SYS] Mode: RESUMED\r\n");
                }
            } else if (strstr(rx_buffer, "test") != NULL) {
                currentState = STATE_TEST;
                timer_seconds = 10;
                sprintf(msg, "\r\n[SYS] Mode: TEST (10 sec countdown)\r\n");
            } else if (rx_index > 0) {
                sprintf(msg, "\r\n[ERR] Unknown command: %s\r\n", rx_buffer);
            }

            if (rx_index > 0) {
                HAL_UART_Transmit(&hlpuart1, (uint8_t *)msg, strlen(msg), 100);
            }

            rx_index = 0;
            memset(rx_buffer, 0, RX_BUFFER_SIZE);
        } else {
            if (rx_index < RX_BUFFER_SIZE - 1) {
                rx_buffer[rx_index++] = rx_data;
            }
        }
        HAL_UART_Receive_IT(&hlpuart1, &rx_data, 1);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        if (currentState != STATE_IDLE && currentState != STATE_PAUSE && currentState != STATE_PUNISH) {
            if (timer_seconds > 0) {
                timer_seconds--;
            } else {
                if (currentState == STATE_FOCUS) {
                    currentState = STATE_REST;
                    timer_seconds = 5 * 60;
                } else if (currentState == STATE_REST) {
                    currentState = STATE_PUNISH_WAIT;
                    timer_seconds = 5;
                } else if (currentState == STATE_TEST) {
                    currentState = STATE_PUNISH_WAIT;
                    timer_seconds = 5;
                } else if (currentState == STATE_PUNISH_WAIT) {
                    currentState = STATE_PUNISH;
                }
            }
        }
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 紀錄上一次按下的時間，用來做軟體防彈跳
    static uint32_t last_btn_tick = 0;

    // 確認觸發中斷的是否為藍色按鈕 (PC13)
    if (GPIO_Pin == GPIO_PIN_13) {
        uint32_t current_tick = HAL_GetTick();

        // 確保兩次按壓間隔超過 200 毫秒 (防彈跳)
        if (current_tick - last_btn_tick > 200) {

            // 只有在「懲罰中」或「等待懲罰」時按鈕才有效
            if (currentState == STATE_PUNISH || currentState == STATE_PUNISH_WAIT) {

                currentState = STATE_FOCUS; // 切換為專注模式
                timer_seconds = 25 * 60;    // 重置倒數 25 分鐘

                // 傳送訊息到 PuTTY 終端機顯示
                char *msg = "\r\n[SYS] Button Pressed! Mode: FOCUS (25 min started)\r\n";
                HAL_UART_Transmit(&hlpuart1, (uint8_t *)msg, strlen(msg), 100);
            }
            last_btn_tick = current_tick;
        }
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
