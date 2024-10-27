/*
 * dashboard.h
 *
 *  Created on: Oct 5, 2024
 *      Author: kirill
 */

#ifndef APP_HTTP_SERVER_APP_INC_DASHBOARD_H_
#define APP_HTTP_SERVER_APP_INC_DASHBOARD_H_

#include "data_types.h"

void * dash_hdl (void);
void reg_parameter_handler(void (*func)(void * parameter, sett_type_t parameter_type,  sett_direction_t direction));

#endif /* APP_HTTP_SERVER_APP_INC_DASHBOARD_H_ */
