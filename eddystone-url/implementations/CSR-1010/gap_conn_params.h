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
 *      gap_conn_params.h
 *
 *  DESCRIPTION
 *      MACROs for connection and advertisement parameter values
 *
 
 *
 *****************************************************************************/

#ifndef __GAP_CONN_PARAMS_H__
#define __GAP_CONN_PARAMS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Advertising parameters. Time is expressed in microseconds and the firmware
 * will round this down to the nearest slot. Acceptable range is 20ms to 10.24s
 * and the minimum must be no larger than the maximum. This value needs to be 
 * modified at a later stage as decided GPA for specific profile.
 *
 * For the GATT Server application, use 60ms as the fast connection
 * advertisement interval. For reduced power connections the recommended range
 * is 1s to 2.5s. Vendors will need to tune these values as per their
 * requirements.
 */
#define FC_ADVERTISING_INTERVAL_MIN         (100 * MILLISECOND)
#define FC_ADVERTISING_INTERVAL_MAX         (100 * MILLISECOND)

/* Maximum number of connection parameter update requests that can be sent when 
 * connected
 */
#define MAX_NUM_CONN_PARAM_UPDATE_REQS      (2)

/* Brackets should not be used around the values of these macros. This file is
 * imported by the GATT Database Generator (gattdbgen) which does not understand 
 * brackets and will raise syntax errors.
 */

/* Preferred connection parameter values should be within the range specified by
 * the Bluetooth specification.
 */

/* Minimum and maximum connection interval in number of frames */
#define PREFERRED_MAX_CON_INTERVAL          0x0190 /* 500 ms */
#define PREFERRED_MIN_CON_INTERVAL          0x0190 /* 500 ms */

/* Slave latency in number of connection intervals */
#define PREFERRED_SLAVE_LATENCY             0x0004 /* 4 conn_intervals */

/* Supervision timeout (ms) = PREFERRED_SUPERVISION_TIMEOUT * 10 ms */
#define PREFERRED_SUPERVISION_TIMEOUT       0x03E8 /* 10 seconds */

#endif /* __GAP_CONN_PARAMS_H__ */
