/*
 * logger.c
 *
 *  Created on: Nov 7, 2024
 *      Author: kiro
 */


#include "logger.h"
#include "definitions.h"
#include "data_types.h"
#include "time.h"

#include "mongoose.h"

#define time_ms_now() mg_now()

static void proto(struct log_message mess ){};

void (*_send_log_mess)(struct log_message mess) = proto;


void reg_logging_fn(void (* fn)(struct log_message)){
	if (fn != NULL){
		_send_log_mess = fn;
	}
}

void logging(uint8_t level, const char *format, ...){
    log_message_t mess;
    va_list args;

    int formatted_len;
    int time_len;

    char * tmp_buf = NULL;
    tmp_buf = calloc(LOG_MES_TEXT_LEN, sizeof(char));

    if (!tmp_buf){
    	return;
    }

    // Инициализация структуры нулями
    memset(&mess, 0, sizeof(log_message_t));

    // Установка уровня логирования
    mess.log_level = level;


    // Получение текущего времени в миллисекундах
    double ms_now = time_ms_now();

    // Преобразование миллисекунд в секунды для time_t
    time_t now = (time_t)(ms_now / 1000.0);

    // Получение локального времени
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    // Форматирование времени
    char time_str[64];
    time_len = strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

    // Вычисление миллисекунд
    int millis = (int)(ms_now) % 1000;

    // Добавление миллисекунд к строке времени
    time_len += snprintf(time_str + time_len, sizeof(time_str) - time_len, ".%03d", millis);

    // Добавление разделителя, например, пробела
    time_str[time_len++] = ' ';
    time_str[time_len++] = '\0';
    time_len += 1;

    // Инициализация списка аргументов
    va_start(args, format);

    // Форматирование строки с ограничением на размер log_text
    formatted_len = vsnprintf(mess.log_text, LOG_MES_TEXT_LEN - 3, format, args);

    // Форматирование строки сообщения, начиная с mess.log_text + time_len
    //formatted_len = vsnprintf(mess.log_text, LOG_MES_TEXT_LEN  - 3, format, args);

    // Завершение работы со списком аргументов
    va_end(args);

    snprintf(tmp_buf ,LOG_MES_TEXT_LEN, "%s %s\r\n", time_str, mess.log_text);
    // Проверка длины отформатированной строки
     if (formatted_len >= 0 && formatted_len < LOG_MES_TEXT_LEN - 3) {
         mess.log_len = time_len + formatted_len;
     } else {
         mess.log_len = LOG_MES_TEXT_LEN - 3;
     }

     strncpy(mess.log_text, tmp_buf, LOG_MES_TEXT_LEN);
     // Добавление символов \r\n\0
//     mess.log_text[mess.log_len] = '\r';
//     mess.log_text[mess.log_len + 1] = '\n';
//     mess.log_text[mess.log_len + 2] = '\0';
//     mess.log_len += 2;  // Обновление длины для учёта \r\n

    // Проверка на существование функции перед вызовом
    if (_send_log_mess) {
        _send_log_mess(mess);
    }

    free(tmp_buf);
}
