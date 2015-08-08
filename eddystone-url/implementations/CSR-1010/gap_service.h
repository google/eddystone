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
 *      gap_service.h
 *
 *  DESCRIPTION
 *      Header definitions for the GAP Service
 *

 *
 *****************************************************************************/

#ifndef __GAP_SERVICE_H__
#define __GAP_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>          /* Commonly used type definitions */
#include <gatt.h>           /* GATT application interface */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gap_conn_params.h"/* Connection and advertisement definitions */

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the GAP Service data structure.*/
extern void GapDataInit(void);

/* Handle read operations on GAP Service attributes maintained by the
 * application
 */
extern void GapHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* Handle write operations on GAP Service attributes maintained by the
 * application
 */
extern void GapHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Read the GAP Service specific data stored in NVM */
extern void GapReadDataFromNVM(uint16 *p_offset);

/* Write GAP Service specific data to NVM for the first time during
 * application initialisation
 */
extern void GapInitWriteDataToNVM(uint16 *p_offset);

/* Check if the handle belongs to the GAP Service */
extern bool GapCheckHandleRange(uint16 handle);

/* Get the reference to the 'g_device_name' array, which contains AD Type and
 * device name
 */
extern uint8 *GapGetNameAndLength(uint16 *p_name_length);

#endif /* __GAP_SERVICE_H__ */
