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
 *      beaconing.h
 *
 *  DESCRIPTION
 *      Header definitions for the beaconing functions
 *
 
 *
 *****************************************************************************/

#ifndef __BEACONING_H__
#define __BEACONING_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <time.h>           /* Time interface */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Maximum beacon data size */
#define BEACON_DATA_SIZE                    (28)

/* Beaconing advertising interval */
#define BEACON_ADVERTISING_INTERVAL         (1 * SECOND)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Start or stop beaconing */
extern void BeaconStart(bool start);

#endif /* __BEACONING_H__ */
