/*
 * mb_tcp_app.c
 *
 *  Created on: Dec 2, 2024
 *      Author: kiro
 */

//
//#include "mb_tcp_app.h"
//
//
//#include "mb.h"
//#include "mbport.h"
//#include "mb_add.h"
//
//#ifndef MB_TCP_BUF_SIZE
//	#define MB_TCP_BUF_SIZE  2048
//#endif
//
//
//
//static const char *s_lsn = "tcp://0.0.0.0:502";   // Listening address ModBusTCP
//
////static void handler_mb_tcp(struct mg_connection *c, int ev, void *ev_data){
////
////	if (ev == MG_EV_READ) {
////		xMBPortEventPost (EV_FRAME_RECEIVED);
////	}
////}
////
////
////void init_mb_tcp(void * param){
////
////	struct mg_mgr *mgr = (struct mg_mgr *)param;  // Event manager
////	mg_listen(mgr, s_lsn, handler_mb_tcp, NULL);  // Create server connection
////
////}
