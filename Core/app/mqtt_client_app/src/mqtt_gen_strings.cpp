/*
 * mqtt_gen_strings.c
 *
 *  Created on: 26 ���. 2022 �.
 *      Author: kir
 */

#include "mqtt_gen_strings.hpp"

#include "definitions.h"
#include "string.h"
#include "stdio.h"


#define key_value_float_JSON_template	"{\"%s\" : \"%.2f\"}"

#define universal_config_topik_template	 "%s/%s/%s_%s/%s%u/config"
#define universal_status_topik_template	 "%s_%s/%s%u/state"
#define universal_control_topik_template "%s_%s/%s%u/set" // "[devModel]_[uid(mac)]/[componentName][componentNumber]/[set]"

#define sensor_state_on                  "ON"
#define sensor_state_off                 "OFF"


char* unical_id = NULL; // unical id for example MAC.
char* dev_conf_ip = NULL; //IPv4 or IPv6


char home_assistant_prefix[] = {"homeassistant"};
//unsigned char bin_sens[] =				{"binary_sensor"};
//unsigned char device_name[] = 			{"biver_"};
//unsigned char switch_sens[] = 			{"switch"};
//unsigned char input[] = 				{"input"};
//const unsigned char dev_hw_version[]=	{"0.1"};
//const unsigned char dev_sw_version[]=	{"0.1"};



//const char config_topik_temlate[] = {"%s/%s/biver_%s/%s%u/config"};

//const char switch_conf_payload_templ[] = {  //todo change to const char
//		"{\n"
//			"\t\"device_class\":\"switch\",\n"
//			"\t\"expire_after\" : 300 ,\n"
//			"\t\"state_topic\":\"biver_%s/switches\",\n"
//			"\t\"command_topic\" : \"biver_%s/sw%u/set\",\n"
//			"\t\"value_template\":\"{{ value_json.SW%u }}\",\n"
//			"\t\"name\":\"Switch%u\",\n"
//			"\t\"unique_id\":\"biver_%s_switch%u\",\n"
//			"\t\"device\":{\n"
//					"\t\t\"identifiers\":[\"biver_%s\"],\n"
//					"\t\t\"name\":\"biver_%s\",\n"
//					"\t\t\"model\":\"unit sensors 3i 3o\",\n"
//					"\t\t\"manufacturer\":\"Shypachov\",\n"
//					"\t\t\"hw_version\" : \"v0.1\",\n"
//					"\t\t\"sw_version\" : \"2023-08-18\",\n"
//					"\t\t\"configuration_url\" : \"http://%s\"\n"
//					"\t}\n"
//		"}"
//											};

//const char input_conf_payload_templ[] = {  //todo change to const char
//		"{\n"
//			"\t\"device_class\" :\"power\",\n"
//			"\t\"expire_after\" : 300 ,\n"
//			"\t\"state_topic\" :\"biver_%s/inputs\",\n"
//			"\t\"value_template\":\"{{ value_json.INP%u }}\",\n"
//			"\t\"name\":\"Input%u\",\n"
//			"\t\"unique_id\":\"biver_%s_input%u\",\n"
//			"\t\"device\":{\n"
//					"\t\t\"identifiers\":[\"biver_%s\"],\n"
//					"\t\t\"name\":\"biver_%s\",\n"
//					"\t\t\"model\":\"unit sensors 3i 3o\",\n"
//					"\t\t\"manufacturer\":\"Shypachov\",\n"
//					"\t\t\"hw_version\" : \"v0.1\",\n"
//					"\t\t\"sw_version\" : \"2023-08-18\",\n"
//					"\t\t\"configuration_url\" : \"http://%s\"\n"
//					"\t}\n"
//		"}"
//											};

const char universal_conf_template[]={
		"{\n"
				"\t\"device_class\" :\"%s\",\n"
				"\t\"expire_after\" : 300 ,\n"
				"\t\"state_topic\" :\"%s_%s/%s%u/state\",\n"
				"\t\"value_template\":\"{{ value_json.%s%u }}\",\n"
				"\t%s"
				"\t\"name\":\"%s\",\n"
				"\t\"unique_id\":\"%s_%s_%s%u\",\n"
				"\t\"unit_of_measurement\": \"%s\",\n"
				"\t\"device\":{\n"
						"\t\t\"identifiers\":[\"%s_%s\"],\n"
						"\t\t\"name\":\"%s\",\n"
						"\t\t\"model\":\"%s\",\n"
						"\t\t\"manufacturer\":\"%s\",\n"
						"\t\t\"hw_version\" : \"%s\",\n"
						"\t\t\"sw_version\" : \"%s\",\n"
						"\t\t\"configuration_url\" : \"http://%s\"\n"
						"\t}\n"
		"}"
};

const char universal_conf_template_comand_topik_part[] = {"\"command_topic\" : \"%s_%s/%s%s/set\",\n"};

const unsigned char status_topik_temlate[]	= {"%s_%s/%s"};
//char subscr_topik_template[] 	= {"%s_%s/sw%u/set"	};
char input_status_topik_temp[] 	= {"%s_%s/inputs"	};
char switch_status_topik_temp[] = {"%s_%s/switches"	};
const char io_template[] = {"{\"%s%u\" : \"%s\"}"};

mqtt_config_error set_device_id(const uint8_t* id, unsigned const int id_len){

	if ( unical_id){
		free(unical_id);
	}
    // Каждый байт будет преобразован в два символа + 1 символ для окончания строки
    unical_id = (char *) calloc(id_len * 2 + 1, sizeof(char));  // +1 для '\0'


	if (unical_id){
        // Проходим по каждому байту идентификатора
        for (unsigned int i = 0; i < id_len; i++) {
            // Конвертируем каждый байт в два символа и добавляем в строку
            sprintf(&unical_id[i * 2], "%02X", id[i]);
        }
	}else{
        return MEM_ALLOC_ERR; // Ошибка выделения памяти
	}

	return NO_ERR; // Успешно
}

mqtt_config_error set_device_conf_ip(char * chr_ip, unsigned int ip_len){

	if(dev_conf_ip){
		free(dev_conf_ip);
	}

	dev_conf_ip = (char *)calloc(ip_len, sizeof(char));

	if(dev_conf_ip){
		strncpy(dev_conf_ip, chr_ip, ip_len);
	}else{
		return MEM_ALLOC_ERR; // Ошибка выделения памяти
	}

	return NO_ERR; // Успешно
}

int set_home_assistant_prefix(char * prefix_string, uint8_t prefix_len){
//TODO validation need
 	strncpy(home_assistant_prefix, prefix_string, prefix_len);

	return 0;
}

int get_config_topik_string (char * buff, uint32_t buff_len, uint8_t topik_type, uint8_t obj_number){

	strcpy(buff, "");
	switch (topik_type) {//%s/%s/%s_%s/%s%u/config
		case INPUT_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_binary_sensor, dev_system, unical_id, component_input, obj_number);
			break;
		case OUTPUT_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_switch, dev_system, unical_id, dev_class_switch, obj_number);
			break;
		case ENERGY_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_energy, obj_number);
			break;
		case VOLTAGE_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_voltage, obj_number);
			break;
		case POWER_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_power, obj_number);
			break;
		case APPARENT_POWER_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_apparent_power, obj_number);
			break;
		case POWER_FACTOR_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_power_factor, obj_number);
			break;
		case CURRENT_SENSOR_TOPIK:
			snprintf(buff, buff_len, universal_config_topik_template, home_assistant_prefix, component_sensor, dev_system, unical_id, dev_class_current, obj_number);
			break;
		default:
			return -1;
			break;
	}


	return 0;
}

int get_config_payload_string( char * payload, uint32_t payload_len, uint8_t payload_type, uint8_t obj_number){

	uint8_t len = 0;
	char * name      = NULL;
	char * com_topik = NULL;
	memset(payload, 0, payload_len * sizeof(char));

	switch (payload_type) {

		case INP_CONF_PAYLOAD:

			name = (char *) calloc(MQTT_TOPIK_MAX_LEN, sizeof(char));
			snprintf(name, MQTT_TOPIK_MAX_LEN, "%s %u", component_input_human, obj_number);

		    len = snprintf(payload, payload_len, universal_conf_template, dev_class_power, dev_system, \
							unical_id, component_input, obj_number, component_input, obj_number, "\n", \
							name, dev_system, unical_id, \
		                    component_input, obj_number,"",dev_system, unical_id, dev_common_name, dev_model_name, \
		                    dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
		    free(name);

			return len;
			break;

		case OUT_CONF_PAYLOAD:

			name =      (char *) calloc(MQTT_TOPIK_MAX_LEN, sizeof(char));
			com_topik = (char *) calloc(MQTT_TOPIK_MAX_LEN, sizeof(char));

			snprintf(name,      MQTT_TOPIK_MAX_LEN, "%s%u", component_switch, obj_number);
			snprintf(com_topik, MQTT_TOPIK_MAX_LEN, universal_conf_template_comand_topik_part, dev_system, unical_id, name, "");
			snprintf(name,      MQTT_TOPIK_MAX_LEN, "%s %u", dev_class_switch_human, obj_number);

			len = snprintf(payload, payload_len, universal_conf_template, dev_class_switch, dev_system, \
							unical_id, component_switch, obj_number, component_switch, obj_number, com_topik, \
							name, dev_system, unical_id, \
							component_switch, obj_number,"",dev_system, unical_id, dev_common_name, dev_model_name, \
							dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			//len = sprintf(payload, switch_conf_payload_templ, unical_id, unical_id, obj_number, obj_number, obj_number, unical_id, obj_number, unical_id, unical_id, dev_conf_ip);
			free(name);
			free(com_topik);

			return len;
			break;

		case ENERGY_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_energy, dev_system, \
					unical_id, state_topik, dev_class_energy, dev_class_energy_state, dev_class_energy_human, dev_system, unical_id, \
					dev_class_energy, dev_class_energy_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;
		case VOLTAGE_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_voltage, dev_system, \
					unical_id, state_topik, dev_class_voltage, "\n", dev_class_voltage_human, dev_system, unical_id, \
					dev_class_voltage, dev_class_voltage_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;
		case POWER_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_power, dev_system, \
					unical_id, state_topik, dev_class_power, "\n", dev_class_power_human, dev_system, unical_id, \
					dev_class_power, dev_class_power_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;
		case APPARENT_POWER_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_apparent_power, dev_system, \
					unical_id, state_topik, dev_class_apparent_power, "\n", dev_class_apparent_power_human, dev_system, unical_id, \
					dev_class_apparent_power, dev_class_apparent_power_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;

		case POWER_FACTOR_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_power_factor, dev_system, \
					unical_id, state_topik, dev_class_power_factor, "\n", dev_class_power_factor_human, dev_system, unical_id, \
					dev_class_power_factor, dev_class_power_factor_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;
		case CURRENT_SENSOR_PAYLOAD:
			len = sprintf(payload, universal_conf_template, dev_class_current, dev_system, \
					unical_id, state_topik, dev_class_current, "\n", dev_class_current_human, dev_system, unical_id, \
					dev_class_current, dev_class_current_unit_of_measurement, dev_system, unical_id, dev_common_name, dev_model_name,\
					dev_manufacturer_name, dev_hw_ver, dev_sw_ver, dev_conf_ip);
			break;

		default:
			return -1;
			break;
	}
}


int gen_bin_sensor_status_payload_JSON(char * payload, uint32_t payload_len, uint8_t sensor_type, uint8_t sensor_number, uint8_t state){

	char * sens_name = NULL;
	switch (sensor_type) {
		case INPUT_SENSOR:
			sens_name = (char *)component_input;
			break;
		case OUTPUT_SENSOR:
			sens_name = (char *)dev_class_switch;
			break;
		default:
			break;
	}

	if (state){
		return snprintf(payload, payload_len, io_template, sens_name, sensor_number, sensor_state_on);
	}else{
		return snprintf(payload, payload_len, io_template, sens_name, sensor_number, sensor_state_off);
	}
}

//int convert_bint_to_JSON_statusIO(char * buff, uint8_t bin, char * on_mess, char * off_mess, char * key_name, uint8_t number_io){
//
//
//	uint8_t test;
//	char internal_buff[30];
//
//	strcpy(buff, "\0");
//	strcat(buff, "{\n");
//	while (number_io > 0){
//		test = bin & 0b00000001 << number_io - 1;
//		if (test){
//			sprintf(internal_buff, io_template, key_name, number_io, on_mess);
//		}else{
//			sprintf(internal_buff, io_template, key_name, number_io, off_mess);
//		}
//		strcat(buff, internal_buff);
//
//		number_io --;
//
//		if (number_io) {
//			strcat(buff, ",\n");
//		}
//	}
//
//	strcat(buff, "\n}");
//
//	return 0;
//}

int generate_comand_topik_for_subscrabe(char * topik, uint32_t topik_len, uint8_t sensor_type, uint8_t sensor_number){

	switch(sensor_type){

		case OUTPUT_SENSOR:
			return snprintf(topik, topik_len, universal_control_topik_template, dev_system, unical_id, dev_class_switch, sensor_number );
			break;
	}
}

int generate_status_topik(char * topik, const uint32_t topik_len, const mqtt_sensor_type_t sensor_type, const uint8_t sensor_number){

	strcpy(topik, "\0");

	switch (sensor_type) {
		case INPUT_SENSOR:
			return snprintf(topik, topik_len, universal_status_topik_template, dev_system, unical_id, component_input, sensor_number);
			break;

		case OUTPUT_SENSOR:
			return snprintf(topik, topik_len, universal_status_topik_template, dev_system, unical_id, dev_class_switch, sensor_number);
			break;

		default:
			return -1;
			break;
	}

	return 0;
}

void generate_key_value_JSON(char * buf, char * key, float value){

	sprintf(buf, key_value_float_JSON_template, key, value);
}
