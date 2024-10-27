/*
 * mqtt.hpp
 *
 *  Created on: Oct 14, 2024
 *      Author: kiro
 */

#ifndef APP_MQTT_CLIENT_APP_INC_MQTT_HPP_
#define APP_MQTT_CLIENT_APP_INC_MQTT_HPP_

#include "mongoose.h"
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mg_connection* get_mqtt_connection(void);
void mqtt_init(void *mgr_parameter, void * mif_parameter, void * broker_url, void *username, void *password, void (*fn)(void*,sett_type_t,  sett_direction_t));


#ifdef __cplusplus
}
#endif

#endif /* APP_MQTT_CLIENT_APP_INC_MQTT_HPP_ */
