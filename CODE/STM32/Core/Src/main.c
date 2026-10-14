
#include "main.h"


#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include <stdlib.h>

#include "TFT_ILI9341.h"
#include "DHT.h"
#include "zigbee.h"
#include "zbnode_ui.h"


SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

DMA_HandleTypeDef hdma_usart3_tx;


void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);


uint8_t uart3_rx_byte;
char uart3_rx_buffer[128];
uint8_t uart3_rx_index = 0;

#define MAX_MY_NODES 4

DHT11_InitTypeDef dht;
DHT11_StatusTypeDef err;

ZbNode_t my_nodes[MAX_MY_NODES];
ZbNode_t *node_ptrs[MAX_MY_NODES];

uint8_t active_nodes = 0;

uint32_t last_ui_update = 0;
uint32_t last_dht_read = 0;
uint32_t last_uart_send = 0;

bool need_redraw = false;



uint8_t uart3_txbuf[256];
volatile uint8_t uart3_busy = 0;



int Update_Or_Add_Node(ZbNode_t *new_data)
{
    for(int i = 0; i < active_nodes; i++)
    {
        if(my_nodes[i].addr == new_data->addr)
        {
            my_nodes[i] = *new_data;
            my_nodes[i].online = true;

            return i;
        }
    }

    if(active_nodes < MAX_MY_NODES)
    {
        my_nodes[active_nodes] = *new_data;

        my_nodes[active_nodes].online = true;

        active_nodes++;

        UI_Init(node_ptrs, active_nodes);

        return active_nodes - 1;
    }

    return -1;
}



void UART3_Send(char *str)
{
    if(uart3_busy)
        return;

    uart3_busy = 1;

    strcpy((char*)uart3_txbuf, str);

    HAL_UART_Transmit_DMA(
        &huart3,
        uart3_txbuf,
        strlen((char*)uart3_txbuf)
    );
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        uart3_busy = 0;
    }
}



void ESP32_Send_Node(ZbNode_t *n)
{
    char txBuf[256];

    float current = 0;
    float voltage = 0;
    float vib = 0;

    
    if(n->valid.voltage_ep2)
        current = n->voltage_ep2;

    
    if(n->valid.voltage_ep3)
        voltage = n->voltage_ep3;

    
    if(n->valid.voltage_ep4)
        vib = n->voltage_ep4;

    snprintf(txBuf,
             sizeof(txBuf),

             "{\"node_id\":\"%04X\","
             "\"temp\":%.1f,"
             "\"current\":%.2f,"
             "\"vib_value\":%.2f,"
             "\"vol\":%.2f,"
             "\"lqi\":%d}\n",

             n->addr,
             n->temperature,
             current,
             vib,
             voltage,
             n->lqi);

    UART3_Send(txBuf);
}



void ESP32_Send_Gateway_Temp(float temp)
{
    char txBuf[64];

    snprintf(txBuf,
             sizeof(txBuf),

             "{\"node_id\":\"F841\","
             "\"temp\":%.1f}\n",

             temp);

    UART3_Send(txBuf);
}



int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();

    MX_SPI1_Init();
    MX_TIM1_Init();

    MX_USART1_UART_Init();
    MX_USART3_UART_Init();

    

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

    TFT_Init();

    ZB_Begin();
	HAL_TIM_Base_Start(&htim1);
    HAL_DHT11_Init(&dht,
                   GPIOA,
                   GPIO_PIN_8,
                   &htim1);
									 
			HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1);

    

    for(int i = 0; i < MAX_MY_NODES; i++)
    {
        node_ptrs[i] = &my_nodes[i];

        memset(&my_nodes[i],
               0,
               sizeof(ZbNode_t));
    }

    UI_Init(node_ptrs, active_nodes);

    UI_Render();

    

    while(1)
    {
        ZB_Poll();

        ZB_Tick();
			UI_Alarm_Tick();

        ZbNode_t temp_node;

        

      if(ZB_Read(&temp_node))
        {
            Update_Or_Add_Node(&temp_node);

            if(HAL_GetTick() - last_uart_send >= 100)
            {
                last_uart_send = HAL_GetTick();

                ESP32_Send_Node(&temp_node);
            }

            
            
            extern uint8_t ui_view;      
            extern uint8_t ui_selIdx;    
            extern ZbNode_t *ui_nodes[]; 

            
            if (ui_view == 0) 
            {
                
                need_redraw = true; 
            } 
            else 
            {
                
                if (temp_node.addr == ui_nodes[ui_selIdx]->addr) 
                {
                    need_redraw = true; 
                }
            }
            
        }

        

        if(HAL_GetTick() - last_dht_read >= 5000)
        {
            last_dht_read = HAL_GetTick();

            HAL_DHT11_ReadData(&dht);

            ESP32_Send_Gateway_Temp(dht.Temperature);
					need_redraw = true;
        }

        

        if (HAL_GetTick() - last_ui_update > 1500)
        {
            
            if (need_redraw)
            {
                UI_Update(); 
                
                need_redraw = false; 
            }
            
            
            last_ui_update = HAL_GetTick();
        }
    }
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_button_press = 0;

    uint32_t current_time = HAL_GetTick();

    if(current_time - last_button_press > 400)
    {
        if(GPIO_Pin == GPIO_PIN_0)
        {
            UI_Button(0);
            need_redraw = true;
        }

        else if(GPIO_Pin == GPIO_PIN_1)
        {
            UI_Button(1);
            need_redraw = true;
        }

        else if(GPIO_Pin == GPIO_PIN_2)
        {
            UI_Button(2);
            need_redraw = true;
        }

        else if(GPIO_Pin == GPIO_PIN_3)
        {
            UI_Button(3);
            need_redraw = true;
        }

        last_button_press = current_time;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    
    if(huart->Instance == USART3)
    {
        
        if (uart3_rx_index < sizeof(uart3_rx_buffer) - 1) {
            uart3_rx_buffer[uart3_rx_index++] = uart3_rx_byte;
        }
        
        
        if (uart3_rx_byte == '\n' || uart3_rx_byte == '\r') {
            uart3_rx_buffer[uart3_rx_index] = '\0'; 
					
					
					if (strstr(uart3_rx_buffer, "\"type\":\"SET_TH\"") != NULL) {
                int t_th = 70;
                int c_th = 10;
                uint16_t target_addr = 0;
                
                
                char *pNode = strstr(uart3_rx_buffer, "\"node_id\":\"");
                if (pNode != NULL) {
                    
                    
                    target_addr = (uint16_t)strtol(pNode + 11, NULL, 16);
                }
                
                
                char *pTemp = strstr(uart3_rx_buffer, "\"temp_th\":");
                if (pTemp != NULL) t_th = atoi(pTemp + 10);
                
                
                char *pCurr = strstr(uart3_rx_buffer, "\"current_th\":");
                if (pCurr != NULL) c_th = atoi(pCurr + 13);
                
                
                if (target_addr != 0) {
                    UI_SetNodeThreshold(target_addr, (int16_t)t_th, (int16_t)c_th);
                }
            }
					
            
            
            if (strstr(uart3_rx_buffer, "Relay 1: ON") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, 1);
            } 
            else if (strstr(uart3_rx_buffer, "Relay 1: OFF") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, 0);
            }
            else if (strstr(uart3_rx_buffer, "Relay 2: ON") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, 1); 
            }
						else if (strstr(uart3_rx_buffer, "Relay 2: OFF") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, 0);
            }
						else if (strstr(uart3_rx_buffer, "Relay 3: ON") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1); 
            }
						else if (strstr(uart3_rx_buffer, "Relay 3: OFF") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0); 
            }
						else if (strstr(uart3_rx_buffer, "Relay 4: ON") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1); 
            }
						else if (strstr(uart3_rx_buffer, "Relay 4: OFF") != NULL) {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
            }
            
            uart3_rx_index = 0;
            memset(uart3_rx_buffer, 0, sizeof(uart3_rx_buffer));
        }
        
        
        HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1);
    }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  
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
}


static void MX_SPI1_Init(void)
{

  
  

  
  
  
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  
  

}


static void MX_TIM1_Init(void)
{

  

  

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  

  
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 35;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  

  

}


static void MX_USART1_UART_Init(void)
{

  
  

  
  
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
  
  

}


static void MX_USART3_UART_Init(void)
{

  
  

  
  
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  
  

}


static void MX_DMA_Init(void)
{

  
  __HAL_RCC_DMA1_CLK_ENABLE();

  
  
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  

  
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  
  
}





void Error_Handler(void)
{
  
  
}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
  
  
}
#endif 
