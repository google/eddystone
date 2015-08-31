/**************************************************************************************************
  Filename:       eddystoneURLCfg.h

  Description:    This file contains the Eddystone URL Configuration Service
                  definitions and prototypes.

* Copyright (c) 2015, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* *  Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* *  Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* *  Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#ifndef EDDYSTONEURLCFG_H
#define EDDYSTONEURLCFG_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// TX Power Mode
#define TX_POWER_MODE_LOWEST      0
#define TX_POWER_MODE_LOW         1
#define TX_POWER_MODE_MEDIUM      2
#define TX_POWER_MODE_HIGH        3


// Parameter index
#define URLCFGSVC_LOCK_STATE              0   // boolean 
#define URLCFGSVC_LOCK                    1   // uint128
#define URLCFGSVC_UNLOCK                  2   // uint128
#define URLCFGSVC_URI_DATA                3   // uint8[18]
#define URLCFGSVC_FLAGS                   4   // int8
#define URLCFGSVC_ADV_TX_PWR_LVLS         5   // int8[4]
#define URLCFGSVC_TX_POWER_MODE           6   // uint8
#define URLCFGSVC_BEACON_PERIOD           7   // uint16
#define URLCFGSVC_RESET                   8   // boolean
#define URLCFGSVC_URI_DATA_LEN            9   // uint8

// URL Configuration Service UUID
#define URLCFGSVC_SVC_UUID                     0x2080
    
// Characteristic UUID
#define URLCFGSVC_LOCK_STATE_UUID              0x2081
#define URLCFGSVC_LOCK_UUID                    0x2082
#define URLCFGSVC_UNLOCK_UUID                  0x2083
#define URLCFGSVC_URI_DATA_UUID                0x2084
#define URLCFGSVC_FLAGS_UUID                   0x2085
#define URLCFGSVC_ADV_TX_PWR_LVLS_UUID         0x2086
#define URLCFGSVC_TX_POWER_MODE_UUID           0x2087
#define URLCFGSVC_BEACON_PERIOD_UUID           0x2088
#define URLCFGSVC_RESET_UUID                   0x2089
  
  
// Simple Keys Profile Services bit fields
#define URLCFGSVC_SERVICE               0x00000001

// Length of URI Data in bytes
#define URLCFGSVC_CHAR_URI_DATA_LEN           18

// Characteristic default values
#define URLCFG_CHAR_URI_DATA_DEFAULT          "http://www.ti.com/ble"
#define URLCFG_CHAR_FLAGS_DEFAULT             0
#define URLCFG_CHAR_TX_POWER_MODE_DEFAULT     TX_POWER_MODE_LOW
#define URLCFG_CHAR_BEACON_PERIOD_DEFAULT     1000
#define URLCFG_CHAR_LOCK_DEFAULT              {0}

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERNAL VARIABLES
 */

extern uint8    URIDataLen;

/*********************************************************************
 * MACROS
 */

// Eddystone Base 128-bit UUID: EE0CXXXX-8786-40BA-AB96-99B91AC981D8
#define EDDYSTONE_BASE_UUID_128( uuid )  0xD8, 0x81, 0xC9, 0x1A, 0xB9, 0x99, \
                                         0x96, 0xAB, 0xBA, 0x40, 0x86, 0x87, \
                           LO_UINT16( uuid ), HI_UINT16( uuid ), 0x0C, 0xEE

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*urlCfgSvcChange_t)( uint8 paramID );

typedef struct
{
  urlCfgSvcChange_t        pfnURLCfgSvcChange;  // Called when characteristic value changes
} urlCfgSvcCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */

/*
 * URLCfgSvc_AddService- Initializes the Eddystone URL Configuration
 *          service by registering GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t URLCfgSvc_AddService( void );

/*
 * URLCfgSvc_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t URLCfgSvc_RegisterAppCBs( urlCfgSvcCBs_t *appCallbacks );

/*
 * URLCfgSvc_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t URLCfgSvc_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * URLCfgSvc_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t URLCfgSvc_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* EDDYSTONEURLCFG_H */
