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
 *      esurl_beacon_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROs for the CSR custom defined Beacon service
 *
 
 *
 ******************************************************************************/
#ifndef __ESURL_BEACON_UUIDS_H__
#define __ESURL_BEACON_UUIDS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Brackets should not be used around the value of a macro. The parser which
 * creates .c and .h files from .db file doesn't understand brackets
 * and will raise syntax errors.
 */
                                         
/*============================================================================*
 *  The Esurl Beacon Service UUID (v2)
 *============================================================================*/

/* Esurl Beacon service UUID */
#define UUID_ESURL_BEACON_SERVICE               0xee0c2080878640baab9699b91ac981d8
                                            
/* Characteristics */ 
#define UUID_ESURL_BEACON_LOCK_STATE            0xee0c2081878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_LOCK                  0xee0c2082878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_UNLOCK                0xee0c2083878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_URI_DATA              0xee0c2084878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_FLAGS                 0xee0c2085878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_ADV_TX_POWER_LEVELS   0xee0c2086878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_TX_POWER_MODE         0xee0c2087878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_PERIOD                0xee0c2088878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_RESET                 0xee0c2089878640baab9699b91ac981d8
#define UUID_ESURL_BEACON_RADIO_TX_POWER_LEVELS 0xee0c208a878640baab9699b91ac981d8  

#endif /* __ESURL_BEACON_UUIDS_H__ */
