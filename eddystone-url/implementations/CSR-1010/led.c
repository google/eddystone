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
 *      led.c
 *
 *  DESCRIPTION
 *      This file defines routines for accessing LED functionality.
 *
 
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <pio.h>            /* PIO configuration and control functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */
#include "led.h"            /* Interface to this file */
#include "hw_access.h"      /* Hardware access */

/* Only compile this file if the LED code has been requested */
#ifdef ENABLE_LED

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Setup PIO4 as LED PIO */
#define LED_PIO                     (4)

#define LED_PIO_MASK                (PIO_BIT_MASK(LED_PIO))

/* The index (0-3) of the PWM unit to be configured */
#define LED_PWM_INDEX                (1)
#define LED_PIO_MODE                 (pio_mode_pwm1)

/* PWM parameters for LED */
#define LOW_LED_OFF_TIME             (50)
#define LOW_LED_ON_TIME              (1)
#define LOW_LED_HOLD_TIME            (50)

#define HIGH_LED_OFF_TIME            (1)
#define HIGH_LED_ON_TIME             (50)
#define HIGH_LED_HOLD_TIME           (50)

#define LED_RAMP_RATE                (50)

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      LedInitHardware
 *
 *  DESCRIPTION
 *      This function initialises the LED hardware.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void LedInitHardware(void)
{
    /* set up LED PIO */
    PioSetModes(LED_PIO_MASK, LED_PIO_MODE);
    PioSetDir(LED_PIO, TRUE);
    PioSet(LED_PIO, FALSE);

    /* Configure LED PIO for glowing */
    PioConfigPWM(LED_PWM_INDEX, pio_pwm_mode_push_pull, LOW_LED_OFF_TIME, 
                 LOW_LED_ON_TIME, LOW_LED_HOLD_TIME, HIGH_LED_OFF_TIME,
                 HIGH_LED_ON_TIME, HIGH_LED_HOLD_TIME, LED_RAMP_RATE);

    PioEnablePWM(LED_PWM_INDEX, FALSE);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      LedEnable
 *
 *  DESCRIPTION
 *      Enables or disables LED indication
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void LedEnable(bool enable)
{
    if(enable)
    {
        /* enable LED */
        PioSetModes(LED_PIO_MASK, pio_mode_pwm1);
        PioEnablePWM(LED_PWM_INDEX, TRUE);
        PioSet(LED_PIO, PIO_STATE_HIGH);
    }
    else
    {
        /* disable LED */
        PioEnablePWM(LED_PWM_INDEX, FALSE);
        PioSetModes(LED_PIO_MASK, pio_mode_user);
        PioSet(LED_PIO, PIO_STATE_LOW);
    }
}

#endif /* ENABLE_LED */
