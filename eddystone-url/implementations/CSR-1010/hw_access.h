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
 *      hw_access.h
 *
 *  DESCRIPTION
 *      Header definitions for HW setup.
 *

 *
 *****************************************************************************/

#ifndef __HW_ACCESS_H__
#define __HW_ACCESS_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <sys_events.h>     /* System Event definitions and declarations */

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Convert a PIO number into a bit mask */
#define PIO_BIT_MASK(pio)       (0x01UL << (pio))

/* PIO direction */
#define PIO_DIRECTION_INPUT     (FALSE)
#define PIO_DIRECTION_OUTPUT    (TRUE)

/* PIO state */
#define PIO_STATE_HIGH          (TRUE)
#define PIO_STATE_LOW           (FALSE)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the application hardware */
extern void InitHardware(void);

/* Initialise the application hardware data structure */
extern void HwDataInit(void);

/* Reset the application hardware data structure */
extern void HwDataReset(void);

/* Handle the PIO changed event */
extern void HandlePIOChangedEvent(pio_changed_data *pio_data);

#endif /* __HW_ACCESS_H__ */
