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
 *      buzzer.c
 *
 *  DESCRIPTION
 *      This file defines routines for accessing buzzer functionality.
 *

 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <pio.h>            /* PIO configuration and control functions */
#include <pio_ctrlr.h>      /* Access to the PIO controller */
#include <timer.h>          /* Chip timer functions */

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"    /* User configuration */
#include "buzzer.h"         /* Interface to this file */
#include "hw_access.h"      /* Hardware access */
#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */

/* Only compile this file if the buzzer code has been requested */
#ifdef ENABLE_BUZZER

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Setup PIOs
 *  PIO3    Buzzer
 */
#define BUZZER_PIO              (3)

#define BUZZER_PIO_MASK         (PIO_BIT_MASK(BUZZER_PIO))

/* The index (0-3) of the PWM unit to be configured */
#define BUZZER_PWM_INDEX_0      (0)

/* PWM parameters for Buzzer */

/* Dull on. off and hold times */
#define DULL_BUZZ_ON_TIME       (2)    /* 60us */
#define DULL_BUZZ_OFF_TIME      (15)   /* 450us */
#define DULL_BUZZ_HOLD_TIME     (0)

/* Bright on, off and hold times */
#define BRIGHT_BUZZ_ON_TIME     (2)    /* 60us */
#define BRIGHT_BUZZ_OFF_TIME    (15)   /* 450us */
#define BRIGHT_BUZZ_HOLD_TIME   (0)    /* 0us */

#define BUZZ_RAMP_RATE          (0xFF)

/* TIMER values for Buzzer */
#define SHORT_BEEP_TIMER_VALUE  (100* MILLISECOND)
#define LONG_BEEP_TIMER_VALUE   (500* MILLISECOND)
#define BEEP_GAP_TIMER_VALUE    (25* MILLISECOND)

/*============================================================================*
 *  Public data types
 *============================================================================*/

/* Buzzer data structure */
typedef struct _BUZZER_DATA_T
{

    /* Buzzer timer ID */
    timer_id                    buzzer_tid;

    /* Current beep type */
    buzzer_beep_type            beep_type;

    /* Beep counter. This variable will be initialised to 0 on beep start and
     * will increment at every beep sound.
     */
    uint16                      beep_count;

} BUZZER_DATA_T;

/*============================================================================*
 *  Private data
 *============================================================================*/

/* Buzzer data instance */
static BUZZER_DATA_T            g_buzz_data;

/*============================================================================*
 *  Private Function Prototypes
 *===========================================================================*/

/* Control buzzer at timer expiry */
static void appBuzzerTimerHandler(timer_id tid);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      appBuzzerTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to stop the Buzzer at the expiry of timer.
 *
 *  PARAMETERS
 *      tid [in]                ID of expired timer (unused)
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void appBuzzerTimerHandler(timer_id tid)
{
    /* Duration of next timer */
    uint32 beep_timer = SHORT_BEEP_TIMER_VALUE;

    /* The buzzer timer has just expired, so reset the timer ID */
    g_buzz_data.buzzer_tid = TIMER_INVALID;

    switch(g_buzz_data.beep_type)
    {
        case buzzer_beep_short: /* FALLTHROUGH */
        case buzzer_beep_long:
        {
            g_buzz_data.beep_type = buzzer_beep_off;

            /* Disable buzzer */
            PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);
        }
        break;

        case buzzer_beep_twice:
        {
            if(g_buzz_data.beep_count == 0)
            {
                /* First beep sounded. Increment the beep count and start the
                 * silent gap.
                 */
                g_buzz_data.beep_count = 1;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                /* Time gap between two beeps */
                beep_timer = BEEP_GAP_TIMER_VALUE;
            }
            else if(g_buzz_data.beep_count == 1)
            {
                /* Sound the second beep and increment the beep count. */
                g_buzz_data.beep_count = 2;

                /* Enable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

                /* Start another short beep */
                beep_timer = SHORT_BEEP_TIMER_VALUE;
            }
            else
            {
                /* Two beeps have been sounded. Stop buzzer now and reset the 
                 * beep count.
                 */
                g_buzz_data.beep_count = 0;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                g_buzz_data.beep_type = buzzer_beep_off;
            }
        }
        break;

        case buzzer_beep_thrice:
        {
            if(g_buzz_data.beep_count == 0 ||
               g_buzz_data.beep_count == 2)
            {
                /* Start the silent gap*/
                ++g_buzz_data.beep_count;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                /* Time gap between two beeps */
                beep_timer = BEEP_GAP_TIMER_VALUE;
            }
            else if(g_buzz_data.beep_count == 1 ||
                    g_buzz_data.beep_count == 3)
            {
                /* Start the beep sounding part. */
                ++g_buzz_data.beep_count;

                /* Enable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

                beep_timer = SHORT_BEEP_TIMER_VALUE;
            }
            else
            {
                /* Three beeps have been sounded. Stop the buzzer. */
                g_buzz_data.beep_count = 0;

                /* Disable buzzer */
                PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

                g_buzz_data.beep_type = buzzer_beep_off;
            }
        }
        break;

        default:
        {
            /* No such beep type */
            ReportPanic(app_panic_unexpected_beep_type);
        }
        break;
    }

    if(g_buzz_data.beep_type != buzzer_beep_off)
    {
        /* Start the timer */
        g_buzz_data.buzzer_tid = TimerCreate(beep_timer, TRUE, 
                                               appBuzzerTimerHandler);
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      BuzzerInitHardware
 *
 *  DESCRIPTION
 *      This function initialises the buzzer hardware.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BuzzerInitHardware(void)
{
    /* Configure the buzzer PIO to use PWM. */
    PioSetModes(BUZZER_PIO_MASK, pio_mode_pwm0);

    /* Configure the PWM for buzzer ON OFF values */
    PioConfigPWM(BUZZER_PWM_INDEX_0, pio_pwm_mode_push_pull, DULL_BUZZ_ON_TIME,
                 DULL_BUZZ_OFF_TIME, DULL_BUZZ_HOLD_TIME, BRIGHT_BUZZ_ON_TIME,
                 BRIGHT_BUZZ_OFF_TIME, BRIGHT_BUZZ_HOLD_TIME, BUZZ_RAMP_RATE);

    /* Disable buzzer for the time being. */
    PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BuzzerInitData
 *
 *  DESCRIPTION
 *      This function initialises the buzzer data to a known state. It is
 *      intended to be called once, for example after a power-on reset or HCI
 *      reset.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BuzzerInitData(void)
{
    /* Initialise buzzer timer */
    g_buzz_data.buzzer_tid = TIMER_INVALID;
    
    /* The remaining values in the data structure are initialised by
     * SoundBuzzer.
     */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      BuzzerResetData
 *
 *  DESCRIPTION
 *      This function resets the buzzer data. It is intended to be called when
 *      the data needs to be reset to a clean state, for example, whenever a
 *      device connects or disconnects.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BuzzerResetData(void)
{
    /* Delete buzzer timer if running */
    if (g_buzz_data.buzzer_tid != TIMER_INVALID)
    {
        TimerDelete(g_buzz_data.buzzer_tid);
        g_buzz_data.buzzer_tid = TIMER_INVALID;
    }
    
    /* The remaining values in the data structure are reset by SoundBuzzer */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SoundBuzzer
 *
 *  DESCRIPTION
 *      This function is called to trigger beeps of different types, enumerated
 *      by 'buzzer_beep_type'.
 *
 *  PARAMETERS
 *      beep_type [in]          Type of beep required
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void SoundBuzzer(buzzer_beep_type beep_type)
{
    /* Duration of beep */
    uint32 beep_timer = SHORT_BEEP_TIMER_VALUE;

    /* Disable the buzzer and stop the buzzer timer. */
    PioEnablePWM(BUZZER_PWM_INDEX_0, FALSE);
    if (g_buzz_data.buzzer_tid != TIMER_INVALID)
    {
        TimerDelete(g_buzz_data.buzzer_tid);
        g_buzz_data.buzzer_tid = TIMER_INVALID;
    }

    /* Reset the beeper state */
    g_buzz_data.beep_count = 0;

    /* Store the beep type. It will be used on timer expiry to check the type of
     * beep being sounded.
     */
    g_buzz_data.beep_type = beep_type;

    switch(g_buzz_data.beep_type)
    {
        case buzzer_beep_off:
        {
            /* Don't do anything */
        }
        break;

        case buzzer_beep_short: /* One short beep will be sounded */
            /* FALLTHROUGH */
        case buzzer_beep_twice: /* Two short beeps will be sounded */
            /* FALLTHROUGH */
        case buzzer_beep_thrice: /* Three short beeps will be sounded */
        {
            beep_timer = SHORT_BEEP_TIMER_VALUE;
        }
        break;

        case buzzer_beep_long:
        {
            /* One long beep will be sounded */
            beep_timer = LONG_BEEP_TIMER_VALUE;
        }
        break;

        default:
        {
            /* No such beep type defined */
            ReportPanic(app_panic_unexpected_beep_type);
        }
        break;
    }

    if(g_buzz_data.beep_type != buzzer_beep_off)
    {
        /* Enable buzzer */
        PioEnablePWM(BUZZER_PWM_INDEX_0, TRUE);

        /* Start the buzzer timer */
        g_buzz_data.buzzer_tid = TimerCreate(beep_timer, TRUE, 
                                             appBuzzerTimerHandler);
    }

}

#endif /* ENABLE_BUZZER */
