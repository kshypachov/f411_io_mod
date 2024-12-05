#include "port.h"
#include <string.h>
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mongoose.h"
#include "mb_tcp_app.h"
#include <string.h>
#include "definitions.h"
#include "fs_adstractions.h"

#ifndef MB_TCP_BUF_SIZE
	#define MB_TCP_BUF_SIZE  2048
#endif

#define DEFAULT_ACL			"-0.0.0.0/0"

uint8_t ucTCPRequestFrame [MB_TCP_BUF_SIZE];
uint16_t ucTCPRequestLen;
uint8_t ucTCPResponseFrame [MB_TCP_BUF_SIZE];
uint16_t ucTCPResponseLen;
uint8_t bFrameSent = FALSE;

struct mg_str mg_acl;
static const char *s_lsn = "tcp://0.0.0.0:502";   // Listening address ModBusTCP




//mg_check_ip_acl(struct mg_str acl, struct mg_addr *remote_ip);
static void handler_mb_tcp(struct mg_connection *c, int ev, void *ev_data){

	if (ev == MG_EV_READ){
		if (mg_check_ip_acl(mg_acl, &c->rem) != 1){
			c->is_closing = 1;
			return;
		}
		struct mg_iobuf *r = &c->recv;

		ucTCPRequestLen = r->len;
		if(ucTCPRequestLen>0){
			memcpy(ucTCPRequestFrame, r->buf, sizeof(uint8_t) * ucTCPRequestLen);
			xMBPortEventPost (EV_FRAME_RECEIVED);
			eMBPoll ();
			eMBPoll ();
			if(bFrameSent){
				bFrameSent = FALSE;
				mg_send(c, ucTCPResponseFrame, ucTCPResponseLen);
				r->len = 0;                  // Tell Mongoose we've consumed data

			}else{
				c->is_closing = 1;
			}
		}else{
			c->is_closing = 1;
		}
	}
}



void init_mb_tcp(void * param){

	struct mg_mgr *mgr = (struct mg_mgr *)param;  // Event manager
	mg_listen(mgr, s_lsn, handler_mb_tcp, NULL);  // Create server connection

	eMBTCPInit(0);
	eMBEnable();

	size_t acl_size = 0;
	mg_fs_lfs_status(ACL_FILE, &acl_size, NULL);
	if(acl_size && (acl_size < ACL_FILE_MAX_SIZE)){
		mg_acl.buf = calloc(acl_size + 1, sizeof(char));
		void *fd = mg_fs_lfs_open(ACL_FILE, MG_FS_READ);
		mg_fs_lfs_read(fd, mg_acl.buf, acl_size);
		mg_acl.len = acl_size;
		mg_fs_lfs_close(fd);

	}else{
		mg_acl.buf = calloc(strlen(DEFAULT_ACL) + 1, sizeof(char));
		mg_acl.len = strlen(DEFAULT_ACL) + 1;
		strncpy(mg_acl.buf, DEFAULT_ACL, mg_acl.len);
		void *fd1 = mg_fs_lfs_open(ACL_FILE, MG_FS_WRITE);
		mg_fs_lfs_write(fd1, mg_acl.buf, mg_acl.len);
		mg_fs_lfs_close(fd1);
	}
}

BOOL  xMBTCPPortInit( USHORT usTCPPort )
{


    return TRUE;
}

BOOL  xMBTCPPortGetRequest( UCHAR **ppucMBTCPFrame, USHORT * usTCPLength )
{
    *ppucMBTCPFrame = (uint8_t *) &ucTCPRequestFrame[0];
    *usTCPLength = ucTCPRequestLen;
    // Reset the buffer.
    ucTCPRequestLen = 0;
    return TRUE;
}


BOOL xMBTCPPortSendResponse( const UCHAR *pucMBTCPFrame, USHORT usTCPLength )
{
      memcpy(ucTCPResponseFrame,pucMBTCPFrame , usTCPLength);
      ucTCPResponseLen = usTCPLength;
      bFrameSent = TRUE;
      return bFrameSent;
}


void  vMBTCPPortClose( void )
{
};

void vMBTCPPortDisable( void )
{
};


eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs );

eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode );

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNCoils,eMBRegisterMode eMode);

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete);

