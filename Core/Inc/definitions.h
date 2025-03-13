/*
 * definitions.h
 *
 *  Created on: Sep 28, 2024
 *      Author: kirill
 */

#ifndef INC_DEFINITIONS_H_
#define INC_DEFINITIONS_H_

#define OUTPUTS_FIRST_INDEX     1
#define OUTPUTS_COUNT			3

#define INPUTS_FIRST_INDEX		1
#define INPUTS_COUNT			3

#define MAX_DOMAIN_NAME_LEN   	128

#define dev_hw_ver				"0.3"
#define dev_sw_ver				"2.4"

#define FIRMWARE_FILE_NAME		"firmware.bin"
#define FIRMWARE_FILE_MAX_SIZE  250000

#define MD5_STR_LEN             33

/*----------------MQTT-------------------------------*/
#define MQTT_TOPIK_MAX_LEN			128
#define MQTT_PAYLOAD_MAX_LEN		600
#define MAX_MQTT_LOGIN_LEN			33
#define MAX_MQTT_PASS_LEN			33
#define MAX_MQTT_URL_LEN			MAX_DOMAIN_NAME_LEN
#define DEV_UID_LEN					14
#define IP_v4_LEN					18
#define IP_v6_STR_LEN               40
#define IP_PORT_STR_LEN				6


#define MQTT_CONF_FILE				"mqtt.conf"

#define LOG_MES_TEXT_LEN    		256
#define LOG_FILE_LOCATION			"/log/main.log"
#define LOG_FILE_LOCATION_OLD		"/log/main1.log"
#define LOG_FILE_MAX_SIZE			50000


#define user_list_size				10
#define sha1_str_len				41
#define token_life_time				172800000 // 48 hour
#define WEB_USER_LEN    			128
#define WEB_PASS_LEN				128
#define WEB_USERS_FILE				"/auth/pass.conf"
#define WEB_ADMIN_DEFAULT   		"admin"
#define	WEB_ADMIN_PASS_DIGEST		{ 208, 51, 226, 42, 227, 72, 174, 181, 102, 15, 194, 20, 10, 236, 53, 133, 12, 77, 169, 151, 0 } // Digest for password "admin"

#define ACL_FILE					"/settings/mb.acl"
#define ACL_FILE_MAX_SIZE			1024

/*-------------------------RE define mem managment functins ---*/

#include "FreeRTOS.h"
#include <string.h>


#define calloc(a, b) my_calloc(a, b)
#define free(a) vPortFree(a)
#define malloc(a) pvPortMalloc(a)

// Re-route calloc/free to the FreeRTOS's functions, don't use stdlib
static inline void *my_calloc(size_t cnt, size_t size) {
  void *p = pvPortMalloc(cnt * size);
  if (p != NULL) memset(p, 0, size * cnt);
  return p;
}

#endif /* INC_DEFINITIONS_H_ */
