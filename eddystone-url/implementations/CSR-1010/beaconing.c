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
 * FILE
 *     beaconing.c
 *
 * DESCRIPTION
 *     This file defines beaconing routines.
 *

 ****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *===========================================================================*/

#include <gatt.h>           /* GATT application interface */
#include <buf_utils.h>      /* Buffer functions */
#include <mem.h>            /* Memory routines */
#include <gatt_prim.h>
#include <gatt_uuid.h>
#include <ls_app_if.h>
#include <gap_app_if.h>

/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

#include "esurl_beacon_service.h" /* Interface to this file */
#include "beaconing.h"      /* Beaconing routines */

/*=============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Advertisement payload size:
 *      31
 *      - 3 octets for mandatory Flags AD (added automatically by the firmware)
 *      - 1 octet for manufacturer specific AD length field (added by the firmware)
 */
#define ADVERT_SIZE                     (28)

/*============================================================================*
 *  Public Function Implementations
 *===========================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      BeaconStart
 *
 *  DESCRIPTION
 *      This function is used to start or stop beaconing
 *
 *  PARAMETERS
 *      None
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void BeaconStart(bool start)
{
    uint8 advData[ADVERT_SIZE];
    uint16 offset = 0;
    uint8* beacon_data;
    uint8 beacon_data_size;
    uint8 i;
    uint8 len_i = 0;
    uint8 adv_parameter_len = 0;
    uint32 beacon_interval = EsurlBeaconGetPeriodMillis();    
    
    /* Stop broadcasting */
    LsStartStopAdvertise(FALSE, whitelist_disabled, ls_addr_type_random);
    
    /* beacon_interval of zero overrides and stops beaconning */
    if (start && (beacon_interval != 0)) 
    {
        /* prepare the advertisement packet */
   
        /* set the GAP Broadcaster role */
        GapSetMode(gap_role_broadcaster,
                   gap_mode_discover_no,
                   gap_mode_connect_no,
                   gap_mode_bond_no,
                   gap_mode_security_none);
        
        /* clear the existing advertisement and scan response data */
        LsStoreAdvScanData(0, NULL, ad_src_advertise);
        LsStoreAdvScanData(0, NULL, ad_src_scan_rsp);
    
        /* set the advertisement interval */

        GapSetAdvInterval(beacon_interval, beacon_interval);
        
        /* get the beaconing data USING SERVICE */
        EsurlBeaconGetData(&beacon_data, &beacon_data_size);
        
        if(beacon_data_size > 0)
        {
            adv_parameter_len = beacon_data[0];
            len_i = adv_parameter_len - 1;
            
            /* and store in the packet */
            for(i = 1; (i < beacon_data_size) && (offset < ADVERT_SIZE); i++,offset++, len_i--)
            {
                advData[offset] = beacon_data[i];
                
                if(len_i == 0)
                {
                    /* store the advertisement parameter and get length for the next parameter */
                    LsStoreAdvScanData(adv_parameter_len, &advData[offset - adv_parameter_len + 1], ad_src_advertise);
    
                    adv_parameter_len = beacon_data[i+1];
                    len_i = adv_parameter_len;
                    i++;
                }
            }
        }
        else
        {
            /* store the advertisement data */
            LsStoreAdvScanData(offset, advData, ad_src_advertise);
        }
        
        /* Start broadcasting */
        LsStartStopAdvertise(TRUE, whitelist_disabled, ls_addr_type_random);
    }
}
