/*
 * mqtt.cpp
 *
 *  Created on: Oct 14, 2024
 *      Author: kiro
 */


#include "mqtt.hpp"
#include "mqtt_gen_strings.hpp"
#include "definitions.h"
#include "logger.h"


static char   *mqtt_broker_url = NULL;
static char   *mqtt_username   = NULL;
static char   *mqtt_password   = NULL;
static char   *topik_buff      = NULL;
static char   *payload_buff    = NULL;
struct mg_mgr *mgr             = NULL;
static struct mg_tcpip_if *mif = NULL;
static struct mg_mqtt_opts mqtt_opts = {}; // Глобальная переменная для хранения состояний MQTT соединения.
static struct mg_connection *mqtt_conn = NULL; //MQTT соединение.
static char * ip_addr_str = NULL; //Переменная для хранения айпи адреса
//mqtt_sensor_type_t sensor_type = INPUT_SENSOR; // Индекс текущего сенсора для регистрации, начинаем с регистрации ENERGY_SENSOR
struct mg_timer * mqtt_timer; //Структура для таймера
struct mg_timer * mqtt_timer_periodic_status_send;
struct mg_timer * mqtt_timer_io_sheck;
bool   is_registered = false; //Статус регистрации сенсоров в  Home Assistant
SensorInfo sensors[] = { //список сенсоров
        {INPUT_SENSOR,  1},
        {INPUT_SENSOR,  2},
		{INPUT_SENSOR,  3},
        {OUTPUT_SENSOR, 1},
		{OUTPUT_SENSOR, 2},
		{OUTPUT_SENSOR, 3}
    };

static void (*r_w_parameter)(void * parameter, sett_type_t parameter_type,  sett_direction_t direction) = NULL;

static void mqtt_send_registration_data(struct mg_connection * arg, const SensorInfo * sensors, const size_t sensor_count){

	mg_snprintf(ip_addr_str, sizeof(char) * IP_v6_STR_LEN, "%M", mg_print_ip, &arg->loc);
	set_device_conf_ip(ip_addr_str, strlen(ip_addr_str));


	for (size_t i = 0; i < sensor_count; i++) {
	        int sensor_type = sensors[i].sensor_type;
	        int sensor_number = sensors[i].sensor_number;
			get_config_topik_string(topik_buff, MQTT_TOPIK_MAX_LEN, sensor_type, sensor_number);

			logging(L_INFO, "Sending registration data to: %s", topik_buff);

			get_config_payload_string(payload_buff, MQTT_PAYLOAD_MAX_LEN, sensor_type, sensor_number);
			mqtt_opts.topic = mg_str(topik_buff);
			mqtt_opts.message = mg_str(payload_buff);
			mqtt_opts.qos = 1;
			mqtt_opts.retain = 1;
			mg_mqtt_pub(arg, &mqtt_opts);

	}
}


static void mqtt_send_io_status(struct mg_connection * arg, bool force_update){

	struct   DeviceStatus status;
	static struct DeviceStatus prev_status;
	uint8_t  i;
	char *   payload = NULL;
	char *   topik = NULL;

	r_w_parameter(status.inputs,  (sett_type_t)INPUT_SENSOR,  S_READ);
	r_w_parameter(status.outputs, (sett_type_t)OUTPUT_SENSOR, S_READ);

	if (memcmp(status.inputs, prev_status.inputs, sizeof(inputs_state_t)) != 0 ||
	    memcmp(status.outputs, prev_status.outputs, sizeof(outputs_state_t)) != 0 || force_update) {
	    // Состояние изменилось, продолжаем
		memcpy(&prev_status, &status, sizeof(status));

		logging(L_INFO, "Send io status data to MQTT server");

		payload = (char * )calloc(MQTT_PAYLOAD_MAX_LEN, sizeof(char));
		topik   = (char * )calloc(MQTT_TOPIK_MAX_LEN,   sizeof(char));

		for (i = 0; i < INPUTS_COUNT; i++){
			gen_bin_sensor_status_payload_JSON(payload, MQTT_PAYLOAD_MAX_LEN, INPUT_SENSOR, i+1, status.inputs[i]);
			generate_status_topik(topik,   MQTT_TOPIK_MAX_LEN, INPUT_SENSOR, i+1);
			mqtt_opts.topic = mg_str(topik);
			mqtt_opts.message = mg_str(payload);
			mqtt_opts.qos = 0;
			mqtt_opts.retain = 0;
			mg_mqtt_pub(arg, &mqtt_opts);
			//strcpy(payload, "\0");
			//strcpy(topik, "\0");
		}

		for (i = 0; i < OUTPUTS_COUNT; i++){
			gen_bin_sensor_status_payload_JSON(payload, MQTT_PAYLOAD_MAX_LEN, OUTPUT_SENSOR, i+1, status.outputs[i]);
			generate_status_topik(topik,   MQTT_TOPIK_MAX_LEN, OUTPUT_SENSOR, i+1);
			mqtt_opts.topic = mg_str(topik);
			mqtt_opts.message = mg_str(payload);
			mqtt_opts.qos = 0;
			mqtt_opts.retain = 0;
			mg_mqtt_pub(arg, &mqtt_opts);
			//strcpy(payload, "\0");
			//strcpy(topik, "\0");
		}


		free(payload);
		free(topik);
	}
}

static void mqtt_subscrabe_on_sw(struct mg_connection *conn, const SensorInfo * sensors, const size_t sensor_count){

	char * topik = NULL;

	for (size_t i = 0; i < sensor_count; i++) {
		if(sensors[i].sensor_type == OUTPUT_SENSOR){
			topik = (char * )calloc(MQTT_TOPIK_MAX_LEN,   sizeof(char));
			generate_comand_topik_for_subscrabe(topik, MQTT_TOPIK_MAX_LEN, OUTPUT_SENSOR, sensors[i].sensor_number);

			logging(L_INFO, "Subscribe on MQTT topik: %s", topik);

			mqtt_opts.qos = 1;
			mqtt_opts.topic = mg_str(topik);
			mg_mqtt_sub(conn, &mqtt_opts);
			free(topik);
		}
	}
}

static void mqtt_subscrabe_recv_cmd_parce(mg_mqtt_message * mess){

	inputs_state_t  sw_r, sw_w;
	int relay_number = 0;  // Переменная для хранения номера реле
    //char *state = NULL;  // Строка для хранения состояния (ON/OFF)

    //Топик выглядит как "cedar_4022422E1436/switch0/set"
    const char *topic = mess->topic.buf;
    const char *message = mess->data.buf;

    if(message == NULL || topic == NULL){
    	MG_ERROR(("Error: topik or message id NULL.\n"));
    	return;
    }

    r_w_parameter(sw_r, (sett_type_t)OUTPUT_SENSOR, S_READ);
    memcpy(sw_w, sw_r, sizeof(sw_r));

    // Ищем "switch" в топике и затем извлекаем номер реле
    const char *switch_position = strstr(topic, "switch");

    if (switch_position != NULL) {
    	// Извлекаем номер реле (например, "3" из "switch3")
    	if (sscanf(switch_position, "switch%d", &relay_number) == 1){
    		MG_INFO(("Getting relay number %d, from topik %s.\n", relay_number, topic));

    		logging(L_INFO, "Getting relay number %d, from topik %s.", relay_number, topic);

    		if(relay_number < 1 || relay_number > OUTPUTS_COUNT){
    			MG_ERROR(("Error: invalid relay number. Relay number should be between 1 and %d. Got %d.\n", OUTPUTS_COUNT, relay_number));

    			logging(L_ERR, "Error: invalid relay number. Relay number should be between 1 and %d. Got %d.\n", OUTPUTS_COUNT, relay_number);

    			return;
    		}

    		// Получаем состояние реле (ON или OFF) из данных сообщения
			if (mess->data.len == 2 && strncmp(message, "ON", 2) == 0) {
				sw_w[relay_number-1] = 1;
				MG_INFO(("Relay %d is ON.\n", relay_number));
				r_w_parameter(sw_w, (sett_type_t)OUTPUT_SENSOR, S_WRITE);
			} else if (mess->data.len == 3 && strncmp(message, "OFF", 3) == 0) {
				sw_w[relay_number-1] = 0;
				MG_INFO(("Relay %d is OFF.\n", relay_number));
				r_w_parameter(sw_w, (sett_type_t)OUTPUT_SENSOR, S_WRITE);

			}else{
				MG_ERROR(("Error: expected message \"ON\" or \"OFF\" but got message: %s \n", message));
				logging(L_ERR, "Error: expected message \"ON\" or \"OFF\" but got message: %s \n", message);
			}

    	}else{
    		MG_ERROR(("Error: relay number is not found, or incorrect format.\n"));
			logging(L_ERR, "Error: relay number is not found, or incorrect format.");
    	}
    }else{
    	MG_ERROR(("Substring \"switch\" is not fount in topik %s. \n", topic));
    	logging(L_ERR, "Substring \"switch\" is not fount in topik %s. \n", topic);
    }
}


static void mqtt_pereodic_status_send_timer_handler(void *arg){
	if(is_registered){
		mqtt_send_io_status(mqtt_conn, 1);
	}
}

static void mqtt_event_handler(struct mg_connection *conn, int ev, void *ev_data){
	if (ev == MG_EV_MQTT_OPEN) {// MQTT connect is successful
		MG_DEBUG(("MQTT_open_connection"));
		logging(L_INFO, "MQTT Open connection");
		mqtt_conn = conn;
		if(!is_registered){
			mqtt_send_registration_data(mqtt_conn, sensors, sizeof(sensors)/sizeof(SensorInfo));
			mqtt_send_io_status(mqtt_conn, 1);
			mqtt_subscrabe_on_sw(mqtt_conn, sensors, sizeof(sensors)/sizeof(SensorInfo));
			is_registered = true;
		}

	}else if (ev == MG_EV_MQTT_MSG){
		logging(L_INFO, "Receive message from MQTT server");
		 struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
		 mqtt_subscrabe_recv_cmd_parce(mm);
		 mqtt_send_io_status(mqtt_conn, 0);

	}else if (ev == MG_EV_MQTT_CMD){

	}else if (ev == MG_EV_CLOSE){
		logging(L_INFO, "MQTT Close connection");
		mqtt_conn     = NULL;
		is_registered = false;
	}
}

// Таймер для установления и поддержания соединения.
static void mqtt_timer_handler(void *arg){
	if (!mqtt_conn){
		logging(L_INFO, "Try to connect to MQTT server");
		memset(&mqtt_opts, 0, sizeof(mqtt_opts));
		mqtt_opts.user 		    = mg_str(mqtt_username);
		mqtt_opts.pass		    = mg_str(mqtt_password);
		mqtt_opts.keepalive 	= 60;
		mqtt_conn = mg_mqtt_connect(mgr, mqtt_broker_url, &mqtt_opts, mqtt_event_handler, NULL);

	}else if(mqtt_conn && is_registered){ //connection established and registered sensors
		//mqtt_send_io_status(mqtt_conn, 0);
		//mg_mqtt_ping(mqtt_conn);
		//SEND IO STATUS DATA
	}else{ // connection established but sensors still not registered at all
		//mqtt_send_registration_data(mqtt_conn, sensors, sizeof(sensors)/sizeof(SensorInfo));
		//is_registered = true;
		//SEND REG DATA
	    //mqtt_send_registration_data(mqtt_conn);
	}

}//void * param, sett_type_t param_type,  sett_direction_t direction

static void mqtt_timer_handler_cher_io_status(void *arg){
	if(mqtt_conn && is_registered){
		mqtt_send_io_status(mqtt_conn, 0);
	}
}

// Getter для mqtt_conn
struct mg_connection* get_mqtt_connection(void){
    return mqtt_conn;
}


void mqtt_init(void *mgr_parameter, void * mif_parameter, void * broker_url, void *username, void *password, void (*fn)(void*, sett_type_t, sett_direction_t)){

	mqtt_broker_url = (char *) calloc(MAX_MQTT_URL_LEN,     sizeof(char));
	mqtt_username   = (char *) calloc(MAX_MQTT_LOGIN_LEN,   sizeof(char));
	mqtt_password   = (char *) calloc(MAX_MQTT_PASS_LEN,    sizeof(char));
	topik_buff      = (char *) calloc(MQTT_TOPIK_MAX_LEN,   sizeof(char));
	payload_buff    = (char *) calloc(MQTT_PAYLOAD_MAX_LEN, sizeof(char));
	ip_addr_str     = (char *) calloc(IP_v6_STR_LEN,        sizeof(char));
	r_w_parameter   = fn;

	strcpy(mqtt_broker_url, (char *)broker_url);
	strcpy(mqtt_username,   (char *)username);
	strcpy(mqtt_password,   (char *)password);

	logging(L_INFO, "Start MQTT task, server: %s, username: %s", mqtt_broker_url, mqtt_username);

	mgr = (struct mg_mgr *)     mgr_parameter;
	mif = (struct mg_tcpip_if*) mif_parameter;

	set_device_id(mif->mac, sizeof(mif->mac));

	mqtt_opts.user 		    = mg_str(mqtt_username);
	mqtt_opts.pass		    = mg_str(mqtt_password);
	mqtt_opts.keepalive 	= 60;

	mqtt_timer =                      mg_timer_add(mgr, 10000 /* 10 seconds */, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, mqtt_timer_handler,                      NULL); // Timer for following connection and reconnect every 10 seconds if needed
	mqtt_timer_periodic_status_send = mg_timer_add(mgr, 15000 /* 15 seconds */, MG_TIMER_REPEAT,                mqtt_pereodic_status_send_timer_handler, NULL); // Timer for send io statuses every ~150 seconds
	mqtt_timer_io_sheck	=             mg_timer_add(mgr,	500 /* 0.5 seconds */, MG_TIMER_REPEAT,                    mqtt_timer_handler_cher_io_status,       NULL); // timer for follow io status every 0.5 seconds

}
