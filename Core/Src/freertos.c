/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "definitions.h"
#include "mongoose.h"
#include "spi.h"
#include "data_types.h"
#include "SPI_flash.h"
#include "fs_adstractions.h"
#include "dashboard.h"
#include "queue.h"
//#include "eth.h"
#include "mqtt_gen_strings.hpp"
#include "mqtt.hpp"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for ethTask */
osThreadId_t ethTaskHandle;
const osThreadAttr_t ethTask_attributes = {
  .name = "ethTask",
  .stack_size = 1500 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IOTask */
osThreadId_t IOTaskHandle;
const osThreadAttr_t IOTask_attributes = {
  .name = "IOTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for settingsTask */
osThreadId_t settingsTaskHandle;
const osThreadAttr_t settingsTask_attributes = {
  .name = "settingsTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for inputReadQ */
osMessageQueueId_t inputReadQHandle;
const osMessageQueueAttr_t inputReadQ_attributes = {
  .name = "inputReadQ"
};
/* Definitions for outputReadQ */
osMessageQueueId_t outputReadQHandle;
const osMessageQueueAttr_t outputReadQ_attributes = {
  .name = "outputReadQ"
};
/* Definitions for outputWriteQ */
osMessageQueueId_t outputWriteQHandle;
const osMessageQueueAttr_t outputWriteQ_attributes = {
  .name = "outputWriteQ"
};
/* Definitions for mqttQ */
osMessageQueueId_t mqttQHandle;
const osMessageQueueAttr_t mqttQ_attributes = {
  .name = "mqttQ"
};
/* Definitions for SPI2Mutex */
osMutexId_t SPI2MutexHandle;
const osMutexAttr_t SPI2Mutex_attributes = {
  .name = "SPI2Mutex"
};
/* Definitions for FSMutex */
osMutexId_t FSMutexHandle;
const osMutexAttr_t FSMutex_attributes = {
  .name = "FSMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void EthResetBegin(void);
void EthResetEnd(void);
void EthBegin(void * param);
void EthEnd(void * param);
void FlashBegin(void);
void FlashEnd(void);
uint8_t EthTxn(void *spi, uint8_t data);
uint8_t GetInputsStatus(void);
uint8_t GetOutputsStatus(void);
void SetOutputs(uint8_t outputs);
void SendByteSPI2(uint8_t byte);
int RecvBuffSPI2(uint8_t * buffer, uint16_t size);
void FS_Lock(void * param);
void FS_Unlock(void * param);
uint8_t SPI_SendReceiveByte(SPI_HandleTypeDef *hspi, uint8_t data);
uint8_t spi_txn(void *spi, uint8_t data);
void RW_parameters_from_queue(void * param, sett_type_t param_type,  sett_direction_t direction);
/* USER CODE END FunctionPrototypes */

void StartEthTask(void *argument);
void StartIOTask(void *argument);
void StartDisplayTask(void *argument);
void StartSettingsTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of SPI2Mutex */
  SPI2MutexHandle = osMutexNew(&SPI2Mutex_attributes);

  /* creation of FSMutex */
  FSMutexHandle = osMutexNew(&FSMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of inputReadQ */
  inputReadQHandle = osMessageQueueNew (1, sizeof(inputs_state_t), &inputReadQ_attributes);

  /* creation of outputReadQ */
  outputReadQHandle = osMessageQueueNew (1, sizeof(outputs_state_t), &outputReadQ_attributes);

  /* creation of outputWriteQ */
  outputWriteQHandle = osMessageQueueNew (1, sizeof(outputs_state_t), &outputWriteQ_attributes);

  /* creation of mqttQ */
  mqttQHandle = osMessageQueueNew (1, sizeof(MQTT_cred_struct), &mqttQ_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ethTask */
  ethTaskHandle = osThreadNew(StartEthTask, NULL, &ethTask_attributes);

  /* creation of IOTask */
  IOTaskHandle = osThreadNew(StartIOTask, NULL, &IOTask_attributes);

  /* creation of displayTask */
  displayTaskHandle = osThreadNew(StartDisplayTask, NULL, &displayTask_attributes);

  /* creation of settingsTask */
  settingsTaskHandle = osThreadNew(StartSettingsTask, NULL, &settingsTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartEthTask */
/**
* @brief Function implementing the ethTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEthTask */
void StartEthTask(void *argument)
{
  /* USER CODE BEGIN StartEthTask */
  void 		           *web_handler;
  struct 			   mg_mgr mgr; // Initialise
  MQTT_cred_struct	   mqtt_conf;
  struct mg_connection *mqtt_conn;
  struct               mg_tcpip_spi spi = {
											  .begin = EthBegin,
											  .end = EthEnd,
											  .txn = EthTxn,
											  .spi = &hspi2,
										  };
  struct              mg_tcpip_if   mif = {
											  .mac = {
													  0x40,
													  0x22,
													  *(uint8_t*)(UID_BASE),
													  *(uint8_t*)(UID_BASE + 0x02),
													  *(uint8_t*)(UID_BASE + 0x04),
													  *(uint8_t*)(UID_BASE + 0x08)
											  },
											  .enable_req_dns = true,
											  .enable_req_sntp = true,
											  .driver = &mg_tcpip_driver_w5500,
											  .driver_data = &spi
										  };

  struct mg_full_net_info mg_full_info = {
											  .mgr = &mgr,
											  .mgr_if = &mif
									  	  };

  mg_mgr_init(&mgr);        // Mongoose event manager
  mg_log_set(MG_LL_DEBUG);  // Set log level

  web_handler = dash_hdl(); // Get dashboard handler

  while(mif.mtu == 0){
	  EthResetBegin();
	  osDelay(500);
	  EthResetEnd();
	  osDelay(2000);
	  mg_tcpip_init(&mgr, &mif);  // Initialize TCP/IP stack
  }
  set_device_id(mif.mac, sizeof(mif.mac));
  reg_parameter_handler(RW_parameters_from_queue);
  mg_http_listen(&mgr, "http://0.0.0.0:80", web_handler, &mg_full_info);

  while (mif.state != MG_TCPIP_STATE_READY) {
    mg_mgr_poll(&mgr, 0);
  }

  RW_parameters_from_queue(&mqtt_conf, S_MQTT, S_READ);
  if (mqtt_conf.enable){
	  mqtt_init(&mgr, &mif, mqtt_conf.uri, mqtt_conf.login, mqtt_conf.pass, RW_parameters_from_queue);
  }

  TickType_t last_tick = xTaskGetTickCount(); // начальное значение тиков

  /* Infinite loop */
  for(;;)
  {
	  TickType_t current_tick = xTaskGetTickCount();
	  uint32_t elapsed_ms = (current_tick - last_tick) * portTICK_PERIOD_MS; // количество миллисекунд, прошедших с последнего вызова
	  last_tick = current_tick; // обновляем значение тиков

	  mg_mgr_poll(&mgr, elapsed_ms); // передаем фактическое время в миллисекундах

      while (mif.driver->up(&mif) == false){
    	  EthResetBegin(); //enable reset state w5500
    	  osDelay(100);
    	  EthResetEnd(); //disable reset state w5500
    	  osDelay(3000);
    	  mif.driver->init(&mif);
    	  osDelay(7000);
      }
      osDelay(1);
  }
  /* USER CODE END StartEthTask */
}

/* USER CODE BEGIN Header_StartIOTask */
/**
  * @brief  Function implementing the IOTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartIOTask */
void StartIOTask(void *argument)
{
  /* USER CODE BEGIN StartIOTask */
	uint16_t pinCoils[] = {coil1_Pin, coil2_Pin, coil3_Pin};
	GPIO_TypeDef* portCoils[] = {coil1_GPIO_Port, coil2_GPIO_Port, coil3_GPIO_Port};
	uint16_t pinInputs[] = {input1_Pin, input2_Pin, input3_Pin};
	GPIO_TypeDef* portInputs[] = {input1_GPIO_Port, input2_GPIO_Port, input3_GPIO_Port};
	inputs_state_t pinInputs_r;
	outputs_state_t pinOutputs_r;
	outputs_state_t pinOutputs_w;
	uint8_t i;

	// check if lenth of massive is correct
	if ((sizeof(pinCoils) / sizeof(pinCoils[0])) != OUTPUTS_COUNT ||
		(sizeof(portCoils) / sizeof(portCoils[0])) != OUTPUTS_COUNT ||
		(sizeof(pinInputs) / sizeof(pinInputs[0])) != INPUTS_COUNT ||
		(sizeof(portInputs) / sizeof(portInputs[0])) != INPUTS_COUNT ||
		(sizeof(pinInputs_r) / sizeof(uint8_t)) != INPUTS_COUNT ||
		(sizeof(pinOutputs_r) / sizeof(uint8_t)) != OUTPUTS_COUNT ||
		(sizeof(pinOutputs_w) / sizeof(uint8_t)) != OUTPUTS_COUNT)
	{
		while (1){

		}
	    // Код обработки ошибки
	}
  /* Infinite loop */
  for(;;)
  {
	 // write outputs
	 if(osMessageQueueGetCount(outputWriteQHandle)){
		 osMessageQueueGet(outputWriteQHandle, &pinOutputs_w, 0, 0);
		 for (i = 0; i < OUTPUTS_COUNT; i++) {
		     if (pinOutputs_w[i]) {
		         HAL_GPIO_WritePin(portCoils[i], pinCoils[i], GPIO_PIN_SET);
		     } else {
		         HAL_GPIO_WritePin(portCoils[i], pinCoils[i], GPIO_PIN_RESET);
		     }
		 }
	 }

	 // read outputs status
	for (i = 0; i < OUTPUTS_COUNT; i++) {
		pinOutputs_r[i] = (uint8_t)HAL_GPIO_ReadPin(portCoils[i], pinCoils[i]);
	}
	//xQueueOverwrite(outputReadQHandle, &pinOutputs_r);
	osMessageQueueReset(outputReadQHandle);
	osMessageQueuePut(outputReadQHandle, &pinOutputs_r, 0, 0);

	// read inputs status
	for (i = 0; i < INPUTS_COUNT; i++) {
		pinInputs_r[i] = (uint8_t)HAL_GPIO_ReadPin(portInputs[i], pinInputs[i]);
	}
	//xQueueOverwrite(inputReadQHandle, &pinInputs_r);
	osMessageQueueReset(inputReadQHandle);
	osMessageQueuePut(inputReadQHandle, &pinInputs_r, 0, 0);

    osDelay(1);
  }
  /* USER CODE END StartIOTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the displayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartSettingsTask */
/**
* @brief Function implementing the settingsTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSettingsTask */
void StartSettingsTask(void *argument)
{
  /* USER CODE BEGIN StartSettingsTask */
	size_t  f_size = 0;
	void *  f_pointer = NULL;

	MQTT_cred_struct mqtt_config;

  SPI_flash_reg_cb(FlashBegin, FlashEnd, RecvBuffSPI2, SendByteSPI2);
  lfs_fs_ll_init(FS_Lock, FS_Unlock);




  mg_fs_lfs_mkdir("/web");
  mg_fs_lfs_remove("/firmware");
  mg_fs_lfs_remove("/firmware.bin");
  mg_fs_lfs_mkdir("/firmware");
  mg_fs_lfs_mkdir("/settings");

  if (mg_fs_lfs_status("/settings/mqtt.conf", &f_size, NULL)){//file found
	  f_pointer = mg_fs_lfs_open("/settings/mqtt.conf", MG_FS_READ);
	  mg_fs_lfs_read(f_pointer, &mqtt_config, sizeof(mqtt_config));
	  mg_fs_lfs_close(f_pointer);
	  mqtt_config.save = 0;
	  xQueueOverwrite(mqttQHandle, &mqtt_config);
  }else{
	  memset(&mqtt_config, 0, sizeof(mqtt_config));
	  xQueueOverwrite(mqttQHandle, &mqtt_config);
  }


  /* Infinite loop */
  for(;;)
  {
	  xQueuePeek(mqttQHandle, &mqtt_config, 0);
	  if(mqtt_config.save){
		  mg_fs_lfs_remove("/settings/mqtt.conf");
		  if((f_pointer = mg_fs_lfs_open("/settings/mqtt.conf", MG_FS_WRITE))){
			  //mg_fs_lfs_seek(f_pointer, 0);
			  mg_fs_lfs_write(f_pointer, &mqtt_config, sizeof(mqtt_config));
			  mg_fs_lfs_close(f_pointer);
			  mqtt_config.save = 0;
			  xQueueOverwrite(mqttQHandle, &mqtt_config);
		  }
	  }

    osDelay(1000);
  }
  /* USER CODE END StartSettingsTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void EthResetBegin(void){
	HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_RESET);
}

void EthResetEnd(void){
	HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_SET);
}

void EthBegin(void * param){
	osMutexAcquire(SPI2MutexHandle, osWaitForever);
	HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_RESET);
}

void EthEnd(void * param){
	osMutexRelease(SPI2MutexHandle);
	HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_SET);
}

uint8_t EthTxn(void *spi, uint8_t data) {
  uint8_t rx_data;
  HAL_SPI_TransmitReceive(&hspi2, &data, &rx_data, 1, HAL_MAX_DELAY);
  return rx_data;
}

void FlashBegin(void){
	osMutexAcquire(SPI2MutexHandle, osWaitForever);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

void FlashEnd(void){
	osMutexRelease(SPI2MutexHandle);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

void SendByteSPI2(uint8_t byte){
	HAL_SPI_Transmit(&hspi2, &byte, 1, HAL_MAX_DELAY);
}

int RecvBuffSPI2(uint8_t * buffer, uint16_t size){
	return HAL_SPI_Receive(&hspi2, buffer, size, HAL_MAX_DELAY);
}

void FS_Lock(void * param){
	osMutexAcquire(FSMutexHandle, HAL_MAX_DELAY);
}

void FS_Unlock(void * param){
	osMutexRelease(FSMutexHandle);
}

uint8_t SPI_SendReceiveByte(SPI_HandleTypeDef *hspi, uint8_t data)
{
    uint8_t receivedData = 0;

    // Отправка и прием данных
    if (HAL_SPI_TransmitReceive(hspi, &data, &receivedData, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        // Обработка ошибки
        // Вы можете добавить обработку ошибок по необходимости
    }

    return receivedData;
}

uint8_t spi_txn(void *spi, uint8_t data)
{
    // Преобразуем указатель к типу SPI_HandleTypeDef
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)spi;

    // �?спользуем функцию отправки и приема байта
    return SPI_SendReceiveByte(hspi, data);
}

void RW_parameters_from_queue(void * param, sett_type_t param_type,  sett_direction_t direction){
	switch (param_type) {
		case S_MQTT:
			MQTT_cred_struct * pr = (MQTT_cred_struct *) param;
			if (direction == S_READ){
				xQueuePeek(mqttQHandle, pr, 0);
				//strncpy(pr->pass, "****", MAX_MQTT_PASS_LEN - 1);
				//pr->pass[MAX_MQTT_PASS_LEN - 1] = '\0'; // добавляем null-terminator
			}else if (direction == S_WRITE){
				xQueueOverwrite(mqttQHandle, pr);
			}
			break;

		case S_INPUTS:
			inputs_state_t * input = (inputs_state_t *) param;
			if (direction == S_READ){
				xQueuePeek(inputReadQHandle, input, 0);
			}
			break;

		case S_OUTPUTS:
			inputs_state_t * output = (inputs_state_t *) param;
			if (direction == S_READ){
				xQueuePeek(outputReadQHandle, output, 0);
			}else if(direction == S_WRITE){
				xQueueOverwrite(outputWriteQHandle, output);
			}
	}
}
/* USER CODE END Application */

