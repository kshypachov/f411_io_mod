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


static time_t s_boot_timestamp = 0;

// SNTP client connection
static struct mg_connection *s_sntp_conn = NULL;

// SNTP client callback
static void sfn(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_SNTP_TIME) {
	  logging(L_INFO, "Response from NTP server received");
    // Time received, the internal protocol handler updates what mg_now() returns
    uint64_t curtime = mg_now();
    MG_INFO(("SNTP-updated current time is: %llu ms from epoch", curtime));
    logging(L_INFO,"SNTP-updated current time is: %" PRIu64 " ms from epoch", curtime);
    // otherwise, you can process the server returned data yourself
    {
      uint64_t t = *(uint64_t *) ev_data;
      s_boot_timestamp = (time_t) ((t - mg_millis()) / 1000);
      MG_INFO(("Got SNTP time: %llu ms from epoch, ", t));
      logging(L_INFO,"Got SNTP time: %" PRIu64 " ms from epoch", t);
    }
  } else if (ev == MG_EV_CLOSE) {
	  logging(L_ERR, "NTP connection is closed");
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
	mg_timer_add(mgr, 1080000, MG_TIMER_REPEAT, timer_fn, mgr); //Repeat every 30 minutes 1080 seconds

}
