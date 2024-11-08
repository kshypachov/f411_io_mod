/*
 * logger.c
 *
 *  Created on: Nov 7, 2024
 *      Author: kiro
 */


#include "logger.h"
#include "definitions.h"
#include "data_types.h"

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

    // Инициализация структуры нулями
    memset(&mess, 0, sizeof(log_message_t));

    // Установка уровня логирования
    mess.log_level = level;

    // Инициализация списка аргументов
    va_start(args, format);

    // Форматирование строки с ограничением на размер log_text
    formatted_len = vsnprintf(mess.log_text, LOG_MES_TEXT_LEN - 3, format, args);

    // Завершение работы со списком аргументов
    va_end(args);

    // Проверка длины отформатированной строки
     if (formatted_len >= 0 && formatted_len < LOG_MES_TEXT_LEN - 3) {
         mess.log_len = formatted_len;
     } else {
         mess.log_len = LOG_MES_TEXT_LEN - 3;
     }

     // Добавление символов \r\n\0
     mess.log_text[mess.log_len] = '\r';
     mess.log_text[mess.log_len + 1] = '\n';
     mess.log_text[mess.log_len + 2] = '\0';
     mess.log_len += 2;  // Обновление длины для учёта \r\n

    // Проверка на существование функции перед вызовом
    if (_send_log_mess) {
        _send_log_mess(mess);
    }
}
