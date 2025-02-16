/*
 * sntp.c
 *
 *  Created on: Jul 31, 2024
 *      Author: kirill
 */


#include "sntp.h"

#include "mongoose.h"
#include "logger.h"
#include <inttypes.h>
#include "rtc.h"
#include <time.h>


static time_t s_boot_timestamp = 0;

// SNTP client connection
static struct mg_connection *s_sntp_conn = NULL;


// Function to update RTC from SNTP time
void update_rtc_from_ntp(uint64_t unix_ms) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    time_t unix_time = unix_ms / 1000;  // Convert milliseconds to seconds
    struct tm t;
    gmtime_r(&unix_time, &t);  // Parse time into the tm structure

    // Fill the RTC time structure
    sTime.Hours = t.tm_hour;
    sTime.Minutes = t.tm_min;
    sTime.Seconds = t.tm_sec;

    // Fill the RTC date structure
    sDate.Year = t.tm_year - 100; // STM32 counts years from 2000, while tm_year starts from 1900
    sDate.Month = t.tm_mon + 1;   // tm_mon [0..11], RTC [1..12]
    sDate.Date = t.tm_mday;
    sDate.WeekDay = t.tm_wday == 0 ? 7 : t.tm_wday; // In STM32, Sunday = 7

    // Set the time in RTC
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        logging(L_ERR, "Failed to set RTC time");
    }

    // Set the date in RTC
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        logging(L_ERR, "Failed to set RTC date");
    }
    //set RTC init flag
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
    logging(L_INFO, "RTC updated: %04d-%02d-%02d %02d:%02d:%02d",
            sDate.Year + 2000, sDate.Month, sDate.Date,
            sTime.Hours, sTime.Minutes, sTime.Seconds);
}

// SNTP client callback
static void sfn(struct mg_connection *c, int ev, void *ev_data) {
	if (ev == MG_EV_SNTP_TIME) {
		logging(L_INFO, "Response from NTP server received");
		// Time received, the internal protocol handler updates what mg_now() returns
		uint64_t curtime = mg_now();
		MG_INFO(("SNTP-updated current time is: %llu ms from epoch", curtime));
		logging(L_INFO,"SNTP-updated current time is: %" PRIu64 " ms from epoch", curtime);
		// otherwise, you can process the server returned data yourself
		uint64_t t = *(uint64_t *) ev_data;
		s_boot_timestamp = (time_t) ((t - mg_millis()) / 1000);
		MG_INFO(("Got SNTP time: %llu ms from epoch, ", t));
		logging(L_INFO,"Got SNTP time: %" PRIu64 " ms from epoch", t);

		update_rtc_from_ntp(curtime);

	} else if (ev == MG_EV_CLOSE) {
		logging(L_INFO, "NTP connection is closed");
		s_sntp_conn = NULL;
	}
	(void) c;
}


void timer_fn(void *arg) {
	struct mg_mgr *mgr = (struct mg_mgr *) arg;
	if (s_sntp_conn == NULL) s_sntp_conn = mg_sntp_connect(mgr, NULL, sfn, NULL);
	if (s_sntp_conn != NULL) mg_sntp_request(s_sntp_conn);
}

void start_sntp(void *arg){
	logging(L_INFO, "starting NTP client service");
	struct mg_mgr *mgr = (struct mg_mgr*)arg;

	mg_timer_add(mgr, 20000,  MG_TIMER_ONCE, timer_fn, mgr); //Run once with delay 20 seconds
	mg_timer_add(mgr, 4000000, MG_TIMER_REPEAT, timer_fn, mgr); //Repeat every ~ 60 minutes

}
