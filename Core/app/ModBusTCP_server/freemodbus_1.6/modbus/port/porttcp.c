#include "port.h"
#include <string.h>
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mongoose.h"
#include "mb_tcp_app.h"
#include <string.h>

#ifndef MB_TCP_BUF_SIZE
	#define MB_TCP_BUF_SIZE  2048
#endif

uint8_t ucTCPRequestFrame [MB_TCP_BUF_SIZE];
uint16_t ucTCPRequestLen;
uint8_t ucTCPResponseFrame [MB_TCP_BUF_SIZE];
uint16_t ucTCPResponseLen;
uint8_t bFrameSent = FALSE;


//void modbus_tcps(USHORT sn, USHORT port)
//{
//	switch (getSn_SR (sn))// �������� ������ ������
//	   {
//	case SOCK_CLOSED:// ����� ������
//		socket(sn,Sn_MR_TCP,port,0x00); // ��������� �����
//		break;
//	case SOCK_INIT: // ����� ��� ���������������
//		listen (sn); // �������
//	case SOCK_ESTABLISHED: // ����� ���������
//		if(getSn_IR(sn) & Sn_IR_CON)
//		{
//			setSn_IR(sn,Sn_IR_CON);
//		}
//		ucTCPRequestLen = getSn_RX_RSR (sn); // �������� ����� ���������� ������
//			if(ucTCPRequestLen>0)
//			{
//				recv(sn,ucTCPRequestFrame, ucTCPRequestLen); // W5500 �������� ������
//				xMBPortEventPost (EV_FRAME_RECEIVED); // ���������� ������� EV_FRAME_RECEIVED ��� ���������� �������� ��������� � ������� eMBpoll ()
//				eMBPoll (); // ������������ ������� EV_FRAME_RECEIVED
//				eMBPoll (); // ������������ ������� EV_EXECUTE
//				if(bFrameSent)
//				{
//					bFrameSent = FALSE;
//					send(sn,ucTCPResponseFrame,ucTCPResponseLen); // W5500 ���������� ����� ������ ������ Modbus
//				}
//			}
//		break;
//	case SOCK_CLOSE_WAIT: // ����� ������� ��������
//		disconnect (sn); // ��������� ����������
//		break;
//	default:
//		break;
//   }
//}
//

static void handler_mb_tcp(struct mg_connection *c, int ev, void *ev_data){

	if (ev == MG_EV_READ){
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

static const char *s_lsn = "tcp://0.0.0.0:502";   // Listening address ModBusTCP

void init_mb_tcp(void * param){

	struct mg_mgr *mgr = (struct mg_mgr *)param;  // Event manager
	mg_listen(mgr, s_lsn, handler_mb_tcp, NULL);  // Create server connection

	eMBTCPInit(0);
	eMBEnable();
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

