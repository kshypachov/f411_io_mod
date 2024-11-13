/*
 * dashboard.c
 *
 *  Created on: Oct 5, 2024
 *      Author: kirill
 */


#include "dashboard.h"
#include "fs_adstractions.h"
#include "mongoose.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "stdbool.h"
#include "mqtt.hpp"
#include "mqtt_gen_strings.hpp"
#include "data_types.h"
#include "stm32f4xx_hal.h"
#include "validation.h"
#include "logger.h"

struct mg_fs mg_fs_lfs = {
	    .st = mg_fs_lfs_status,
	    .ls = mg_fs_lfs_list,
	    .op = mg_fs_lfs_open,
	    .cl = mg_fs_lfs_close,
	    .rd = mg_fs_lfs_read,
	    .wr = mg_fs_lfs_write,
	    .sk = mg_fs_lfs_seek,
	    .mv = mg_fs_lfs_rename,
	    .rm = mg_fs_lfs_remove,
	    .mkd = mg_fs_lfs_mkdir,
	};

#define mqtt_conf_answ    "{\"enabled\": %s, \"broker\": \"%s\", \"username\": \"%s\", \"password\": \"%s\"}\r\n"

#define headers     "Content-Type: application/json\r\n" "Access-Control-Allow-Origin: *\r\n" "Access-Control-Allow-Headers: Content-Type\r\n"

static void (*r_w_parameter)(void * parameter, sett_type_t parameter_type,  sett_direction_t direction) = NULL;



static void handle_ram_status_get(struct mg_connection *c){
	HeapStats_t heap_status;
	vPortGetHeapStats(&heap_status);

	logging(1, "Start execute API RAM status");

    // Формирование и отправка JSON ответа с помощью mg_http_reply
    mg_http_reply(c, 200, "Content-Type: application/json\r\n"
    				"Access-Control-Allow-Origin: *\r\n", //TODO delete for release,
                  "{"
                  "\"xAvailableHeapSpaceInBytes\": \"%u\","
                  "\"xSizeOfLargestFreeBlockInBytes\": \"%u\","
                  "\"xSizeOfSmallestFreeBlockInBytes\": \"%u\","
                  "\"xNumberOfFreeBlocks\": \"%u\","
                  "\"xMinimumEverFreeBytesRemaining\": \"%u\","
                  "\"xNumberOfSuccessfulAllocations\": \"%u\","
                  "\"xNumberOfSuccessfulFrees\": \"%u\""
                  "}\r\n",
                  (unsigned int)heap_status.xAvailableHeapSpaceInBytes,
                  (unsigned int)heap_status.xSizeOfLargestFreeBlockInBytes,
                  (unsigned int)heap_status.xSizeOfSmallestFreeBlockInBytes,
                  (unsigned int)heap_status.xNumberOfFreeBlocks,
                  (unsigned int)heap_status.xMinimumEverFreeBytesRemaining,
                  (unsigned int)heap_status.xNumberOfSuccessfulAllocations,
                  (unsigned int)heap_status.xNumberOfSuccessfulFrees);
}

static int calc_file_md5sum(const char * path, char md5_str[static 33]){
	uint32_t md5_str_size = 33; // 32 символа для MD5 + 1 символ для '\0'
	uint16_t  buff_len = 1024;
	uint8_t	  digest_len = 16;
	char * buff = calloc(buff_len, 1);
	if (buff == NULL){
		free(buff);
		return -1;
	}


	struct mg_fd *fd = mg_fs_open(&mg_fs_lfs, path, MG_FS_READ); //Try to open file
	if (fd==NULL){
		mg_fs_close(fd);
		free(buff);
		return -2;
		}


	mg_md5_ctx md5_ctx;
	mg_md5_init(&md5_ctx);
	size_t bytes_read;
	while ((bytes_read = mg_fs_lfs.rd(fd->fd, buff, buff_len)) > 0) {
		mg_md5_update(&md5_ctx, (const unsigned char *)buff, bytes_read);
	}

	mg_fs_close(fd);

	uint8_t * digest = calloc(digest_len, 1);
	if (digest == NULL){
		free(buff);
		free(digest);
		return -1;
	}

	mg_md5_final(&md5_ctx, digest);
	// Преобразование контрольной суммы в строку
	for (int i = 0; i < 16; ++i) {
		snprintf(md5_str + (i * 2), md5_str_size - (i * 2), "%02x", digest[i]);
	}

	free(buff);
	free(digest);

	return 0;
}

static void handle_io_status_get(struct mg_connection *c, struct mg_http_message *hm){


	//Processing GET request
	if (mg_match(hm->method, mg_str("GET"), NULL)){
		struct DeviceStatus status;

		r_w_parameter(status.inputs, S_INPUTS, S_READ);
		r_w_parameter(status.outputs, S_OUTPUTS, S_READ);
		mg_http_reply(c, 200, headers, //TODO delete for release,
						"{"
						"\"inputs\": [%d, %d, %d], \"outputs\": [%d, %d, %d]"
						"}\r\n",
						status.inputs[0], status.inputs[1], status.inputs[2],
						status.outputs[0], status.outputs[1], status.outputs[2]
						);
	}else if(mg_match(hm->method, mg_str("POST"), NULL)){
		uint8_t relay_id;
		uint8_t state;
		outputs_state_t output_status;
		double tmp1, tmp2;

		// Извлекаем номер реле и его состояние из тела POST-запроса
		if (mg_json_get_num(hm->body, "$.relay_id", &tmp1) > 0 &&
			mg_json_get_num(hm->body, "$.state", &tmp2) > 0){

			relay_id = (uint8_t) tmp1;
			state = (uint8_t) tmp2;
			// Проверка корректности данных
			if (relay_id > 0 && relay_id <= OUTPUTS_COUNT && (state == 0 || state == 1)) {
				// Управление состоянием реле
				r_w_parameter(output_status, S_OUTPUTS, S_READ);
				output_status[relay_id-1] = state;  // make -1 for make human understandable
				r_w_parameter(output_status, S_OUTPUTS, S_WRITE);
				// Возвращаем успешный ответ
				mg_http_reply(c, 200, headers , //TODO delete for release,
						"{\"status\":\"success\", \"message\":\"Relay %d set to %d\"}\r\n", relay_id, state);
			} else {
                // Неверные данные
                mg_http_reply(c, 400, headers, //TODO delete for release,
                		"{\"status\":\"error\",\"message\":\"Invalid relay_id or state\"}\r\n");
            }
		}else {
            // Ошибка в теле запроса
            mg_http_reply(c, 400, headers, //TODO delete for release,
            		"{\"status\":\"error\",\"message\":\"Missing relay_id or state\"}\r\n");
        }

	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
				"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET, POST methods\"}\r\n");

	}
}

static void handle_mqtt_conf(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("GET"), NULL)){

		uint8_t pass_len = 0;
		MQTT_cred_struct * mqtt_config = (MQTT_cred_struct *) calloc(1, sizeof(MQTT_cred_struct));
		if (mqtt_config == NULL) {
			mg_http_reply(c, 500, headers, "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}\r\n");
			free(mqtt_config);
			return;
		}

		r_w_parameter(mqtt_config, S_MQTT, S_READ);
		pass_len = strlen(mqtt_config->pass);
		memset(mqtt_config->pass, 42,pass_len); // маскируем пароль
		mqtt_config->pass[pass_len] = '\0'; // добавляем null-terminator


		mg_http_reply(c, 200, headers, mqtt_conf_answ, mqtt_config->enable ? "true" : "false",
						mqtt_config->uri, mqtt_config->login, mqtt_config->pass);

		free(mqtt_config);

	}else if(mg_match(hm->method, mg_str("POST"), NULL)){

		MQTT_cred_struct * mqtt_config = (MQTT_cred_struct *) calloc(1, sizeof(MQTT_cred_struct));
		if (mqtt_config == NULL) {
			mg_http_reply(c, 500, headers, "{\"status\":\"error\",\"message\":\"Memory allocation failed\"}\r\n");
			free(mqtt_config);
			return;
		}
		int length;
		if(mg_json_get(hm->body, "$", &length) < 0){//JSON validation err

			mg_http_reply(c, 400, headers, //TODO delete for release,
					"{\"status\":\"error\",\"message\":\"Incorrect JSON\"}\r\n");

		}else{//JSON validation success
			mg_json_get_bool(hm->body, "$.enabled", &mqtt_config->enable);

			char * broker = NULL;
			if ((broker = mg_json_get_str(hm->body, "$.broker"))){
				int err = 0;
				if((err=is_valid_mqtt_url(broker))==0){  //Validate MQTT URI
					strncpy(mqtt_config->uri, broker, MAX_MQTT_URL_LEN -1);
				}else{
					switch (err) {
						case -1:
							mg_http_reply(c, 400, headers, //TODO delete for release,
									"{\"status\":\"error\",\"message\":\"URL does not start with 'mqtt://'\"}\r\n");
							break;
						case -2:
							mg_http_reply(c, 400, headers, //TODO delete for release,
									"{\"status\":\"error\",\"message\":\"URL does not contain a port separator ':'\"}\r\n");
							break;
						case -3:
							mg_http_reply(c, 400, headers, //TODO delete for release,
									"{\"status\":\"error\",\"message\":\"Invalid domain or IP address\"}\r\n");
							break;
						case -4:
							mg_http_reply(c, 400, headers, //TODO delete for release,
									"{\"status\":\"error\",\"message\":\"Invalid port number\"}\r\n");
							break;
						default:
							mg_http_reply(c, 400, headers, //TODO delete for release,
									"{\"status\":\"error\",\"message\":\"Invalid MQTT_URI\"}\r\n");

							break;
					}
					free(broker);
					free(mqtt_config);
					return;
				}

			}
			free(broker);

			char * username = NULL;
			if((username = mg_json_get_str(hm->body, "$.username"))){
				strncpy(mqtt_config->login, username, MAX_MQTT_LOGIN_LEN -1);
			}
			free(username);

			char * password = NULL;
			if((password = mg_json_get_str(hm->body, "$.password"))){
				strncpy(mqtt_config->pass, password, MAX_MQTT_PASS_LEN -1);
			}
			free(password);

			mqtt_config->save = 1;

			r_w_parameter(mqtt_config, S_MQTT, S_WRITE);

			mg_http_reply(c, 200, headers, //TODO delete for release,
				"{\"status\":\"success\",\"message\":\"MQTT settings updated!\"}\r\n");

		}

		free(mqtt_config);

	}else{//error, unsupported method
		mg_http_reply(c, 400, headers, //TODO delete for release,
				"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET, POST methods\"}\r\n");
	}
}

static void handle_dev_status(struct mg_connection *c, struct mg_http_message *hm){

	char   dev_status_template[] = "{\"mqtt_conn_status\": \"%s\", \"local_addr\": \"%s\", \"server_addr\": \"%s\" , \"pub_topics\": [%s], \"subscr_topics\": [%s]}\r\n";
	struct mg_connection* mqtt_connection = NULL;
	int    str_part_len = 30;
	char  *mqtt_status_str   = calloc(str_part_len, sizeof(char));
	char  *mqtt_server_addr  = calloc(IP_v6_STR_LEN + IP_PORT_STR_LEN, sizeof(char));
	char  *mqtt_local_addr   = calloc(IP_v6_STR_LEN + IP_PORT_STR_LEN, sizeof(char));
	char  *pub_topics_str    = calloc((OUTPUTS_COUNT+ INPUTS_COUNT) * MQTT_TOPIK_MAX_LEN, sizeof(char));
	char  *subscr_topics_str = calloc(OUTPUTS_COUNT * MQTT_TOPIK_MAX_LEN, sizeof(char));

	if (mqtt_status_str && mqtt_server_addr && mqtt_local_addr && pub_topics_str && subscr_topics_str){
		if (mg_match(hm->method, mg_str("GET"), NULL)){
			mqtt_connection =  get_mqtt_connection();

			if(!mqtt_connection){
				strncpy(mqtt_status_str, "closed", str_part_len);
			}else{
				if (mqtt_connection->is_connecting){
					strncpy(mqtt_status_str, "connecting", str_part_len);
				}else if(mqtt_connection->is_closing){
					strncpy(mqtt_status_str, "closing",str_part_len);
				}else{
					strncpy(mqtt_status_str, "established", str_part_len);
				}

				mg_snprintf(mqtt_local_addr, IP_v6_STR_LEN + IP_PORT_STR_LEN, "%M", mg_print_ip_port, &mqtt_connection->loc);
				mg_snprintf(mqtt_server_addr, IP_v6_STR_LEN + IP_PORT_STR_LEN, "%M", mg_print_ip_port, &mqtt_connection->rem);

			}

			char topic[MQTT_TOPIK_MAX_LEN];
			strcat(pub_topics_str, "\"");
			// Формируем список топиков состояний для INPUT_SENSOR
			for (int i = 1; i <= INPUTS_COUNT; i++){
				generate_status_topik(topic, MQTT_TOPIK_MAX_LEN, INPUT_SENSOR, i);
				strcat(pub_topics_str, topic);
				strcat(pub_topics_str, "\", \"");
			}

			// Формируем список топиков состояний для OUTPUT_SENSOR
			for (int i = 1; i <= OUTPUTS_COUNT; i++){
				generate_status_topik(topic, MQTT_TOPIK_MAX_LEN, OUTPUT_SENSOR, i);
				strcat(pub_topics_str, topic);
				// Добавляем запятую между топиками, кроме последнего
				if (i < OUTPUTS_COUNT) {
					strcat(pub_topics_str, "\", \"");
				}else{
					strcat(pub_topics_str, "\"");
				}
			}

			strcat(subscr_topics_str, "\"");
			// Формируем список топиков подписки для OUTPUT_SENSOR
			for (int i = 1; i <= OUTPUTS_COUNT; i++){
				generate_comand_topik_for_subscrabe(topic, MQTT_TOPIK_MAX_LEN, OUTPUT_SENSOR, i);
				strcat(subscr_topics_str, topic);
				// Добавляем запятую между топиками, кроме последнего
				if (i < OUTPUTS_COUNT) {
					strcat(subscr_topics_str, "\", \"");
				}else{
					strcat(subscr_topics_str, "\"");
				}
			}


			mg_http_reply(c, 200, headers, //TODO delete for release,
					dev_status_template, mqtt_status_str, mqtt_local_addr, mqtt_server_addr, pub_topics_str, subscr_topics_str);

		}else{//error, unsupported method
			mg_http_reply(c, 400, headers, //TODO delete for release,
					"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET, POST methods\"}\r\n");
		}
	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
				"{\"status\":\"error\",\"message\":\"Memory allocation error\"}\r\n");

	}

	free(mqtt_status_str);
	free(mqtt_server_addr);
	free(mqtt_local_addr);
	free(pub_topics_str);
	free(subscr_topics_str);
}


static void handle_OPTIONS_method(struct mg_connection *c){

	mg_http_reply(c, 200,
	    "Access-Control-Allow-Origin: *\r\n"  // Или конкретный домен
	    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
	    "Access-Control-Allow-Headers: Content-Type\r\n"
	    "Access-Control-Max-Age: 86400\r\n","\r\n");  // Без тела ответа
}

static void handle_restart_mcu(struct mg_connection *c, struct mg_http_message *hm){

	//HAL_NVIC_SystemReset();
    if (mg_match(hm->method, mg_str("POST"), NULL)){
		struct mg_full_net_info * param = (struct mg_full_net_info *)c->fn_data;


		mg_timer_add(param->mgr, 2000 /* 05 seconds */, MG_TIMER_REPEAT, HAL_NVIC_SystemReset, NULL);

		mg_http_reply(c, 200, headers, //TODO delete for release,
							"{\"status\":\"success\",\"message\":\"Restarting...\"}\r\n");

	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
				"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
	}
}

static void handle_firmware_upload(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("POST"), NULL)){
		char file[MG_PATH_MAX];
		mg_http_get_var(&hm->query, "file", file, sizeof(file));
		if(strcmp(file, FIRMWARE_FILE_NAME) == 0){
			mg_http_upload(c, hm, &mg_fs_lfs, "/firmware", FIRMWARE_FILE_MAX_SIZE);
		}else{
			mg_http_reply(c, 400, headers, //TODO delete for release,
							"{\"status\":\"error\",\"message\":\"Incorrect file name. Expected file name %s\"}\r\n", FIRMWARE_FILE_NAME);
		}

	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
	}
}

static void handle_firmware_md5(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("GET"), NULL)){
		char * md5_str = calloc(MD5_STR_LEN, sizeof(char));
		if(calc_file_md5sum("/firmware/firmware.bin", md5_str) == 0){
			mg_http_reply(c, 200, headers, //TODO delete for release,
						"{\"status\":\"success\",\"md5\":\" %s\"}\r\n", md5_str);
		}else{
			mg_http_reply(c, 404, headers, //TODO delete for release,
							"{\"status\":\"error\",\"message\":\"Firmware update file not found\"}\r\n");
		}

		free(md5_str);
	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET method\"}\r\n");
	}
}

static void handle_firmware_activate(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("GET"), NULL)){
		size_t  size;

		if (mg_fs_lfs.st("/firmware.bin", &size, NULL)){ //Check if file exist
			mg_http_reply(c, 200, headers, //TODO delete for release,
								"{\"status\":\"success\", \"message\": \"Firmware activated\"}\r\n");
		}else{
			mg_http_reply(c, 200, headers, //TODO delete for release,
					"{\"status\":\"success\",\"message\": \"Firmware NOT activated\"}\r\n");
		}

	}else if (mg_match(hm->method, mg_str("POST"), NULL)){
	    if (mg_fs_lfs.mv("/firmware/firmware.bin", "/firmware.bin")){
			mg_http_reply(c, 200, headers, //TODO delete for release,
					"{\"status\":\"success\", \"message\": \"Firmware activated\"}\r\n");
	    }else{
			mg_http_reply(c, 500, headers, //TODO delete for release,
					"{\"status\":\"fail\", \"message\": \"Fail during activating firmware\"}\r\n");
	    }

	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET and POST methods\"}\r\n");

	}
}

static void handle_firmware_deactivate(struct mg_connection *c, struct mg_http_message *hm){

	if (mg_match(hm->method, mg_str("POST"), NULL)){
		if (mg_fs_lfs.rm("/firmware.bin")){ //if firmware delete saccess

			mg_http_reply(c, 200, headers, //TODO delete for release,
							"{\"status\":\"success\", \"message\": \"Firmware deactivated\"}\r\n");
				}else{//if error during deleting file
					mg_http_reply(c, 200, headers, //TODO delete for release,
							"{\"status\":\"fail\", \"message\": \"Fail during deactivate firmware\"}\r\n");
				}
	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
	}
}

static void handle_web_files_remove(struct mg_connection *c, struct mg_http_message *hm){

	if (mg_match(hm->method, mg_str("POST"), NULL)){
			mg_fs_lfs.rm("/web");
	        mg_fs_lfs.mkd("/web");
	        mg_http_reply(c, 200, headers, //TODO delete for release,
			    "{\"status\":\"success\",\"message\":\"All web interface files are deleted\"}\r\n");
    }else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
    }
}

// Функция для создания папки
static void handle_fs_mkdir(struct mg_connection *c, struct mg_http_message *hm) {
  char name[123];
  char path[MG_PATH_MAX];

  if (mg_match(hm->method, mg_str("POST"), NULL)){
			// Получаем имя папки из параметра запроса
	  if (mg_http_get_var(&hm->body, "name", name, sizeof(name)) > 0) {
		// Формируем полный путь
		snprintf(path, sizeof(path), "/web/%s", name);

		// Создаем папку
		if (mg_fs_lfs.mkd(path)) {
		  // Если создание успешно, отправляем положительный ответ
		  mg_http_reply(c, 200, headers, //TODO delete for release,
				  "{\"status\":\"success\",\"message\":\"Dir is created\"}\r\n");
		} else {
		  // Если произошла ошибка, отправляем сообщение об ошибке
		  mg_http_reply(c, 500, headers, //TODO delete for release,
				  "{\"status\":\"error\",\"message\":\"Failed to create directory\"}");
		}
	  } else {
		// Если имя папки не передано, отправляем сообщение об ошибке
		mg_http_reply(c, 400, headers, //TODO delete for release,
				"{\"status\":\"error\",\"message\":\"Directory name is required\"}");
	  }
  }else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
  }
}

// function for show logging
// function for show logging
static void handle_fs_get_log(struct mg_connection *c, struct mg_http_message *hm, int number){

	if (mg_match(hm->method, mg_str("GET"), NULL)){

		struct mg_http_serve_opts opts = {
			.root_dir = "/log",
			.fs = &mg_fs_lfs
		};

		switch (number) {
			case 0:
				mg_http_serve_file(c, hm, LOG_FILE_LOCATION, &opts);
				break;
			case 1:
				mg_http_serve_file(c, hm, LOG_FILE_LOCATION_OLD, &opts);
				break;
			default:
				mg_http_serve_file(c, hm, LOG_FILE_LOCATION, &opts);
				break;
		}


	}else if (mg_match(hm->method, mg_str("POST"), NULL)){

	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET and POST methods\"}\r\n");
	}
}

static void dashboard(struct mg_connection *c, int ev, void *ev_data) {

	if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (mg_match(hm->method, mg_str("OPTIONS"), NULL)) {
            	handle_OPTIONS_method(c);
        }else if (mg_match(hm->uri, mg_str("/api/ram/status"), NULL)) { // Get free and allocated RAM space
			handle_ram_status_get(c);
		}else if(mg_match(hm->uri, mg_str("/api/io/status"), NULL)){
			handle_io_status_get(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/mqtt/settings"), NULL)){
			handle_mqtt_conf(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/device/status"), NULL)){
			handle_dev_status(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/device/log"), NULL)){
			handle_fs_get_log(c, hm, 0);
		}else if(mg_match(hm->uri, mg_str("/api/device/log_old"), NULL)){
			handle_fs_get_log(c, hm, 1);
		}else if(mg_match(hm->uri, mg_str("/api/device/restart"), NULL)){
			handle_restart_mcu(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/upload"), NULL)){
			handle_firmware_upload(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/md5"), NULL)){
			handle_firmware_md5(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/activate_and_reboot"), NULL)){
			handle_firmware_activate(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/deactivate"), NULL)){
			handle_firmware_deactivate(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/web_interface/remove"), NULL)){
			 handle_web_files_remove(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/web_interface/upload"), NULL)){
			 mg_http_upload(c, hm, &mg_fs_lfs, "/web", 1500000);
		}else if(mg_match(hm->uri, mg_str("/api/web_interface/mkdir"), NULL)){
			 handle_fs_mkdir(c, hm);
		}else{

			struct mg_http_serve_opts opts = {
				.root_dir = "/web",
				.fs = &mg_fs_lfs
			};
			mg_http_serve_dir(c, ev_data, &opts);
		}
	}
}

void * dash_hdl (void){
	return &dashboard;
}

// Реализация функции, которая принимает указатель на функцию чтения и записи параметров
void reg_parameter_handler(void (*func)(void * parameter, sett_type_t parameter_type,  sett_direction_t direction)) {
    r_w_parameter = func;
}

