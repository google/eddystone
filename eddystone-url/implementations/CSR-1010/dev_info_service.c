/*****************************************************************************
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
 *      device_info_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the Device Information Service.
 *

 *****************************************************************************/

/*============================================================================*
 *  SDK Header files
 *============================================================================*/

#include <gatt.h>           /* GATT application interface */
#include <bluetooth.h>      /* Bluetooth specific type definitions */
#include <config_store.h>   /* Interface to the Configuration Store */

/*============================================================================*
 *  Local Header files
 *============================================================================*/

#include "app_gatt_db.h"    /* GATT database definitions */
#include "dev_info_service.h"/* Interface to this file */

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Device Information Service System ID characteristic */
/* Bytes have been reversed */
#define SYSTEM_ID_FIXED_CONSTANT    (0xFFFE)
#define SYSTEM_ID_LENGTH            (8)

/*============================================================================*
 *  Private Datatypes
 *============================================================================*/

/* System ID : System ID has two fields;
 * Manufacturer Identifier           : The Company Identifier is concatenated 
 *                                     with 0xFFFE
 * Organizationally Unique Identifier: Company Assigned Identifier of the
 *                                     Bluetooth Address
 *
 *
 * See following web link for definition of system ID
 * http://developer.bluetooth.org/gatt/characteristics/Pages/
 * CharacteristicViewer.aspx?u=org.bluetooth.characteristic.system_id.xml 
 */

typedef struct _SYSTEM_ID_T
{
    /* System ID size is 8 octets */
    uint8 byte[SYSTEM_ID_LENGTH];
} SYSTEM_ID_T;

typedef struct _DEV_INFO_DATA_T
{
    /* System ID of Device Information Service */
    SYSTEM_ID_T system_id;
} DEV_INFO_DATA_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* DIS data structure */
static DEV_INFO_DATA_T          g_dev_info_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Calculate the System ID based on the Bluetooth address of the device */
static bool getSystemId(SYSTEM_ID_T *sys_id);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      getSystemId
 *
 *  DESCRIPTION
 *      This function returns the Device Information Service System ID.
 *
 *  PARAMETERS
 *      sys_id [out]            System ID
 *
 *  RETURNS
 *      TRUE on success,
 *      FALSE if unable to access the device's Bluetooth address
 *----------------------------------------------------------------------------*/
static bool getSystemId(SYSTEM_ID_T * sys_id)
{
    BD_ADDR_T bdaddr;           /* Device's Bluetooth address */
    
    if(CSReadBdaddr(&bdaddr))
    {
        /* Manufacturer-defined identifier */
        sys_id->byte[0] = (uint8)(bdaddr.lap);
        sys_id->byte[1] = (uint8)(bdaddr.lap >> 8);
        sys_id->byte[2] = (uint8)(bdaddr.lap >> 16);
        sys_id->byte[3] = (uint8)(SYSTEM_ID_FIXED_CONSTANT);
        sys_id->byte[4] = (uint8)(SYSTEM_ID_FIXED_CONSTANT >> 8);

        /* Organizationally Unique Identifier */
        sys_id->byte[5] = (uint8)(bdaddr.uap);
        sys_id->byte[6] = (uint8)(bdaddr.nap);
        sys_id->byte[7] = (uint8)(bdaddr.nap >> 8);
	    
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operations on Device Information Service
 *      attributes maintained by the application and responds with the
 *      GATT_ACCESS_RSP message.
 *
 *  PARAMETERS
 *      p_ind [in]              Data received in GATT_ACCESS_IND message.
 *
 *  RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern void DeviceInfoHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;                  /* Length of attribute data, octets */
    uint8 *p_value = NULL;              /* Pointer to attribute value */
    sys_status rc = gatt_status_unlikely_error;/* Function status */
    
    switch(p_ind->handle)
    {
        case HANDLE_DEVICE_INFO_SYSTEM_ID:
        {
            /* System ID read has been requested */
            length = SYSTEM_ID_LENGTH; 
            if(getSystemId(&g_dev_info_data.system_id))
            {
                p_value = (uint8 *)(&g_dev_info_data.system_id);
                rc = sys_status_success;
            }
        }
        break;
        
        default:
        {
            rc = gatt_status_read_not_permitted;
        }
        break;
    }

    /* Send response indication */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, length, p_value);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      DeviceInfoCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the Device 
 *      Information Service.
 *
 *  PARAMETERS
 *      handle [in]             Handle to check
 *
 *  RETURNS
 *      TRUE if handle belongs to the Device Information Service, FALSE
 *      otherwise
 *----------------------------------------------------------------------------*/
extern bool DeviceInfoCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_DEVICE_INFO_SERVICE) &&
            (handle <= HANDLE_DEVICE_INFO_SERVICE_END))
            ? TRUE : FALSE;
}
