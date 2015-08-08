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
 *      buzzer.h
 *
 *  DESCRIPTION
 *      This file contains prototypes for accessing buzzer functionality.
 *
 
 *
 *****************************************************************************/

#ifndef __BUZZER_H__
#define __BUZZER_H__

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */

/* Only compile this file if the buzzer code has been requested */
#ifdef ENABLE_BUZZER

/*============================================================================*
 *  Public data type
 *============================================================================*/

/* Data type for different type of buzzer beeps */
typedef enum
{
    /* No beeps */
    buzzer_beep_off = 0,

    /* Short beep */
    buzzer_beep_short,

    /* Long beep */
    buzzer_beep_long,

    /* Two short beeps */
    buzzer_beep_twice,

    /* Three short beeps */
    buzzer_beep_thrice

} buzzer_beep_type;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialise the buzzer hardware */
extern void BuzzerInitHardware(void);

/* Initialise the buzzer data to a known state */
extern void BuzzerInitData(void);

/* Reset the buzzer data to a clean state */
extern void BuzzerResetData(void);

/* Trigger beeps of different types, enumerated by 'buzzer_beep_type' */
extern void SoundBuzzer(buzzer_beep_type beep_type);

#else /* ENABLE_BUZZER */

/* Define buzzer functions to expand to nothing as buzzer functionality is not 
 * enabled 
 */

#define BuzzerInitHardware()
#define BuzzerInitData()
#define BuzzerResetData()
#define SoundBuzzer(beep_type)

#endif /* ENABLE_BUZZER */

#endif /* __BUZZER_H__ */

