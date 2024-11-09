/*
 * data_types.h
 *
 *  Created on: Sep 28, 2024
 *      Author: kirill
 */

#ifndef INC_DATA_TYPES_H_
#define INC_DATA_TYPES_H_

#include "stdbool.h"
#include "string.h"
#include "stdint.h"
#include "definitions.h"
#include "mongoose.h"


// Определяем новый тип данных для массива входов
typedef uint8_t inputs_state_t[INPUTS_COUNT];

// Определяем новый тип данных для массива выходов
typedef uint8_t outputs_state_t[OUTPUTS_COUNT];

// Объявляем структуру с использованием этих новых типов данных
typedef struct DeviceStatus {
    inputs_state_t inputs;   // Состояние входов
    outputs_state_t outputs; // Состояние выходов
} DeviceStatus_t;


//Data type for stor all net info from mongoose
typedef struct mg_full_net_info{
	struct mg_mgr* mgr;
	struct mg_tcpip_if * mgr_if;
}mg_full_net_info_t;

typedef struct MQTT_cred_t{
	bool save;
	bool enable;
	char uri[MAX_MQTT_URL_LEN];
	uint16_t port;
	char login[MAX_MQTT_LOGIN_LEN];
	char pass[MAX_MQTT_PASS_LEN];
}__attribute__((packed)) MQTT_cred_struct;

//Define settings typt
typedef enum sett_type{
	S_MQTT,
	S_INPUTS,
	S_OUTPUTS
}sett_type_t;

//Define direction, read settings from memory or write settings to memory
typedef enum sett_direction{
	S_READ,
	S_WRITE
}sett_direction_t;


#endif /* INC_DATA_TYPES_H_ */
