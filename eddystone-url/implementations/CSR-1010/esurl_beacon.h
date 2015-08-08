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
 *      esurl_beacon.h
 *
 *  DESCRIPTION
 *      Header file for a simple GATT server application.
 *
 
 *
 ******************************************************************************/

#ifndef __ESURL_BEACON_SAMPLE_H__
#define __ESURL_BEACON_SAMPLE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gatt_access.h"    /* GATT-related routines */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Maximum number of words in central device Identity Resolving Key (IRK) */
#define MAX_WORDS_IRK                       (8)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Call the firmware Panic() routine and provide a single point for debugging
 * any application level panics
 */
extern void ReportPanic(app_panic_code panic_code);

/* Handle a short button press. If connected, the device disconnects from the
 * host, otherwise it starts advertising.
 */
extern void HandleShortButtonPress(void);

/* Change the current state of the application */
extern void SetState(app_state new_state);

/* Return the current state of the application.*/
extern app_state GetState(void);

/* Check if the whitelist is enabled or not. */
extern bool IsWhiteListEnabled(void);

/* Handle pairing removal */
extern void HandlePairingRemoval(void);

/* Start the advertisement timer. */
extern void StartAdvertTimer(uint32 interval);

/* Return whether the connected device is bonded or not */
extern bool IsDeviceBonded(void);

/* Return the unique connection ID (UCID) of the connection */
extern uint16 GetConnectionID(void);

#endif /* __ESURL_BEACON_SAMPLE_H__ */
