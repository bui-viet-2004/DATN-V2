

#include <stdbool.h>
#include "DHT.h"

#define DHT11_PIN_OUTPUT 0
#define DHT11_PIN_INPUT 1
#define DHT11_MAX_DATA_BITS 40
#define DHT11_MAX_BYTE_PACKETS 5
#define DHT11_MAX_TIMEOUT 100

#define TEMP_C_TO_F(x) (((x) * 1.8f) + (32))

const char* const ErrorMsg[__DHT11_STATUS_TYPEDEF_COUNT__] = {
		"OK",
		"TIMEOUT",
		"ERROR",
		"CHECKSUM MISMATCH"
};

static void HAL_DelayUs(TIM_HandleTypeDef *tim, uint16_t us) {
	__HAL_TIM_SET_COUNTER(tim, 0);
	while(__HAL_TIM_GET_COUNTER(tim) < us);
}

static bool DHT11_ObserveState(DHT11_InitTypeDef *DHT11, uint8_t FinalState) {
	__HAL_TIM_SET_COUNTER(DHT11->_Tim, 0);
	while(__HAL_TIM_GET_COUNTER(DHT11->_Tim) < DHT11_MAX_TIMEOUT) {
		if(HAL_GPIO_ReadPin(DHT11->_GPIOx, DHT11->_Pin) == FinalState) return true;
	}

	return false;
}

static void DHT11_SetPinMode(DHT11_InitTypeDef *DHT11, uint8_t MODE) {
	GPIO_InitTypeDef GPIO_CFG = {
			.Mode = MODE?GPIO_MODE_INPUT:GPIO_MODE_OUTPUT_PP,
			.Pin = DHT11->_Pin,
			.Pull = GPIO_NOPULL,
			.Speed = GPIO_SPEED_FREQ_LOW
	};

	HAL_GPIO_Init(DHT11->_GPIOx, &GPIO_CFG);
}

void HAL_DHT11_Init(
		DHT11_InitTypeDef *DHT11,
		GPIO_TypeDef *GPIOx,
		uint16_t GPIO_Pin,
		TIM_HandleTypeDef *TIM
) {
	DHT11->_GPIOx = GPIOx;
	DHT11->_Pin = GPIO_Pin;
	DHT11->_Tim = TIM;
	DHT11->Temperature = 0.0f;
	DHT11->Humidity = 0.0f;

	HAL_TIM_Base_Start(DHT11->_Tim);
}

void HAL_DHT11_DeInit(DHT11_InitTypeDef *DHT11) {
	HAL_GPIO_DeInit(DHT11->_GPIOx, DHT11->_Pin);
	HAL_TIM_Base_Stop(DHT11->_Tim);
}

const char* const HAL_DHT11_GetErrorMsg(DHT11_StatusTypeDef Status) {
	return ErrorMsg[Status];
}

static DHT11_StatusTypeDef DHT11_ReadData(DHT11_InitTypeDef *DHT11) {
	uint8_t Bits = 0;
	uint8_t Packets[DHT11_MAX_BYTE_PACKETS] = {0};
	uint8_t PacketIndex = 0;

	DHT11_SetPinMode(DHT11, DHT11_PIN_OUTPUT);
	// PULLING the Line to Low and waits for 20ms
	HAL_GPIO_WritePin(DHT11->_GPIOx, DHT11->_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	// PULLING the Line to HIGH and waits for 40us
	HAL_GPIO_WritePin(DHT11->_GPIOx, DHT11->_Pin, GPIO_PIN_SET);
	HAL_DelayUs(DHT11->_Tim, 40);

	__disable_irq();
	DHT11_SetPinMode(DHT11, DHT11_PIN_INPUT);

	// If the Line is still HIGH, that means DHT11 is not responding
	if(HAL_GPIO_ReadPin(DHT11->_GPIOx, DHT11->_Pin) == GPIO_PIN_SET) {
		__enable_irq();
		return DHT11_ERROR;
	}

	// Now DHT11 have pulled the Line to LOW, we will wait till it PULLS is HIGH
	if(!DHT11_ObserveState(DHT11, GPIO_PIN_SET)) {
		__enable_irq();
		return DHT11_TIMEOUT;
	}

	// Now DHT11 have pulled the Line to HIGH, we will wait till it PULLS is to LOW
	// which means the handshake is done
	if(!DHT11_ObserveState(DHT11, GPIO_PIN_RESET)) {
			__enable_irq();
			return DHT11_TIMEOUT;
	}

	while(Bits < 40) {
			// DHT11 is now starting to transmit One Bit
			// We will wait till it PULL the Line to HIGH
			if(!DHT11_ObserveState(DHT11, GPIO_PIN_SET)) {
				__enable_irq();
				return DHT11_TIMEOUT;
			}

			// Now we will just count the us it stays HIGH
			// 28us means 0
			// 70us means 1
			__HAL_TIM_SET_COUNTER(DHT11->_Tim, 0);
			while(HAL_GPIO_ReadPin(DHT11->_GPIOx, DHT11->_Pin) == GPIO_PIN_SET) {
				if(__HAL_TIM_GET_COUNTER(DHT11->_Tim) > DHT11_MAX_TIMEOUT) {
					return DHT11_TIMEOUT;
				}
			}

			Packets[PacketIndex] = Packets[PacketIndex] << 1;
			Packets[PacketIndex] |= (__HAL_TIM_GET_COUNTER(DHT11->_Tim) > 50); // 50us is good in between
			Bits++;
			if(!(Bits % 8)) PacketIndex++;
	}

	__enable_irq();

	// Last 8 bits are Checksum, which is the sum of all the previously transmitted 4 bytes
	if(Packets[4] != (Packets[0] + Packets[1] + Packets[2] + Packets[3])) {
		return DHT11_CHECKSUM_MISMATCH;
	}

	DHT11->Humidity = Packets[0] + (Packets[1] * 0.1f);
	DHT11->Temperature = Packets[2] + (Packets[3] * 0.1f);

	return DHT11_OK;
}

DHT11_StatusTypeDef HAL_DHT11_ReadData(DHT11_InitTypeDef *DHT11) {
	return DHT11->Status = DHT11_ReadData(DHT11);
}

float HAL_DHT11_ReadTemperatureC(DHT11_InitTypeDef *DHT11) {
	HAL_DHT11_ReadData(DHT11);
	return DHT11->Temperature;
}

float HAL_DHT11_ReadTemperatureF(DHT11_InitTypeDef *DHT11) {
	float TempC = HAL_DHT11_ReadTemperatureC(DHT11);
	return TEMP_C_TO_F(TempC);
}

float HAL_DHT11_ReadHumidity(DHT11_InitTypeDef *DHT11) {
	HAL_DHT11_ReadData(DHT11);
	return DHT11->Humidity;
}