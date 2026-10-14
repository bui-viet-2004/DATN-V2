




#include "main.h"
#include "stm32f1xx_it.h"


#include "zigbee.h"

































extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;








void NMI_Handler(void)
{
  

  
  
  while (1)
  {
  }
  
}


void HardFault_Handler(void)
{
  

  
  while (1)
  {
    
    
  }
}


void MemManage_Handler(void)
{
  

  
  while (1)
  {
    
    
  }
}


void BusFault_Handler(void)
{
  

  
  while (1)
  {
    
    
  }
}


void UsageFault_Handler(void)
{
  

  
  while (1)
  {
    
    
  }
}


void SVC_Handler(void)
{
  

  
  

  
}


void DebugMon_Handler(void)
{
  

  
  

  
}


void PendSV_Handler(void)
{
  

  
  

  
}


void SysTick_Handler(void)
{
  

  
  HAL_IncTick();
  

  
}









void EXTI0_IRQHandler(void)
{
  

  
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  

  
}


void EXTI1_IRQHandler(void)
{
  

  
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  

  
}


void EXTI2_IRQHandler(void)
{
  

  
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  

  
}


void EXTI3_IRQHandler(void)
{
  

  
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
  

  
}


void DMA1_Channel2_IRQHandler(void)
{
  

  
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  

  
}


void USART1_IRQHandler(void)
{
  
  ZB_IRQHandler(); 
  return;          
  
  HAL_UART_IRQHandler(&huart1);
  

  
}


void USART3_IRQHandler(void)
{
  

  
  HAL_UART_IRQHandler(&huart3);
  

  
}




