/******************************************************************************
 *    Copyright (c) 2015 Cambridge Silicon Radio Limited 
 *    All rights reserved.
 * 
 *    Redistribution and use in source and binary forms, with or without modification, 
 *    are permitted (subject to the limitations in the disclaimer below) provided that the
 *    following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice, this list of 
 *    conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * 
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  FILE
 *      debug_interface.h
 *
 *  DESCRIPTION
 *      This file defines debug output routines for the application
 *
 *
 *****************************************************************************/

#ifndef __DEBUG_INTERFACE_H__
#define __DEBUG_INTERFACE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <bluetooth.h>      /* Bluetooth specific type definitions */
#include <debug.h>          /* Simple host interface to the UART driver */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

#ifdef DEBUG_OUTPUT_ENABLED

/* Map macros to the debug functions */
#define DebugIfWriteString(a)          DebugWriteString(a)
#define DebugIfWriteUint8(a)           DebugWriteUint8(a)
#define DebugIfWriteUint16(a)          DebugWriteUint16(a)
#define DebugIfWriteUint32(a)          DebugWriteUint32(a)

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfInit
 *
 *  DESCRIPTION
 *      Initialise debug output.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DebugIfInit(void);

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteBdAddress
 *
 *  DESCRIPTION
 *      Print out a Bluetooth address.
 *
 *  PARAMETERS
 *      address [in]            Bluetooth address to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteBdAddress(const TYPED_BD_ADDR_T *address);

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteInt
 *
 *  DESCRIPTION
 *      Print out an integer value
 *
 *  PARAMETERS
 *      value [in]              Integer value to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteInt(int16 value);

/* Write an error message with a status code */
#define DebugIfWriteErrorMessage(msg, error)   \
    DebugWriteString(msg);                     \
    DebugWriteString(" (0x");                  \
    DebugWriteUint16((uint16)error);           \
    DebugWriteString(")\r\n")

/*----------------------------------------------------------------------------*
 *  NAME
 *      DebugIfWriteUuid128
 *
 *  DESCRIPTION
 *      Print out a 128-bit UUID
 *
 *  PARAMETERS
 *      uuid [in]               Buffer containing 128-bit UUID to write to UART
 *
 *  RETURNS
 *      Nothing
 *---------------------------------------------------------------------------*/
extern void DebugIfWriteUuid128(const uint8 *uuid);
    
#else

#define DebugIfWriteString(a)
#define DebugIfWriteUint8(a)
#define DebugIfWriteUint16(a)
#define DebugIfWriteUint32(a)
#define DebugIfInit()
#define DebugIfWriteBdAddress(a)
#define DebugIfWriteInt(a)
#define DebugIfWriteErrorMessage(msg, error)
#define DebugIfWriteUuid128(a)

#endif /* DEBUG_OUTPUT_ENABLED */

#endif /* __DEBUG_INTERFACE_H__ */

