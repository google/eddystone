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
 *      hw_access.c
 *
 *  DESCRIPTION
 *      This file defines the application hardware specific routines.
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

#include "hw_access.h"      /* Interface to this file */
#include "esurl_beacon.h"    /* Definitions used throughout the GATT server */
#include "buzzer.h"         /* Buzzer functions */
#include "led.h"            /* LED functions */

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Setup PIO 11 as Button PIO */
#define BUTTON_PIO                  (11)

#define BUTTON_PIO_MASK             (PIO_BIT_MASK(BUTTON_PIO))

/* Extra long button press timer */
#define EXTRA_LONG_BUTTON_PRESS_TIMER \
                                    (4*SECOND)

/*============================================================================*
 *  Public data type
 *============================================================================*/

/* Application Hardware data structure */
typedef struct _APP_HW_DATA_T
{

    /* Timer for button press */
    timer_id                    button_press_tid;

} APP_HW_DATA_T;

/*============================================================================*
 *  Public data
 *============================================================================*/

/* Application hardware data instance */
static APP_HW_DATA_T            g_app_hw_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Handle extra long button press */
static void handleExtraLongButtonPress(timer_id tid);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      handleExtraLongButtonPress
 *
 *  DESCRIPTION
 *      This function contains handling of extra long button press, which
 *      triggers pairing / bonding removal.
 *
 *  PARAMETERS
 *      tid [in]                ID of timer that has expired
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
static void handleExtraLongButtonPress(timer_id tid)
{
    if(tid == g_app_hw_data.button_press_tid)
    {
        /* Re-initialise button press timer */
        g_app_hw_data.button_press_tid = TIMER_INVALID;

        /* Sound three beeps to indicate pairing removal to user */
        SoundBuzzer(buzzer_beep_thrice);
        
        /* Handle pairing removal */
        HandlePairingRemoval();

    } /* Else ignore timer */

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitHardware
 *
 *  DESCRIPTION
 *      This function is called to initialise the application hardware.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void InitHardware(void)
{
    /* Setup PIOs
     * PIO11 - Button
     */
    /* Set the button PIO to user mode */
    PioSetModes(BUTTON_PIO_MASK, pio_mode_user);

    /* Set the PIO direction as input */
    PioSetDir(BUTTON_PIO, PIO_DIRECTION_INPUT);

    /* Pull up the PIO */
    PioSetPullModes(BUTTON_PIO_MASK, pio_mode_strong_pull_up);

    /* Initialise buzzer hardware */
    BuzzerInitHardware();
    
    /* Initialise LED hardware */
    LedInitHardware();

    /* Request an event when the button PIO changes state */
    PioSetEventMask(BUTTON_PIO_MASK, pio_event_mode_both);

    /* Save power by changing the I2C pull mode to pull down.*/
    PioSetI2CPullMode(pio_i2c_pull_mode_strong_pull_down);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HwDataInit
 *
 *  DESCRIPTION
 *      This function initialises the hardware data to a known state. It is
 *      intended to be called once, for example after a power-on reset or HCI
 *      reset.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HwDataInit(void)
{
    /* Initialise button press timer */
    g_app_hw_data.button_press_tid = TIMER_INVALID;

    /* Initialise buzzer data */
    BuzzerInitData();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HwDataReset
 *
 *  DESCRIPTION
 *      This function resets the hardware data. It is intended to be called when
 *      the data needs to be reset to a clean state, for example, whenever a
 *      device connects or disconnects.
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HwDataReset(void)
{
    /* Delete button press timer */
    if (g_app_hw_data.button_press_tid != TIMER_INVALID)
    {
        TimerDelete(g_app_hw_data.button_press_tid);
        g_app_hw_data.button_press_tid = TIMER_INVALID;
    }

    /* Reset buzzer data */
    BuzzerResetData();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOChangedEvent
 *
 *  DESCRIPTION
 *      This function handles the PIO Changed event.
 *
 *  PARAMETERS
 *      pio_data [in]           State of the PIOs when the event occurred
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void HandlePIOChangedEvent(pio_changed_data *pio_data)
{

    if(pio_data->pio_cause & BUTTON_PIO_MASK)
    {
        /* PIO changed */
        uint32 pios = PioGets();

        if(!(pios & BUTTON_PIO_MASK))
        {
            /* This event is triggered when a button is pressed. */

            /* Start a timer for EXTRA_LONG_BUTTON_PRESS_TIMER seconds. If the
             * timer expires before the button is released an extra long button
             * press is detected. If the button is released before the timer
             * expires a short button press is detected.
             */
            TimerDelete(g_app_hw_data.button_press_tid);

            g_app_hw_data.button_press_tid = 
                TimerCreate(EXTRA_LONG_BUTTON_PRESS_TIMER,
                                           TRUE, handleExtraLongButtonPress);
        }
        else
        {
            /* This event comes when a button is released. */
            if(g_app_hw_data.button_press_tid != TIMER_INVALID)
            {
                /* Timer was already running. This means it was a short button 
                 * press.
                 */
                TimerDelete(g_app_hw_data.button_press_tid);
                g_app_hw_data.button_press_tid = TIMER_INVALID;

                /* Indicate short button press using short beep */
                SoundBuzzer(buzzer_beep_short);

                HandleShortButtonPress();
            }
        }
    }
}

