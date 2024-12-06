/* ----------------------- System includes ----------------------------------*/
#include "assert.h"

/* ----------------------- FreeRTOS includes --------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* ----------------------- STR71X includes ----------------------------------*/
//#include "gpio.h"
//#include "eic.h"
//#include "uart.h"
//#include "tim.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/


/* ----------------------- Static variables ---------------------------------*/
static USHORT   usTimerDeltaOCRA;

/* ----------------------- Static functions ---------------------------------*/
void            prvvMBTimerIRQHandler( void ) __attribute__ ( ( naked ) );

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    /* Calculate output compare value for timer1. */


    return TRUE;
}

void
prvvMBTimerIRQHandler( void )
{

}

void
vMBPortTimersEnable(  )
{

}

void
vMBPortTimersDisable(  )
{

}
