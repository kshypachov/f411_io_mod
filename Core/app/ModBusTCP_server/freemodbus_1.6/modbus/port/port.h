/*
 * port.h
 *
 *  Created on: 28 сент. 2021 г.
 *      Author: kir
 */

#ifndef MODBUS_INCLUDE_PORT_H_
#define MODBUS_INCLUDE_PORT_H_

#include <assert.h>
//--------FreeRTOS Lib---------------------------------------//
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
//--------FreeRTOS Lib---------------------------------------//

#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif
/* ----------------------- Type definitions ---------------------------------*/
typedef char    BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef unsigned short USHORT;
typedef short   SHORT;

typedef unsigned long ULONG;
typedef long    LONG;

#define  ENTER_CRITICAL_SECTION() portENTER_CRITICAL()

#define  EXIT_CRITICAL_SECTION() portEXIT_CRITICAL()

#endif /* MODBUS_INCLUDE_PORT_H_ */
