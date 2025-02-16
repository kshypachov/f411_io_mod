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
#include "rtc.h"  // Include RTC header

//#include "mongoose.h"

time_t get_rtc_timestamp_ms(void);
//#define time_ms_now() mg_now()
#define time_ms_now() get_rtc_timestamp_ms()
logging_level_t _level_ = L_DEBUG;
const char *level_strings[] = {"DEBUG", "INFO", "WARNING", "ERROR"};

static void proto(struct log_message mess ){};

void (*_send_log_mess)(struct log_message mess) = proto;



// Function to get current timestamp from RTC
time_t get_rtc_timestamp_ms(void) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    // Read the current date and time from RTC
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Convert RTC date and time to UNIX timestamp
    struct tm t;
    memset(&t, 0, sizeof(struct tm));

    t.tm_year = sDate.Year + 100;  // Year since 1900 (RTC year starts from 2000)
    t.tm_mon = sDate.Month - 1;    // RTC month is 1-12, struct tm month is 0-11
    t.tm_mday = sDate.Date;
    t.tm_hour = sTime.Hours;
    t.tm_min = sTime.Minutes;
    t.tm_sec = sTime.Seconds;

    time_t timestamp = mktime(&t); // Convert to UNIX timestamp

    // Add milliseconds from RTC subseconds
    uint32_t ms = 1000 - ((sTime.SubSeconds * 1000) / sTime.SecondFraction + 1);

    return (timestamp * 1000) + ms;
}



void reg_logging_fn(void (* fn)(struct log_message)){
	if (fn != NULL){
		_send_log_mess = fn;
	}
}

void logger_set_level(logging_level_t l){
	_level_ = l;
}

void logging(logging_level_t level, const char *format, ...){
    log_message_t mess;
    va_list args;

    int formatted_len;
    int time_len;

    char * tmp_buf = NULL;

    if (level < _level_) return;

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

    snprintf(tmp_buf ,LOG_MES_TEXT_LEN, "%s %s:%s\r\n", time_str, level_strings[mess.log_level], mess.log_text);
    // Проверка длины отформатированной строки
     if (formatted_len >= 0 && formatted_len < LOG_MES_TEXT_LEN - 10) {
         mess.log_len = strlen(tmp_buf);
     } else {
         mess.log_len = LOG_MES_TEXT_LEN - 10;
     }

     strncpy(mess.log_text, tmp_buf, LOG_MES_TEXT_LEN);

    // Проверка на существование функции перед вызовом
    if (_send_log_mess) {
        _send_log_mess(mess);
    }

    free(tmp_buf);
}
