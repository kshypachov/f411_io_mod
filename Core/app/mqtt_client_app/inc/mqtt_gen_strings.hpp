/*
 * mqqt_gen_strings.h
 *
 *  Created on: 26 ���. 2022 �.
 *      Author: kir
 */

#ifndef USERLIB_INC_MQTT_GEN_STRINGS_H_
#define USERLIB_INC_MQTT_GEN_STRINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>



typedef enum{
	INPUT_TOPIK					= 1,
	OUTPUT_TOPIK				= 2,
	ENERGY_SENSOR_TOPIK			= 3,
	POWER_SENSOR_TOPIK			= 4,
	VOLTAGE_SENSOR_TOPIK		= 5,
	POWER_FACTOR_SENSOR_TOPIK	= 6,
	CURRENT_SENSOR_TOPIK 		= 7,
	APPARENT_POWER_SENSOR_TOPIK	= 8
}mqtt_topik_string_type;

typedef enum{
	INP_CONF_PAYLOAD				= 1,
	OUT_CONF_PAYLOAD				= 2,
	ENERGY_SENSOR_PAYLOAD			= 3,
	POWER_SENSOR_PAYLOAD			= 4,
	VOLTAGE_SENSOR_PAYLOAD			= 5,
	POWER_FACTOR_SENSOR_PAYLOAD		= 6,
	CURRENT_SENSOR_PAYLOAD			= 7,
	APPARENT_POWER_SENSOR_PAYLOAD	= 8
}mqtt_conf_payload_string_type;


typedef enum{
	INPUT_SENSOR			= 1,
	OUTPUT_SENSOR			= 2,
	ENERGY_SENSOR			= 3,
	POWER_SENSOR			= 4,
	VOLTAGE_SENSOR			= 5,
	POWER_FACTOR_SENSOR		= 6,
	CURRENT_SENSOR 			= 7,
	APPARENT_POWER_SENSOR	= 8
}mqtt_sensor_type_t;

typedef enum{
	NO_ERR = 0,
	MEM_ALLOC_ERR = -1,
	NOT_FOUND = -2
}mqtt_config_error;

#define MAX_SENSORS 10

typedef struct {
    int sensor_type;    // Тип сенсора (например, INPUT_SENSOR, OUTPUT_SENSOR)
    int sensor_number;  // Номер сенсора
} SensorInfo;



#define component_sensor				"sensor"
#define component_binary_sensor			"binary_sensor"

#define component_input					"input"
#define component_input_human			"Вхід"

#define component_switch				"switch"

#define dev_system						"cedar"
#define dev_common_name					"CedarSwitch"
#define dev_manufacturer_name			"Manufacturer"
#define dev_model_name					"CedarSwitch-3R3I"

#define dev_class_energy				"energy"
#define dev_class_energy_human			"Енергія"
#define dev_class_energy_state			"\"state_class\": \"total_increasing\",\n"
#define dev_class_energy_unit_of_measurement	"kWh"

#define dev_class_voltage				"voltage"
#define dev_class_voltage_human			"Напруга"
#define dev_class_voltage_unit_of_measurement	"V"

#define dev_class_power					"power"
#define dev_class_power_human			"Активна потужність"
#define dev_class_power_unit_of_measurement	"W"

#define dev_class_power_factor			"power_factor"
#define dev_class_power_factor_human	"Коефіцієнт потужності"
#define dev_class_power_factor_unit_of_measurement "%"

#define dev_class_current				"current"
#define dev_class_current_human			"Струм"
#define dev_class_current_unit_of_measurement	"A"

#define dev_class_apparent_power		"apparent_power"
#define dev_class_apparent_power_human	"Повна потужність"
#define dev_class_apparent_power_unit_of_measurement	"VA"

#define dev_class_switch				component_switch
#define dev_class_switch_human			"Перемикач"


#define state_topik						"main"

mqtt_config_error set_device_id(const uint8_t* id, unsigned const int dev_id_len);
mqtt_config_error set_device_conf_ip(char * chr_ip, unsigned int ip_len);
int set_home_assistant_prefix(char * prefix_string, uint8_t prefix_len);
int get_config_topik_string (char * buff, uint32_t buff_len, uint8_t topik_type, uint8_t obj_number);
int get_config_payload_string( char * payload, uint32_t payload_len, uint8_t payload_type, uint8_t obj_number);
int convert_bint_to_JSON_statusIO(char * buff, uint8_t bin, char * on_mess, char * off_mess, char * key_name, uint8_t number_io);
int generate_comand_topik_for_subscrabe(char * topik, uint32_t topik_len, uint8_t sensor_type, uint8_t sensor_number);
int generate_status_topik(char * topik, const uint32_t topik_len, const mqtt_sensor_type_t sensor_type, const uint8_t sensor_number);
void generate_key_value_JSON(char * buf, char * key, float value);
int gen_bin_sensor_status_payload_JSON(char * payload, uint32_t payload_len, uint8_t sensor_type, uint8_t sensor_number, uint8_t state);


#ifdef __cplusplus
}
#endif

#endif /* USERLIB_INC_MQTT_GEN_STRINGS_H_ */
