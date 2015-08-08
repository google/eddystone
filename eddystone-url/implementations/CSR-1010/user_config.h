/*******************************************************************************
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
 * FILE
 *      user_config.h
 *
 * DESCRIPTION
 *      This file contains definitions to customise the application.
 *
 
 *
 ******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* The ENABLE_BUZZER macro controls whether buzzer control code is compiled.
 * This flag may be disabled to prevent use of the buzzer, for example to
 * provide more accurate current consumption measurements.
 */
#define ENABLE_BUZZER

/* The ENABLE_LED macro controls whether LED control code is compiled. */
/*#define ENABLE_LED*/
  
/* The PAIRING_SUPPORT macro controls whether pairing and encryption code is
 * compiled. This flag may be disabled for the applications that do not require
 * pairing.
 */
/*
#define PAIRING_SUPPORT
*/

/* This macro when defined enables the debug output on UART */
/*
#define DEBUG_OUTPUT_ENABLED
*/

/* The CONNECTED_IDLE_TIMEOUT_VALUE macro specifies how long the application may
 * be idle for during the Connected state. The device will disconnect when this
 * timer expires. Vendors are free to decide whether to enable this timeout, and
 * the timeout duration, as per their use case.
 * If this macro is not defined there will be no disconnection.
 */
/*#define CONNECTED_IDLE_TIMEOUT_VALUE   (5 * MINUTE)*/

#endif /* __USER_CONFIG_H__ */
