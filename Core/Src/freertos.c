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
#include "ssd1306.h"
#include "fonts.h"
#include "i2c.h"
#include "logger.h"
#include "sntp.h"
#include "ssdp.h"
#include "mb.h"
#include "mb_tcp_app.h"
#include "mb.h"
#include "mbutils.h"
#include "iwdg.h"
//#include "mbport.h"

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
struct mg_full_net_info mg_full_info;
//flag to indicate file system healthy
int8_t	flash_status_flag = 0;
//flag to indicate flash is not found
uint8_t flash_not_connect_flag = 0;
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
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for settingsTask */
osThreadId_t settingsTaskHandle;
const osThreadAttr_t settingsTask_attributes = {
  .name = "settingsTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for loggingTask */
osThreadId_t loggingTaskHandle;
const osThreadAttr_t loggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for WatchDog */
osThreadId_t WatchDogHandle;
const osThreadAttr_t WatchDog_attributes = {
  .name = "WatchDog",
  .stack_size = 265 * 4,
  .priority = (osPriority_t) osPriorityHigh,
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
/* Definitions for loggingQ */
osMessageQueueId_t loggingQHandle;
const osMessageQueueAttr_t loggingQ_attributes = {
  .name = "loggingQ"
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
//void SendByteSPI2(uint8_t byte);
//int RecvBuffSPI2(uint8_t * buffer, uint16_t size);
void FlashSPIsendByte(uint8_t byte);
int FlashSPIrecvBuff(uint8_t * buffer, uint16_t size);

void FS_Lock(void * param);
void FS_Unlock(void * param);
uint8_t EthSPIsendReceiveByte(SPI_HandleTypeDef *hspi, uint8_t data);
uint8_t spi_txn(void *spi, uint8_t data);
void RW_parameters_from_queue(void * param, sett_type_t param_type,  sett_direction_t direction);
void add_log_mess_to_q(struct log_message mess);
void empty_fn(void *data);
/* USER CODE END FunctionPrototypes */

void StartEthTask(void *argument);
void StartIOTask(void *argument);
void StartDisplayTask(void *argument);
void StartSettingsTask(void *argument);
void StartLoggingTask(void *argument);
void StartWatchDogTask(void *argument);

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

  /* creation of loggingQ */
  loggingQHandle = osMessageQueueNew (15, sizeof(log_message_t), &loggingQ_attributes);

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

  /* creation of loggingTask */
  loggingTaskHandle = osThreadNew(StartLoggingTask, NULL, &loggingTask_attributes);

  /* creation of WatchDog */
  WatchDogHandle = osThreadNew(StartWatchDogTask, NULL, &WatchDog_attributes);

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

  mg_full_info.mgr =  &mgr;
  mg_full_info.mgr_if =  &mif;


  while (mg_fs_mounted() == 0){
	  osDelay(500);
  }

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
    mg_mgr_poll(&mgr, 1);
    osDelay(1);
  }

  logging(L_INFO, "IP addr: %lu.%lu.%lu.%lu", (mg_full_info.mgr_if->ip) & 0xFF, (mg_full_info.mgr_if->ip >> 8) & 0xFF,
			(mg_full_info.mgr_if->ip >> 16) & 0xFF, (mg_full_info.mgr_if->ip >> 24) & 0xFF);

  RW_parameters_from_queue(&mqtt_conf, S_MQTT, S_READ);
  if (mqtt_conf.enable){
	  logging(L_INFO, "MQTT function enabled.");
	  mqtt_init(&mgr, &mif, mqtt_conf.uri, mqtt_conf.login, mqtt_conf.pass, RW_parameters_from_queue);
  }else{
	  logging(L_INFO, "MQTT function disabled");
  }
  start_sntp(&mgr);
  ssdp_start_server(&mgr, &mif);
  init_mb_tcp(&mgr);
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
	uint16_t pinCoils[] = {coil3_Pin, coil2_Pin,coil1_Pin};
	GPIO_TypeDef* portCoils[] = {coil3_GPIO_Port, coil2_GPIO_Port, coil1_GPIO_Port};
	uint16_t pinInputs[] = {input3_Pin, input2_Pin, input1_Pin };
	GPIO_TypeDef* portInputs[] = {input3_GPIO_Port, input2_GPIO_Port, input1_GPIO_Port};
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
		pinInputs_r[i] = (uint8_t)(!HAL_GPIO_ReadPin(portInputs[i], pinInputs[i]));
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
	uint8_t x,y;
	char buf[20];
	osDelay(1000);//

	static struct DeviceStatus io_status;

	SSD1306_Init (); // initialise the display
	osDelay(100);//
	SSD1306_GotoXY(x=1,y=0);
	SSD1306_Fill(SSD1306_COLOR_BLACK);

	if (flash_not_connect_flag){
		SSD1306_GotoXY(x=1,y=0);
		SSD1306_Fill(SSD1306_COLOR_BLACK);
		SSD1306_Puts("Error. Flash mem", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_GotoXY(x=1,y=10);
		SSD1306_Puts("chip is not", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_GotoXY(x=1,y=20);
		SSD1306_Puts("connected. Reboot", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_UpdateScreen();
		vTaskDelay(3000);
	}

	SSD1306_Puts("Builded: ", &Font_7x10, SSD1306_COLOR_WHITE);
	//SSD1306_GotoXY(x=1,y=y+11);
	SSD1306_Puts(__TIME__, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(x=1,y=y+11);
	SSD1306_Puts(__DATE__, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_GotoXY(x=1,y=y+11);
	sprintf((char *)buf,"%02X:%02X:%02X:%02X:%02X:%02X",
			mg_full_info.mgr_if->mac[0], mg_full_info.mgr_if->mac[1],
			mg_full_info.mgr_if->mac[2], mg_full_info.mgr_if->mac[3],
			mg_full_info.mgr_if->mac[4], mg_full_info.mgr_if->mac[5]);
	SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_WHITE);

	SSD1306_GotoXY(x=1,y=y+11);
	sprintf((char *)buf, "%s",get_flash_chip_model());
	SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();
	osDelay(3000);
	int i=0;

  /* Infinite loop */
	for(;;)
	{
		RW_parameters_from_queue(io_status.inputs, S_INPUTS, S_READ);
		RW_parameters_from_queue(io_status.outputs, S_OUTPUTS, S_READ);

		SSD1306_Fill(SSD1306_COLOR_BLACK);


		switch (flash_status_flag) {
		case -5:
			SSD1306_GotoXY(x=1,y=40);
			SSD1306_Puts("LFS_ERR_IO", &Font_7x10, SSD1306_COLOR_WHITE);
			break;
		case -84:
			SSD1306_GotoXY(x=1,y=40);
			SSD1306_Puts("LFS_ERR_CORRUPT", &Font_7x10, SSD1306_COLOR_WHITE);
			break;
		case -24:
			SSD1306_GotoXY(x=1,y=40);
			SSD1306_Puts("LFS_ERR_NOSPC", &Font_7x10, SSD1306_COLOR_WHITE);
			break;
		case -12:
			SSD1306_GotoXY(x=1,y=40);
			SSD1306_Puts("LFS_ERR_NOMEM", &Font_7x10, SSD1306_COLOR_WHITE);
			break;
		default:
			break;
		}

		SSD1306_GotoXY(x=1,y=0);
		SSD1306_Puts("IP:", &Font_7x10, SSD1306_COLOR_WHITE);
		sprintf(buf,"%lu.%lu.%lu.%lu",
				(mg_full_info.mgr_if->ip) & 0xFF, (mg_full_info.mgr_if->ip >> 8) & 0xFF,
				(mg_full_info.mgr_if->ip >> 16) & 0xFF, (mg_full_info.mgr_if->ip >> 24) & 0xFF);
		SSD1306_Puts(buf, &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_GotoXY(x,y=y+11);
		SSD1306_Puts("Inputs:", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_GotoXY(x=x+60,y);
		for (i=0 ; i<INPUTS_COUNT ; i++){
			sprintf((char *)buf,"%d",i+1);
			if ( io_status.inputs[i] ){
				SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_BLACK);
			}else{
				SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			SSD1306_GotoXY(x=x+8,y);
		}
		SSD1306_GotoXY(x=1,y=y+11);
		SSD1306_Puts("Outputs:", &Font_7x10, SSD1306_COLOR_WHITE);
		SSD1306_GotoXY(x=x+60,y);

		for (i=0 ; i<OUTPUTS_COUNT ; i++){
			sprintf((char *)buf,"%d",i+1);
			if ( io_status.outputs[i] ){
				SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_BLACK);
			}else{
				SSD1306_Puts((char *)buf, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			SSD1306_GotoXY(x=x+8,y);
		}

		SSD1306_UpdateScreen();

		while(HAL_I2C_IsDeviceReady(&hi2c1, SSD1306_I2C_ADDR, 1, 1000) != HAL_OK){
			HAL_I2C_DeInit(&hi2c1);
			vTaskDelay(1000);
			MX_I2C1_Init();
			vTaskDelay(1000);
			SSD1306_Init ();
		}
		osDelay(200);
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
	uint8_t press_time = 0;

	MQTT_cred_struct mqtt_config;

  SPI_flash_reg_cb(FlashBegin, FlashEnd, FlashSPIrecvBuff, FlashSPIsendByte);
  if (lfs_fs_ll_init(FS_Lock, FS_Unlock) < 0){
	  flash_not_connect_flag = 1; //indicate that flash is not found
	  osDelay(3000);
	  HAL_NVIC_SystemReset();
  }

  mg_fs_lfs_mkdir("/web");
  mg_fs_lfs_remove("/firmware");
  mg_fs_lfs_remove("/firmware.bin");
  mg_fs_lfs_mkdir("/firmware");
  mg_fs_lfs_mkdir("/settings");
  mg_fs_lfs_mkdir("/certs");
  mg_fs_lfs_mkdir("/log");
  mg_fs_lfs_mkdir("/auth");



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

  if (mg_fs_lfs_status(WEB_USERS_FILE, &f_size, NULL) == 0){//file not found
	  users_list_t *web_users = NULL;
	  web_users = (users_list_t *)malloc(user_list_size * sizeof(users_list_t));

	  if (web_users == NULL) {
		  logging(L_ERR, "Failed to allocate memory for 'web_users' ");
	  }else{
		  logging(L_INFO, "Initialize default login pass for web (admin:admin)'");
		  strcpy(web_users[0].user, WEB_ADMIN_DEFAULT);

		  unsigned char pass_digest[] = WEB_ADMIN_PASS_DIGEST;

		  memcpy(web_users[0].pass_digest, pass_digest, sizeof(pass_digest));

		  f_pointer = mg_fs_lfs_open(WEB_USERS_FILE, MG_FS_WRITE);

		  mg_fs_lfs_write(f_pointer, web_users, user_list_size * sizeof(users_list_t));
		  mg_fs_lfs_close(f_pointer);
	  }

	  free(web_users);
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

	 if(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){ // If key pressed, more than 8 secons - reset auth file and reset mcu
		 press_time++;
		 HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		 osDelay(100);
		 HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

		 if (press_time == 8){
			 HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			 while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
				 osDelay(1);
			 }
			 osDelay(1000);
			 mg_fs_lfs_remove(WEB_USERS_FILE);
			 HAL_NVIC_SystemReset();
		 }
	 }else{
		 press_time = 0;
	 }

	  osDelay(1000);
  }
  /* USER CODE END StartSettingsTask */
}

/* USER CODE BEGIN Header_StartLoggingTask */
/**
* @brief Function implementing the loggingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoggingTask */
void StartLoggingTask(void *argument)
{
  /* USER CODE BEGIN StartLoggingTask */

	uint32_t count = 19400;
	void *  f_pointer = NULL;
	size_t fs_size;
	HeapStats_t heap_status;

	reg_logging_fn(add_log_mess_to_q);
	logger_set_level(L_INFO);
	logging(L_INFO, "Device started...");

	  while (mg_fs_mounted() == 0){
		  osDelay(500);
	  }

  /* Infinite loop */
  for(;;)
  {
	  //--------logging
	  if (osMessageQueueGetCount(loggingQHandle)){
		  log_message_t log;
		  osMessageQueueGet(loggingQHandle, &log, 0, 0);
		  f_pointer = mg_fs_lfs_open(LOG_FILE_LOCATION, MG_FS_WRITE);
		  flash_status_flag = mg_fs_lfs_write(f_pointer, &log.log_text, log.log_len);
		  mg_fs_lfs_close(f_pointer);
	  }
	  //--------logging
    osDelay(500);


    if (count == 20000){
    	mg_fs_lfs_status(LOG_FILE_LOCATION, &fs_size, NULL);
    	if (fs_size > LOG_FILE_MAX_SIZE){
    		mg_fs_lfs_remove(LOG_FILE_LOCATION_OLD);
    		mg_fs_lfs_rename(LOG_FILE_LOCATION, LOG_FILE_LOCATION_OLD);
    		logging(L_INFO, "Log file rotated");
    	}

    	count = 0;
    	vPortGetHeapStats(&heap_status);
    	logging(L_INFO, "Free Heap: %u, LargestFreeBlock: %u, SmallestFeeBlock: %u, NumFreeBlock: %u,  MinEverFreeBytes: %u, NumOfSuccessAlloc: %u, NumOfSuccessFree: %u ",
                (unsigned int)heap_status.xAvailableHeapSpaceInBytes,
                (unsigned int)heap_status.xSizeOfLargestFreeBlockInBytes,
                (unsigned int)heap_status.xSizeOfSmallestFreeBlockInBytes,
                (unsigned int)heap_status.xNumberOfFreeBlocks,
                (unsigned int)heap_status.xMinimumEverFreeBytesRemaining,
                (unsigned int)heap_status.xNumberOfSuccessfulAllocations,
                (unsigned int)heap_status.xNumberOfSuccessfulFrees);

    	logging(L_INFO, "IP addr: %lu.%lu.%lu.%lu",
    			(mg_full_info.mgr_if->ip) & 0xFF, (mg_full_info.mgr_if->ip >> 8) & 0xFF,
				(mg_full_info.mgr_if->ip >> 16) & 0xFF, (mg_full_info.mgr_if->ip >> 24) & 0xFF);

    	logging(L_INFO, "MAC addr: %02X:%02X:%02X:%02X:%02X:%02X",
			mg_full_info.mgr_if->mac[0], mg_full_info.mgr_if->mac[1],
			mg_full_info.mgr_if->mac[2], mg_full_info.mgr_if->mac[3],
			mg_full_info.mgr_if->mac[4], mg_full_info.mgr_if->mac[5]);

    	logging(L_INFO, "Flash chip model name: %s",  get_flash_chip_model());
    	logging(L_INFO, "Firmware version: %s", dev_sw_ver);

    }
    count ++;
  }
  /* USER CODE END StartLoggingTask */
}

/* USER CODE BEGIN Header_StartWatchDogTask */
/**
* @brief Function implementing the WatchDog thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWatchDogTask */
void StartWatchDogTask(void *argument)
{
  /* USER CODE BEGIN StartWatchDogTask */
  /* Infinite loop */
  for(;;)
  {
	HAL_IWDG_Refresh(&hiwdg);
    osDelay(500);
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  }
  /* USER CODE END StartWatchDogTask */
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
	//osMutexAcquire(SPI2MutexHandle, osWaitForever);
	HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_RESET);
}

void EthEnd(void * param){
	HAL_GPIO_WritePin(ETH_CS_GPIO_Port, ETH_CS_Pin, GPIO_PIN_SET);
	//osMutexRelease(SPI2MutexHandle);
}

uint8_t EthTxn(void *spi, uint8_t data) {
  uint8_t rx_data;
  HAL_SPI_TransmitReceive(&hspi2, &data, &rx_data, 1, HAL_MAX_DELAY);
  return rx_data;
}

void FlashBegin(void){
	//osMutexAcquire(SPI2MutexHandle, osWaitForever);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

void FlashEnd(void){
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	//osMutexRelease(SPI2MutexHandle);
}

void FlashSPIsendByte(uint8_t byte){
	HAL_SPI_Transmit(&hspi1, &byte, 1, HAL_MAX_DELAY);
}

int FlashSPIrecvBuff(uint8_t * buffer, uint16_t size){
	return HAL_SPI_Receive(&hspi1, buffer, size, HAL_MAX_DELAY);
}

void FS_Lock(void * param){
	osMutexAcquire(FSMutexHandle, HAL_MAX_DELAY);
}

void FS_Unlock(void * param){
	osMutexRelease(FSMutexHandle);
}

uint8_t EthSPIsendReceiveByte(SPI_HandleTypeDef *hspi, uint8_t data)
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
    return EthSPIsendReceiveByte(hspi, data);
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

void add_log_mess_to_q(struct log_message mess){

	osMessageQueuePut(loggingQHandle,  &mess, 0, 0);

}

void empty_fn(void *data){};

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete){

	eMBErrorCode eStatus = MB_ENOERR;

	//usAddress --; /* to c-style address */
	if (((int16_t) usAddress >= INPUTS_FIRST_INDEX) && (usAddress + usNDiscrete <= INPUTS_FIRST_INDEX + INPUTS_COUNT)){
		inputs_state_t input;
		usAddress --; /* to c-style address */
		RW_parameters_from_queue(input, S_INPUTS, S_READ);

		for(int i = usAddress; i < usNDiscrete; i++){
			xMBUtilSetBits( pucRegBuffer, i, 1, input[i] );
		}
	}else{
		eStatus = MB_ENOREG;
	}
	return eStatus;
}

eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
	eMBErrorCode eStatus = MB_ENOERR;

	if (((int16_t) usAddress >= OUTPUTS_FIRST_INDEX) && (usAddress + usNCoils <= OUTPUTS_FIRST_INDEX + OUTPUTS_COUNT)){
		outputs_state_t outputs;

		switch (eMode) {

			case MB_REG_WRITE:
				usAddress --; /* to c-style address */
				RW_parameters_from_queue(outputs, S_OUTPUTS, S_READ);
				for(int i = usAddress; i < usNCoils + usAddress; i++){
					UCHAR wbit = xMBUtilGetBits(pucRegBuffer, i, 1 );
					outputs[i] = xMBUtilGetBits(pucRegBuffer, 0, 1 );
					//xMBUtilSetBits( pucRegBuffer, i, 1, outputs[i] );
				}
				RW_parameters_from_queue(outputs, S_OUTPUTS, S_WRITE);
				return MB_ENOERR;
				break;
			case MB_REG_READ:
				usAddress --; /* to c-style address */
				RW_parameters_from_queue(outputs, S_OUTPUTS, S_READ);
				for(int i = usAddress; i < usNCoils; i++){
					xMBUtilSetBits( pucRegBuffer, i, 1, outputs[i] );
				}
				return MB_ENOERR;
				break;
			default:
				return MB_ENOREG;
				break;
		}
	}
	return MB_ENOREG;
}

eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	return MB_ENOREG;
}

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{
	return MB_ENOREG;
}

/* USER CODE END Application */

