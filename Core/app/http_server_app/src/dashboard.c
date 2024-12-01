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


//TEST---AUTH---------------------------------
#define user_name 			WEB_ADMIN_DEFAULT
#define user_cred_file		WEB_USERS_FILE





users_list_t  web_users[user_list_size]= {0};
tokens_list_t web_tokens[user_list_size]= {0};


static bool load_users(const char *file_path, users_list_t * users, size_t count){
	struct mg_fd *fd = mg_fs_open(&mg_fs_lfs, file_path, MG_FS_READ);
	if (!fd) {
		logging(L_ERR, "Can't open file with users");
		return false;
	}

	mg_fs_lfs.rd(fd->fd, users, sizeof(users_list_t) * count);
	mg_fs_close(fd);
	logging(L_INFO, "Loaded web users from file");
	return true;
}

static void write_users(const char *file_path, users_list_t * users, size_t count){
	mg_fs_lfs.rm(file_path);
	struct mg_fd *fd = mg_fs_open(&mg_fs_lfs, file_path, MG_FS_WRITE);

	mg_fs_lfs.wr(fd->fd, users, sizeof(users_list_t) * count);
	mg_fs_close(fd);
	logging(L_INFO, "Saved web users to file");
}


static void handle_login(struct mg_connection *c, tokens_list_t *u) {
  char cookie[256];
  const char *cookie_name = c->is_tls ? "secure_access_token" : "access_token";
  mg_snprintf(cookie, sizeof(cookie),
              "Set-Cookie: %s=%s; Path=/; "
              "%sHttpOnly; SameSite=Lax; Max-Age=%d\r\n",
              cookie_name, u->token,
              c->is_tls ? "Secure; " : "", token_life_time / 1000);
  mg_http_reply(c, 200, cookie, "{%m:%m}", MG_ESC("user"), MG_ESC(u->user));
  logging(L_INFO, "Authenticated web user: %s ", u->user);
}

static bool auth(struct mg_connection *c, struct mg_http_message *hm, bool set_cookie){
	//users_list_t tmp_user_pass;
	int i;
	char pass_digest[sha1_str_len] = {0};
	mg_sha1_ctx sha1_ctx;
	char username[128];
	char pass[128];

	mg_http_creds(hm, username, sizeof(username), pass, sizeof(pass));

	if (username[0] != '\0' && pass[0] != '\0') {
		logging(L_INFO, "Try to authenticate user: %s", username);
	// Both user and password is set, search by user/password
		mg_sha1_init(&sha1_ctx);
		mg_sha1_update(&sha1_ctx, (const unsigned char *)pass, strlen(pass) * sizeof(char));
		mg_sha1_final((unsigned char *)&pass_digest, &sha1_ctx); // calculate sha1 pass
		for (i=0; i< user_list_size; i++ ){
			if((strcmp(web_users[i].user, username) == 0) && (strcmp(web_users[i].pass_digest, pass_digest) == 0)){
				for(int k=0; k < user_list_size; k++){
					if(web_tokens[i].expare < mg_now()){
						strncpy(web_tokens[i].user, username, sizeof(web_tokens[i].user));
						web_tokens[i].expare = mg_now() + token_life_time;
						mg_random_str(web_tokens[i].token, sha1_str_len -1);
						if (set_cookie)handle_login(c, &web_tokens[i]);
						logging(L_INFO, "User: %s is authenticated", web_tokens[i].user);

						return true;
					}
					strncpy(web_tokens[1].user, username, sizeof(web_tokens[1].user));
					web_tokens[1].expare = mg_now() + token_life_time;
					mg_random_str(web_tokens[1].token, sha1_str_len -1);
					if (set_cookie)handle_login(c, &web_tokens[1]);
					logging(L_INFO, "User: %s is authenticated", web_tokens[i].user);

					return true;
				}
			}
		}

	} else if (username[0] == '\0' && pass[0] != '\0') {
	// Only password is set, search by token
		for (i=0;i<user_list_size; i++){
			if((strcmp(web_tokens[i].token, pass) == 0 && web_tokens[i].user[0] != 0)){// && (web_tokens[i].expare > mg_now())
				if (set_cookie) mg_http_reply(c, 200, headers,
				        		"{\"status\":\"success\",\"message\":\"Token is valid\", \"user\": \"%s\"}\r\n", web_tokens[i].user);

				return true;
			}
		}
	}
	return false;
}


static void handler_authanticate(struct mg_connection *c,struct mg_http_message *hm){
	if ( auth(c, hm, 1) ){

//        mg_http_reply(c, 200, headers, //TODO delete for release,
//        		"{\"status\":\"OK\",\"message\":\"Auth completed\"}\r\n");

	}else{
        mg_http_reply(c, 403, headers, //TODO delete for release,
        		"{\"status\":\"error\",\"message\":\"Access deny\"}\r\n");

	}
}

static void handler_logout(struct mg_connection *c,struct mg_http_message *hm){

	char username[128] = {0};
	char pass[128] = {0};
	int i;

	mg_http_creds(hm, username, sizeof(username), pass, sizeof(pass));

	if (username[0] != '\0' && pass[0] != '\0') {
        mg_http_reply(c, 400, headers, //TODO delete for release,
        		"{\"status\":\"error\",\"message\":\"Not found authentication cookie. Provided login and password but expected cookie access token\"}\r\n");
        return;


	}else if (username[0] == '\0' && pass[0] != '\0') {
		for (i=0;i<user_list_size; i++){
			if((strcmp(web_tokens[i].token, pass) == 0)){
				logging(L_INFO, "User: %s is logout", web_tokens[i].user);
				web_tokens[i].token[0] = '\0';
				web_tokens[i].expare = 0;
				web_tokens[i].user[0] = '\0';

		        mg_http_reply(c, 200, headers, //TODO delete for release,
		        		"{\"status\":\"success\",\"message\":\"User successfully logout\"}\r\n");
		        return;

			}
		}

        mg_http_reply(c, 404, headers,
                      "{\"status\":\"error\",\"message\":\"Token not found\"}\r\n");
        return;


	}else{
        mg_http_reply(c, 404, headers, //TODO delete for release,
        		"{\"status\":\"error\",\"message\":\"Not found authentication cookie. For logout please provide cookie with access token\"}\r\n");
	}
}

static void handle_ram_status_get(struct mg_connection *c){
	HeapStats_t heap_status;
	vPortGetHeapStats(&heap_status);

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


		mg_timer_add(param->mgr, 2000 /* 2 seconds */, MG_TIMER_REPEAT, HAL_NVIC_SystemReset, NULL);

		mg_http_reply(c, 200, headers, //TODO delete for release,
							"{\"status\":\"success\",\"message\":\"Device is restarting\"}\r\n");

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
			vTaskDelay(1);
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
						"{\"status\":\"success\",\"md5\":\"%s\"}\r\n", md5_str);
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

static void handle_activete_and_reboot(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("POST"), NULL)){
		if (mg_fs_lfs.mv("/firmware/firmware.bin", "/firmware.bin")){
			handle_restart_mcu(c, hm);

		}else{
			mg_http_reply(c, 500, headers, //TODO delete for release,
					"{\"status\":\"fail\", \"message\": \"Fail during activating firmware\"}\r\n");
		}
	}else{
		mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only POST method\"}\r\n");
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

static void handle_manage_user(struct mg_connection *c, struct mg_http_message *hm){

	if (mg_match(hm->method, mg_str("GET"), NULL)){

		uint32_t response_len = 1024;
		uint32_t user_entry_len = 160;
	    char *response = calloc(response_len, sizeof(char));
	    char *user_entry = calloc(user_entry_len, sizeof(char));
	    size_t offset = 0;

	    if (response && user_entry) {
	        // Начало JSON-объекта
	        offset += snprintf(response + offset, response_len - offset, "{ \"users\": [");

	        // Проход по всем пользователям
	        for (int i = 0; i < user_list_size; i++) {
	            if (web_users[i].user[0] != '\0') {  // Проверяем, что имя пользователя не пустое
	                // Формируем JSON-объект для пользователя
	                snprintf(user_entry, user_entry_len, "{\"user\": \"%s\"}", web_users[i].user);

	                // Добавляем запятую между объектами (но не перед первым)
	                if (offset > 12) { // 12 символов — это длина строки "{ \"users\": ["
	                    offset += snprintf(response + offset, response_len - offset, ",");
	                }

	                // Добавляем объект пользователя в JSON
	                offset += snprintf(response + offset, response_len - offset, "%s", user_entry);
	            }
	        }

	        // Завершаем JSON-объект
	        offset += snprintf(response + offset, response_len - offset, "]}");

	        // Отправляем JSON-ответ
	        mg_http_reply(c, 200, headers, "%s", response);
	    } else {
	        // Если память не выделена, отправляем ошибку
	    	logging(L_ERR, "Memory allocation failed in handle_manage_user GET");
	        mg_http_reply(c, 500, headers, "{\"error\": \"Memory allocation failed\"}");
	    }

	    // Освобождаем память
	    free(response);
	    free(user_entry);

	    return;

	}else if (mg_match(hm->method, mg_str("POST"), NULL)){

		char *username_add = NULL;
		char *password_add = NULL;
		int length;

	    if (mg_json_get(hm->body, "$", &length) < 0) {
	        // JSON validation error
	        mg_http_reply(c, 400, headers,
	                      "{\"status\":\"error\",\"message\":\"Incorrect JSON\"}\r\n");
	        return;
	    }

	    username_add = mg_json_get_str(hm->body, "$.user");
	    password_add = mg_json_get_str(hm->body, "$.pass");


	    if (username_add && password_add && strlen(username_add) > 0 && strlen(password_add) > 0) {
	        // Проверяем, существует ли пользователь с таким именем
	        for (int i = 0; i < user_list_size; i++) {
	            if (strcmp(web_users[i].user, username_add) == 0) {
	                // Имя пользователя уже существует
	                mg_http_reply(c, 400, headers,
	                              "{\"status\":\"error\",\"message\":\"Username already exists\"}\r\n");
	                free(username_add);
	                free(password_add);
	                return;
	            }
	        }

			// Вычисляем SHA1 хэш для пароля
			char add_pass_digest[sha1_str_len] = {0};
			mg_sha1_ctx sha1_ctx;
			mg_sha1_init(&sha1_ctx);
			mg_sha1_update(&sha1_ctx, (const unsigned char *)password_add, strlen(password_add) * sizeof(char));
			mg_sha1_final((unsigned char *)add_pass_digest, &sha1_ctx);

	        // Ищем свободное место в массиве web_users
	        bool user_added = false;
	        for (int i = 0; i < user_list_size; i++) {
	            if (web_users[i].user[0] == '\0') { // Пустая запись
	                // Копируем данные в массив
	                strncpy(web_users[i].user, username_add, sizeof(web_users[i].user) - 1);
	                strncpy(web_users[i].pass_digest, add_pass_digest, sizeof(web_users[i].pass_digest) - 1);
	                write_users(user_cred_file, web_users, user_list_size);
	                user_added = true;
	                break;
	            }
	        }

	        if (user_added) {
	            // Успешный ответ
	            mg_http_reply(c, 200, headers,
	                          "{\"status\":\"success\",\"message\":\"User added successfully\"}\r\n");
	        } else {
	            // Нет свободного места в массиве
	            mg_http_reply(c, 400, headers,
	                          "{\"status\":\"error\",\"message\":\"User list is full\"}\r\n");
	        }
	    } else {
	        // Неполные данные
	        mg_http_reply(c, 400, headers,
	                      "{\"status\":\"error\",\"message\":\"Field user or pass is empty\"}\r\n");
	    }

		// Освобождаем память
		free(username_add);
		free(password_add);

		return;

	}else if (mg_match(hm->method, mg_str("PUT"), NULL)){

	    char *username_edit = NULL;
	    char *new_password = NULL;
	    int length;

	    if (mg_json_get(hm->body, "$", &length) < 0) {
	        // JSON validation error
	        mg_http_reply(c, 400, headers,
	                      "{\"status\":\"error\",\"message\":\"Incorrect JSON\"}\r\n");
	    } else {
	        // Extract username and new password
	        username_edit = mg_json_get_str(hm->body, "$.user");
	        new_password = mg_json_get_str(hm->body, "$.pass");

	        if (username_edit && new_password && strlen(username_edit) > 0 && strlen(new_password) > 0) {
	            bool user_found = false;

	            // Calculate SHA1 hash for the new password
	            char new_pass_digest[sha1_str_len] = {0};
	            mg_sha1_ctx sha1_ctx;
	            mg_sha1_init(&sha1_ctx);
	            mg_sha1_update(&sha1_ctx, (const unsigned char *)new_password, strlen(new_password));
	            mg_sha1_final((unsigned char *)new_pass_digest, &sha1_ctx);

//	    		mg_sha1_init(&sha1_ctx);
//	    		mg_sha1_update(&sha1_ctx, (const unsigned char *)pass, strlen(pass) * sizeof(char));
//	    		mg_sha1_final((unsigned char *)&pass_digest, &sha1_ctx); // calculate sha1 pass


	            // Search for the user in the array
	            for (int i = 0; i < user_list_size; i++) {
	                if (strcmp(web_users[i].user, username_edit) == 0) {
	                    // Update the password hash
	                    strncpy(web_users[i].pass_digest, new_pass_digest, sizeof(web_users[i].pass_digest) - 1);
	                    web_users[i].pass_digest[sizeof(web_users[i].pass_digest) - 1] = '\0';
	                    write_users(user_cred_file, web_users, user_list_size);
	                    user_found = true;
	                    break;
	                }
	            }

	            if (user_found) {
	                mg_http_reply(c, 200, headers,
	                              "{\"status\":\"success\",\"message\":\"Password updated successfully\"}\r\n");
	            } else {
	                mg_http_reply(c, 404, headers,
	                              "{\"status\":\"error\",\"message\":\"User not found\"}\r\n");
	            }
	        } else {
	            mg_http_reply(c, 400, headers,
	                          "{\"status\":\"error\",\"message\":\"Field user or new_pass is empty\"}\r\n");
	        }
	    }

	    // Free allocated memory
	    free(username_edit);
	    free(new_password);

	    return;


	}else if (mg_match(hm->method, mg_str("DELETE"), NULL)){
		char *username_del = NULL;

	    username_del = mg_json_get_str(hm->body, "$.user");

	    if (username_del && strlen(username_del) > 0) {
	        bool user_found = false;

	        for (int i = 1; i < user_list_size; i++) { // Нельзя удалять пользователя под индексом 0

	            if (strcmp(web_users[i].user, username_del) == 0) {
	                // Удаляем пользователя, очищая запись
	                memset(web_users[i].user, 0, sizeof(web_users[i].user));
	                memset(web_users[i].pass_digest, 0, sizeof(web_users[i].pass_digest));

	                write_users(user_cred_file, web_users, user_list_size);
	                user_found = true;
	                break;
	            }
	        }

	        if (user_found) {
	            // Успешный ответ
	            mg_http_reply(c, 200, headers,
	                          "{\"status\":\"success\",\"message\":\"User deleted successfully\"}\r\n");
	        } else {
	            // Пользователь не найден
	            mg_http_reply(c, 400, headers,
	                          "{\"status\":\"error\",\"message\":\"User not found or cannot delete user at index 0\"}\r\n");
	        }
	    } else {
	        // Неполные данные
	        mg_http_reply(c, 400, headers,
	                      "{\"status\":\"error\",\"message\":\"Field user is empty or missing\"}\r\n");
	    }

	    // Освобождаем память
	    free(username_del);

	    return;

	}else{
		logging(L_ERR, "Call API /api/device/user 400 error");
		char * method = calloc(hm->method.len + 2, sizeof(char));
		if (method){
			strncpy(method, hm->method.buf, hm->method.len);

			mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method: %s, support only GET POST PUT DELETE methods\"}\r\n", method);
		}else{
			mg_http_reply(c, 400, headers, //TODO delete for release,
						"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET POST PUT DELETE methods\"}\r\n", method);

		}

		free(method);

		return;
	}
}


static void dashboard(struct mg_connection *c, int ev, void *ev_data) {

	if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    bool authenticate = auth(c, hm, 0); //Check authentication status

        if (mg_match(hm->method, mg_str("OPTIONS"), NULL)) {
            	handle_OPTIONS_method(c);
        }else if (mg_match(hm->uri, mg_str("/api/#"), NULL) && !authenticate ) { // All requests to /api should be authenticated
        	mg_http_reply(c, 403, "", "Not Authorised\n");
		}else if (mg_match(hm->uri, mg_str("/api/login"), NULL)) {
			 logging(L_INFO, "Call API /api/login");
        	 handler_authanticate(c,hm);
		}else if (mg_match(hm->uri, mg_str("/api/logout"), NULL)) {
			logging(L_INFO, "Call API /api/logout");
			handler_logout(c,hm);
		}else if (mg_match(hm->uri, mg_str("/api/ram/status"), NULL)) { // Get free and allocated RAM space
			logging(L_INFO, "Call API /api/ram/status");
			handle_ram_status_get(c);
		}else if(mg_match(hm->uri, mg_str("/api/io/status"), NULL)){
			logging(L_INFO, "Call API /api/io/status");
			handle_io_status_get(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/mqtt/settings"), NULL)){
			logging(L_INFO, "Call API /api/mqtt/settings");
			handle_mqtt_conf(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/device/status"), NULL)){
			logging(L_INFO, "Call API /api/device/status");
			handle_dev_status(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/device/log"), NULL)){
			logging(L_INFO, "Call API /api/device/log");
			handle_fs_get_log(c, hm, 0);
		}else if(mg_match(hm->uri, mg_str("/api/device/log_old"), NULL)){
			logging(L_INFO, "Call API /api/device/log_old");
			handle_fs_get_log(c, hm, 1);
		}else if(mg_match(hm->uri, mg_str("/api/device/user"), NULL)){
			logging(L_INFO, "Call API /api/device/user");
			handle_manage_user(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/device/restart"), NULL)){
			logging(L_INFO, "Call API /api/device/restart");
			handle_restart_mcu(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/upload"), NULL)){
			handle_firmware_upload(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/md5"), NULL)){
			logging(L_INFO, "Call API /api/firmware/md5");
			handle_firmware_md5(c, hm);
		}else if(mg_match(hm->uri, mg_str("/api/firmware/apply"), NULL)){
			logging(L_INFO, "Call API /api/firmware/apply");
			handle_activete_and_reboot(c, hm);
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

	load_users(user_cred_file, web_users, user_list_size);

	return &dashboard;
}

// Реализация функции, которая принимает указатель на функцию чтения и записи параметров
void reg_parameter_handler(void (*func)(void * parameter, sett_type_t parameter_type,  sett_direction_t direction)) {
    r_w_parameter = func;
}

