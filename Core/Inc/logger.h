/*
 * logger.h
 *
 *  Created on: Nov 7, 2024
 *      Author: kiro
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "data_types.h"
#include <stdarg.h>  // Для работы с va_list и обработкой переменных аргументов
#include <stdio.h>


typedef enum logging_level{
	L_DEBUG,
	L_INFO,
	L_WARN,
	L_ERR
}logging_level_t;

typedef struct log_message{
	logging_level_t log_level;
	char log_text[LOG_MES_TEXT_LEN];
	uint8_t log_len;
}log_message_t;

void reg_logging_fn(void (* fn)(struct log_message));
void logger_set_level(logging_level_t l);
void logging(logging_level_t level, const char *format, ...);

#ifdef __cplusplus
}
#endif


#endif /* INC_LOGGER_H_ */
