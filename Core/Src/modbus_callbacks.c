/*
 * modbus_callbacks.c
 *
 *  Created on: Dec 2, 2024
 *      Author: kiro
 */


#include "mb.h"
#include "mbutils.h"

//// Пример массива регистров
//static USHORT usRegHoldingBuf[100];
//
// Обработчик для чтения/записи регистров
//eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
//    USHORT i;
//    usAddress--; // Modbus адреса начинаются с 1
//
//    if ((usAddress >= 0) && (usAddress + usNRegs <= 100)) {
//        if (eMode == MB_REG_READ) {
//            for (i = 0; i < usNRegs; i++) {
//                pucRegBuffer[2 * i] = (usRegHoldingBuf[usAddress + i] >> 8) & 0xFF;
//                pucRegBuffer[2 * i + 1] = usRegHoldingBuf[usAddress + i] & 0xFF;
//            }
//        } else if (eMode == MB_REG_WRITE) {
//            for (i = 0; i < usNRegs; i++) {
//                usRegHoldingBuf[usAddress + i] = (pucRegBuffer[2 * i] << 8) | pucRegBuffer[2 * i + 1];
//            }
//        }
//        return MB_ENOERR;
//    }
//    return MB_ENOREG;
//}






